/**
 * \file      GatewayAccessServerHandler.cpp
 * \brief     
 * \author    Florian Evers, florian-evers@gmx.de
 * \copyright GNU Public License version 3.
 *
 * The HDLC Deamon implements the HDLC protocol to easily talk to devices connected via serial communications.
 * Copyright (C) 2016  Florian Evers, florian-evers@gmx.de
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "GatewayAccessServerHandler.h"
#include "Routing.h"
#include "SnetServiceMessage.h"
#include "AddressLease.h"
#include "AddressService.h"
#include "Component.h"
#include <assert.h>

GatewayAccessServerHandler::GatewayAccessServerHandler(boost::asio::io_service& a_IOService, std::shared_ptr<GatewayAccessServerHandlerCollection> a_GatewayAccessServerHandlerCollection, boost::asio::ip::tcp::socket& a_TCPSocket, std::shared_ptr<Routing> a_RoutingEntity, std::shared_ptr<AddressLease> a_AddressLease):
    m_GatewayAccessServerHandlerCollection(a_GatewayAccessServerHandlerCollection),
    m_RoutingEntity(a_RoutingEntity),
    m_AddressLease(a_AddressLease),
    m_GatewayAccessProtocol(a_IOService, a_TCPSocket),
    m_Registered(false) {
    // Checks
    assert(m_GatewayAccessServerHandlerCollection);
    assert(m_RoutingEntity);
    assert(m_AddressLease);
    
    // Register callbacks
    m_GatewayAccessProtocol.SetOnDataCallback([this](const std::vector<unsigned char> &a_HigherLayerPayload){ OnPayload(a_HigherLayerPayload); });
    m_GatewayAccessProtocol.SetOnClosedCallback([this](){ Close(); });
}

void GatewayAccessServerHandler::Start() {
    assert(m_Registered == false);
    m_Registered = true;
    m_GatewayAccessServerHandlerCollection->RegisterGatewayAccessServerHandler(shared_from_this());
    m_GatewayAccessProtocol.Start();
}

void GatewayAccessServerHandler::Close() {
    // Keep this object alive
    auto self(shared_from_this());
    if (m_Registered) {
        m_Registered = false;
        assert(m_AddressLease);
        m_AddressLease.reset(); // Deregisters itself
        m_GatewayAccessServerHandlerCollection->DeregisterGatewayAccessServerHandler(self);
        m_GatewayAccessServerHandlerCollection.reset();
    } // if
}

bool GatewayAccessServerHandler::Send(const SnetServiceMessage& a_SnetServiceMessage) {
    // Check destination address
    if (((a_SnetServiceMessage.GetDstSSA() > 0x4000) && (a_SnetServiceMessage.GetDstSSA() < 0xFFF0)) && (a_SnetServiceMessage.GetDstSSA() != m_AddressLease->GetAddress())) {
        // Not for this tool!
        return false;
    } // if
    
    // Check subscription if the message was sent to the gateway address 0x4000 or, i.e., the wildcard address 0xFFFE (WIRED_ADDR)
    if (((a_SnetServiceMessage.GetDstSSA() == 0x4000) || (a_SnetServiceMessage.GetDstSSA() >= 0xFFF0)) &&
        (m_PublishSubscribeService.IsServiceIdForMe(a_SnetServiceMessage.GetSrcServiceId()) == false)) {
        // Subscription does not fit!
        return false;
    } // if
        
    // Deliver
    return m_GatewayAccessProtocol.Send(a_SnetServiceMessage.Serialize());
}

void GatewayAccessServerHandler::OnPayload(const std::vector<unsigned char> &a_HigherLayerPayload) {
    // Relay the payload
    SnetServiceMessage l_ServiceMessage;
    if (l_ServiceMessage.Deserialize(a_HigherLayerPayload)) {
        // Check if it is directed to the address service
        if (l_ServiceMessage.GetDstSSA() == 0x3FFC) {
            auto l_AddressAssignmentReply = AddressService::ProcessRequest(l_ServiceMessage, m_AddressLease);
            if (l_AddressAssignmentReply.GetSrcServiceId() == 0xAE) {
                m_GatewayAccessProtocol.Send(l_AddressAssignmentReply.Serialize());
            } // if

            return;
        } // if
        
        // Check if it is directed to the publish / subscribe service
        if (l_ServiceMessage.GetDstSSA() == 0x4000) {
            auto l_PublishSubscribeConfirmation = m_PublishSubscribeService.ProcessRequest(l_ServiceMessage, m_AddressLease);
            if (l_PublishSubscribeConfirmation.GetDstServiceId() == 0xB0) {
                m_GatewayAccessProtocol.Send(l_PublishSubscribeConfirmation.Serialize());
            } // if

            return;
        } // if
        
        // Remap the source address
        if (l_ServiceMessage.GetSrcSSA() != 0xFFFE) {
            l_ServiceMessage.SetSrcSSA(m_AddressLease->GetAddress());
        } // if
        
        // Route this packet
        m_RoutingEntity->RouteSnetPacket(l_ServiceMessage, COMPONENT_GWCLIENTS);
    } // if
}
