/**
 * \file      HdlcdClientHandler.h
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

#ifndef HDLCD_CLIENT_HANDLER_H
#define HDLCD_CLIENT_HANDLER_H

#include <memory>
#include <vector>
#include <string>
#include <boost/asio.hpp>
#include "HdlcdPacketData.h"
class HdlcdAccessClient;
class Routing;

class HdlcdClientHandler {
public:
    HdlcdClientHandler(boost::asio::io_service& a_IOService, const std::string& a_DestinationName, const std::string& a_TcpPort, const std::string& a_SerialPortName, Routing* a_pRouting);
    void Send(const HdlcdPacketData& a_HdlcdPacketData, std::function<void()> a_OnSendDoneCallback = std::function<void()>());
    
private:
    // Helpers
    void ResolveDestination();
    
    // Members
    boost::asio::io_service& m_IOService;
    const std::string m_DestinationName;
    const std::string m_TcpPort;
    const std::string m_SerialPortName;
    
    // Resolver
    boost::asio::ip::tcp::resolver m_Resolver;
    boost::asio::deadline_timer m_ConnectionRetryTimer;
    
    // The connection to the HDLC Daemon
    std::shared_ptr<HdlcdAccessClient> m_HdlcdAccessClient;
    
    Routing* m_pRouting;
};

#endif // HDLCD_CLIENT_HANDLER_H
