/**
 * \file      HdlcdClientHandlerCollection.h
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

#ifndef HDLCD_CLIENT_HANDLER_COLLECTION_H
#define HDLCD_CLIENT_HANDLER_COLLECTION_H

#include <memory>
#include <vector>
#include <string>
#include <boost/asio.hpp>
#include "HdlcdPacketData.h"
class HdlcdClientHandler;
class Routing;

class HdlcdClientHandlerCollection {
public:
    // CTOR, initializer, and resetter
    HdlcdClientHandlerCollection(boost::asio::io_service& a_IOService);
    void Initialize(std::shared_ptr<Routing> a_RoutingEntity);
    void SystemShutdown();
    
    void CreateHdlcdClientHandler(const std::string& a_DestinationName, const std::string& a_TcpPort, const std::string& a_SerialPortName);
    
    void Send(const HdlcdPacketData& a_HdlcdPacketData);
    
private:
    // Members
    boost::asio::io_service& m_IOService;
    std::vector<std::shared_ptr<HdlcdClientHandler>> m_HdlcdClientHandlerVector;
    std::shared_ptr<Routing> m_RoutingEntity;
};

#endif // HDLCD_CLIENT_HANDLER_COLLECTION_H
