/**
 * \file      GatewayAccessServerHandler.h
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

#ifndef GATEWAY_ACCESS_SERVER_HANDLER_H
#define GATEWAY_ACCESS_SERVER_HANDLER_H

#include <memory>
#include <string>
#include <vector>
#include <deque>
#include <boost/asio.hpp>
#include "GatewayAccessProtocol.h"
#include "GatewayAccessServerHandlerCollection.h"
#include "PublishSubscribeService.h"
#include "SnetServiceMessage.h"
class Routing;
class AddressLease;

class GatewayAccessServerHandler: public std::enable_shared_from_this<GatewayAccessServerHandler> {
public:
    GatewayAccessServerHandler(boost::asio::io_service& a_IOService, std::shared_ptr<GatewayAccessServerHandlerCollection> a_GatewayAccessServerHandlerCollection, boost::asio::ip::tcp::socket& a_TCPSocket, std::shared_ptr<Routing> a_RoutingEntity, std::shared_ptr<AddressLease> a_AddressLease);
    
    void Start();
    void Close();

    // Called by the routing entity
    bool Send(const SnetServiceMessage& a_SnetServiceMessage);
    
    void OnPayload(const std::vector<unsigned char> &a_HigherLayerPayload);
    
private:
    // Members
    std::shared_ptr<GatewayAccessServerHandlerCollection> m_GatewayAccessServerHandlerCollection;
    std::shared_ptr<Routing> m_RoutingEntity;
    std::shared_ptr<AddressLease> m_AddressLease;
    PublishSubscribeService m_PublishSubscribeService;
    GatewayAccessProtocol m_GatewayAccessProtocol;
    bool m_Registered;
};

#endif // GATEWAY_ACCESS_SERVER_HANDLER_H
