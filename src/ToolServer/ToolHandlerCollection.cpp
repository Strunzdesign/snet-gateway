/**
 * \file      ToolHandlerCollection.cpp
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

#include "ToolHandlerCollection.h"
#include "ToolHandler.h"
#include "ToolFrame.h"
#include "SnetServiceMessage.h"
#include "AddressPool.h"
#include <assert.h>
using boost::asio::ip::tcp;

ToolHandlerCollection::ToolHandlerCollection(boost::asio::io_service& a_IOService, uint16_t a_TcpPortNbr): m_IOService(a_IOService), m_TcpAcceptor(a_IOService, tcp::endpoint(tcp::v4(), a_TcpPortNbr)), m_TcpSocket(a_IOService)  {
    m_AddressPool = std::make_shared<AddressPool>();
}

void ToolHandlerCollection::Initialize(std::shared_ptr<Routing> a_RoutingEntity) {
    assert(a_RoutingEntity);
    m_RoutingEntity = a_RoutingEntity;
    
    // Trigger activity
    DoAccept();
}

void ToolHandlerCollection::SystemShutdown() {
    // Stop accepting subsequent TCP connections
    m_TcpAcceptor.close();

    // Drop all shared pointers
    m_RoutingEntity.reset();
    while (!m_ToolHandlerList.empty()) {
        (*m_ToolHandlerList.begin())->Close();
    } // while
}

std::shared_ptr<AddressLease> ToolHandlerCollection::RegisterToolHandler(std::shared_ptr<ToolHandler> a_ToolHandler) {
    assert(m_RoutingEntity);
    auto l_AddressLease = m_AddressPool->ObtainAddressLease();
    a_ToolHandler->RegisterRoutingEntity(m_RoutingEntity);
    m_ToolHandlerList.emplace_back(std::move(a_ToolHandler));
    assert(l_AddressLease);
    return l_AddressLease;
}

void ToolHandlerCollection::DeregisterToolHandler(std::shared_ptr<ToolHandler> a_ToolHandler) {
    m_ToolHandlerList.remove(a_ToolHandler);
}

void ToolHandlerCollection::Send(const SnetServiceMessage& a_SnetServiceMessage) {
    for (auto l_It = m_ToolHandlerList.begin(); l_It != m_ToolHandlerList.end(); ++l_It) {
        (*l_It)->Send(a_SnetServiceMessage);
    } // for
}

void ToolHandlerCollection::DoAccept() {
    m_TcpAcceptor.async_accept(m_TcpSocket, [this](boost::system::error_code a_ErrorCode) {
        if (!a_ErrorCode) {
            // Create a tool handler object and start it. It registers itself to the tool handler collection
            auto l_ToolHandler = std::make_shared<ToolHandler>(shared_from_this(), m_TcpSocket);
            l_ToolHandler->Start();
        } // if

        // Wait for subsequent TCP connections
        DoAccept();
    }); // async_accept
}
