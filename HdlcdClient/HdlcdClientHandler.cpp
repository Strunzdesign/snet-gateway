/**
 * \file      HdlcdClientHandler.cpp
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

#include "HdlcdClientHandler.h"
#include "HdlcdAccessClient.h"
#include "../Routing/Routing.h"
#include "SnetServiceMessage.h"
#include <assert.h>

HdlcdClientHandler::HdlcdClientHandler(boost::asio::io_service& a_IOService, const std::string& a_DestinationName, const std::string& a_TcpPort, const std::string& a_SerialPortName, Routing* a_pRouting):
    m_IOService(a_IOService), m_DestinationName(a_DestinationName), m_TcpPort(a_TcpPort), m_SerialPortName(a_SerialPortName), m_Resolver(a_IOService), m_ConnectionRetryTimer(a_IOService), m_pRouting(a_pRouting) {
    assert(m_pRouting);
    ResolveDestination();
}

void HdlcdClientHandler::Send(const HdlcdPacketData& a_HdlcdPacketData, std::function<void()> a_OnSendDoneCallback) {
    // TODO: check what happens if this is currently not connected, or will be deletet. Starvation?
    m_HdlcdAccessClient->Send(a_HdlcdPacketData, a_OnSendDoneCallback);
}

void HdlcdClientHandler::ResolveDestination() {
    m_Resolver.async_resolve({m_DestinationName, m_TcpPort}, [this](const boost::system::error_code& a_ErrorCode, boost::asio::ip::tcp::resolver::iterator a_EndpointIterator) {
        // Start the HDLCd Access Client
        m_HdlcdAccessClient = std::make_shared<HdlcdAccessClient>(m_IOService, a_EndpointIterator, m_SerialPortName, 0x01);
        
        // On any error, restart after a short delay
        m_HdlcdAccessClient->SetOnClosedCallback([this](){
            m_ConnectionRetryTimer.expires_from_now(boost::posix_time::seconds(2));
            m_ConnectionRetryTimer.async_wait([this](const boost::system::error_code& a_ErrorCode) {
                if (!a_ErrorCode) {
                    // Reestablish the connection to the HDLC Daemon
                    ResolveDestination();
                } // if
            });
        });
        
        m_HdlcdAccessClient->SetOnDataCallback([this](const HdlcdPacketData& a_PacketData){
            SnetServiceMessage l_ServiceMessage;
            if (l_ServiceMessage.Deserialize(a_PacketData.GetData())) {
                m_pRouting->RouteSnetPacket(&l_ServiceMessage);
            } // if
        });
    });
}
