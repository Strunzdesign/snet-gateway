/**
 * \file      ToolHandlerCollection.h
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

#ifndef TOOL_HANDLER_COLLECTION_H
#define TOOL_HANDLER_COLLECTION_H

#include <memory>
#include <list>
#include <boost/asio.hpp>
#include "SnetServiceMessage.h"
class ToolHandler;
class Routing;
class AddressPool;
class AddressLease;

class ToolHandlerCollection: public std::enable_shared_from_this<ToolHandlerCollection> {
public:
    // CTOR, initializer, and resetter
    ToolHandlerCollection(boost::asio::io_service& a_IOService, uint16_t a_TcpPortNbr);
    void Initialize(std::shared_ptr<Routing> a_RoutingEntity);
    void SystemShutdown();
    
    // Self-registering and -deregistering of tool handler objects
    std::shared_ptr<AddressLease> RegisterToolHandler(std::shared_ptr<ToolHandler> a_ToolHandler);
    void DeregisterToolHandler(std::shared_ptr<ToolHandler> a_ToolHandler);

    void Send(const SnetServiceMessage& a_SnetServiceMessage);
    
private:
    // Internal helpers
    void DoAccept();

    // Members
    std::shared_ptr<AddressPool> m_AddressPool;
    std::list<std::shared_ptr<ToolHandler>> m_ToolHandlerList;
    std::shared_ptr<Routing> m_RoutingEntity;
    
    // Accept incoming TCP connections
    boost::asio::ip::tcp::tcp::acceptor m_TcpAcceptor; //!< The TCP listener
    boost::asio::ip::tcp::tcp::socket   m_TcpSocket; //!< One incoming TCP socket
};

#endif // TOOL_HANDLER_COLLECTION_H
