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
#include "HdlcdClient.h"
#include "../Routing/Routing.h"
#include "SnetServiceMessage.h"
#include <assert.h>
using boost::asio::ip::tcp;

HdlcdClientHandler::HdlcdClientHandler(boost::asio::io_service& a_IOService, const std::string& a_DestinationName, const std::string& a_TcpPort, const std::string& a_SerialPortName,
                                       std::shared_ptr<Routing> a_RoutingEntity):
    m_IOService(a_IOService), m_DestinationName(a_DestinationName), m_TcpPort(a_TcpPort), m_SerialPortName(a_SerialPortName), m_Resolver(a_IOService), m_ConnectionRetryTimer(a_IOService), m_RoutingEntity(a_RoutingEntity) {
    // Checks
    assert(m_RoutingEntity);
    
    // Trigger activity
    ResolveDestination();
}

void HdlcdClientHandler::Close() {
    // Drop all shared pointers
    m_RoutingEntity.reset();
    if (m_HdlcdClient) {
        m_HdlcdClient->Close();
        m_HdlcdClient.reset();
    } // if
}

void HdlcdClientHandler::Send(const HdlcdPacketData& a_HdlcdPacketData) {
    if (m_HdlcdClient) {
        m_HdlcdClient->Send(a_HdlcdPacketData);
    } // if
}

void HdlcdClientHandler::ResolveDestination() {
    m_Resolver.async_resolve({m_DestinationName, m_TcpPort}, [this](const boost::system::error_code& a_ErrorCode, boost::asio::ip::tcp::resolver::iterator a_EndpointIterator) {
        if (a_ErrorCode) {
            std::cout << "Failed to resolve host name: " << m_DestinationName << std::endl;
            m_ConnectionRetryTimer.expires_from_now(boost::posix_time::seconds(2));
            m_ConnectionRetryTimer.async_wait([this](const boost::system::error_code& a_ErrorCode) {
                if (!a_ErrorCode) {
                    // Reestablish the connection to the HDLC Daemon
                    ResolveDestination();
                } // if
            }); // async_wait
        } else {
            // Start the HDLCd access client. On any error, restart after a short delay
            m_HdlcdClient = std::make_shared<HdlcdClient>(m_IOService, m_SerialPortName, HdlcdSessionDescriptor(SESSION_TYPE_TRX_ALL, SESSION_FLAGS_DELIVER_RCVD));
            m_HdlcdClient->SetOnClosedCallback([this]() {
                m_ConnectionRetryTimer.expires_from_now(boost::posix_time::seconds(2));
                m_ConnectionRetryTimer.async_wait([this](const boost::system::error_code& a_ErrorCode) {
                    if (!a_ErrorCode) {
                        // Reestablish the connection to the HDLC Daemon
                        ResolveDestination();
                    } // if
                }); // async_wait
            }); // SetOnClosedCallback
            
            m_HdlcdClient->SetOnDataCallback([this](const HdlcdPacketData& a_PacketData){
                SnetServiceMessage l_ServiceMessage;
                if (l_ServiceMessage.Deserialize(a_PacketData.GetData())) {
                    m_RoutingEntity->RouteSnetPacket(&l_ServiceMessage);
                } // if
            }); // SetOnDataCallback
            
            // Connect
            m_HdlcdClient->AsyncConnect(a_EndpointIterator, [this](bool a_bSuccess) {
                if (!a_bSuccess) {
                    std::cout << "Failed to connect to the HDLC Daemon!" << std::endl;
                    m_ConnectionRetryTimer.expires_from_now(boost::posix_time::seconds(2));
                    m_ConnectionRetryTimer.async_wait([this](const boost::system::error_code& a_ErrorCode) {
                        if (!a_ErrorCode) {
                            // Reestablish the connection to the HDLC Daemon
                            ResolveDestination();
                        } // if
                    }); // async_wait
                } // if
            }); // AsyncConnect
        } // else
    }); // async_resolve
}
