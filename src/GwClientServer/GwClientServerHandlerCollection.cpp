/**
 * \file      GwClientServerHandlerCollection.cpp
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

#include "GwClientServerHandlerCollection.h"
#include "GwClientServerHandler.h"
#include "SnetServiceMessage.h"
#include "AddressPool.h"
#include <assert.h>
using boost::asio::ip::tcp;

GwClientServerHandlerCollection::GwClientServerHandlerCollection(boost::asio::io_service& a_IOService, uint16_t a_TcpPortNbr): m_IOService(a_IOService), m_TcpAcceptor(a_IOService, tcp::endpoint(tcp::v4(), a_TcpPortNbr)), m_TcpSocket(a_IOService) {
    m_AddressPool = std::make_shared<AddressPool>();
}

void GwClientServerHandlerCollection::Initialize(std::shared_ptr<Routing> a_RoutingEntity) {
    assert(a_RoutingEntity);
    m_RoutingEntity = a_RoutingEntity;
    
    // Trigger activity
    DoAccept();
}

void GwClientServerHandlerCollection::SystemShutdown() {
    // Stop accepting subsequent TCP connections
    m_TcpAcceptor.close();

    // Drop all shared pointers
    m_RoutingEntity.reset();
    while (!m_GwClientServerHandlerList.empty()) {
        (*m_GwClientServerHandlerList.begin())->Close();
    } // while
}

void GwClientServerHandlerCollection::RegisterGwClientServerHandler(std::shared_ptr<GwClientServerHandler> a_GwClientServerHandler) {
    m_GwClientServerHandlerList.emplace_back(std::move(a_GwClientServerHandler));
}

void GwClientServerHandlerCollection::DeregisterGwClientServerHandler(std::shared_ptr<GwClientServerHandler> a_GwClientServerHandler) {
    m_GwClientServerHandlerList.remove(a_GwClientServerHandler);
}

void GwClientServerHandlerCollection::Send(const SnetServiceMessage& a_SnetServiceMessage) {
    for (auto l_It = m_GwClientServerHandlerList.begin(); l_It != m_GwClientServerHandlerList.end(); ++l_It) {
        (*l_It)->Send(a_SnetServiceMessage);
    } // for
}

void GwClientServerHandlerCollection::DoAccept() {
    m_TcpAcceptor.async_accept(m_TcpSocket, [this](boost::system::error_code a_ErrorCode) {
        if (!a_ErrorCode) {
            // Create a tool handler object and start it. It registers itself to the tool handler collection
            assert(m_RoutingEntity);
            assert(m_AddressPool);
            auto l_AddressLease = m_AddressPool->ObtainAddressLease();
            assert(l_AddressLease);
            auto l_GwClientServerHandler = std::make_shared<GwClientServerHandler>(m_IOService, shared_from_this(), m_TcpSocket, m_RoutingEntity, l_AddressLease);
            l_GwClientServerHandler->Start();
        } // if

        // Wait for subsequent TCP connections
        DoAccept();
    }); // async_accept
}
