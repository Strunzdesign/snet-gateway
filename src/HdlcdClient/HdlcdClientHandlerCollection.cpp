/**
 * \file      HdlcdClientHandlerCollection.cpp
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

#include "HdlcdClientHandlerCollection.h"
#include "HdlcdClientHandler.h"
#include <assert.h>

HdlcdClientHandlerCollection::HdlcdClientHandlerCollection(boost::asio::io_service& a_IOService): m_IOService(a_IOService) {
}

void HdlcdClientHandlerCollection::Initialize(std::shared_ptr<Routing> a_RoutingEntity) {
    assert(a_RoutingEntity);
    m_RoutingEntity = a_RoutingEntity;
}

void HdlcdClientHandlerCollection::SystemShutdown() {
    // Drop all shared pointers
    m_RoutingEntity.reset();    
    for (auto l_HandlerIterator = m_HdlcdClientHandlerVector.begin(); l_HandlerIterator != m_HdlcdClientHandlerVector.end(); ++l_HandlerIterator) {
        auto& l_Handler = (*l_HandlerIterator);
        l_Handler->Close();
        l_Handler.reset();
    } // for
}

void HdlcdClientHandlerCollection::CreateHdlcdClientHandler(const std::string& a_DestinationName, const std::string& a_TcpPort, const std::string& a_SerialPortName) {
    // Create new HDLCd client entity
    assert(m_RoutingEntity);
    auto l_NewClientHandler = std::make_shared<HdlcdClientHandler>(m_IOService, a_DestinationName, a_TcpPort, a_SerialPortName, m_RoutingEntity);
    m_HdlcdClientHandlerVector.push_back(l_NewClientHandler);
}

void HdlcdClientHandlerCollection::Send(const HdlcdPacketData& a_HdlcdPacketData, std::function<void()> a_OnSendDoneCallback) {
    for (auto l_HdlcdClientHandlerIterator = m_HdlcdClientHandlerVector.begin(); l_HdlcdClientHandlerIterator != m_HdlcdClientHandlerVector.end(); ++l_HdlcdClientHandlerIterator) {
        (*l_HdlcdClientHandlerIterator)->Send(a_HdlcdPacketData);
    } // for
}
