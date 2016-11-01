/**
 * \file      GwClientServerHandler.h
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

#ifndef GWCLIENT_SERVER_HANDLER_H
#define GWCLIENT_SERVER_HANDLER_H

#include <memory>
#include <string>
#include <vector>
#include <deque>
#include <boost/asio.hpp>
#include "GwClientServerHandlerCollection.h"
#include "ToolFrameParser.h"
#include "CommandResponseFrame.h"
#include "PublishSubscribeService.h"
#include "SnetServiceMessage.h"
class Routing;
class AddressLease;

class GwClientServerHandler: public std::enable_shared_from_this<GwClientServerHandler> {
public:
    GwClientServerHandler(std::shared_ptr<GwClientServerHandlerCollection> a_GwClientServerHandlerCollection, boost::asio::ip::tcp::socket& a_TCPSocket);
    void RegisterRoutingEntity(std::shared_ptr<Routing> a_RoutingEntity);
    
    void Start();
    void Close();

    bool Send(const SnetServiceMessage& a_SnetServiceMessage);
    void InterpretDeserializedToolFrame(const std::shared_ptr<CommandResponseFrame> a_CommandResponseFrame);
    
private:
    // Internal helpers
    bool SendHelper(const SnetServiceMessage& a_SnetServiceMessage);
    bool Send(const CommandResponseFrame& a_CommandResponseFrame);
    void ReadChunkFromSocket();
    void DoWrite();

    // Members
    std::shared_ptr<Routing> m_RoutingEntity;
    std::shared_ptr<GwClientServerHandlerCollection> m_GwClientServerHandlerCollection;
    boost::asio::ip::tcp::socket m_TCPSocket;
    std::shared_ptr<AddressLease> m_AddressLease;
    PublishSubscribeService m_PublishSubscribeService;
    
    bool m_Registered;
    enum { E_MAX_LENGTH = 65535 };
    unsigned char m_ReadBuffer[E_MAX_LENGTH];
    ToolFrameParser m_ToolFrameParser;
    
    bool m_bWriteInProgress;
    std::deque<std::vector<unsigned char>> m_SendQueue; // To be transmitted
    size_t m_SendBufferOffset; //!< To detect and handle partial writes to the TCP socket
};

#endif // GWCLIENT_SERVER_HANDLER_H
