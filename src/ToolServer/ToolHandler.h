/**
 * \file      ToolHandler.h
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

#ifndef TOOL_HANDLER_H
#define TOOL_HANDLER_H

#include <memory>
#include <string>
#include <vector>
#include <deque>
#include <boost/asio.hpp>
#include "ToolHandlerCollection.h"
#include "ToolFrameParser.h"
#include "PublishSubscribeService.h"
#include "SnetServiceMessage.h"
class Routing;
class AddressLease;

class ToolHandler: public std::enable_shared_from_this<ToolHandler> {
public:
    ToolHandler(std::shared_ptr<ToolHandlerCollection> a_ToolHandlerCollection, boost::asio::ip::tcp::socket& a_TCPSocket);
    void RegisterRoutingEntity(std::shared_ptr<Routing> a_RoutingEntity);
    
    void Start();
    void Close();

    bool Send(const SnetServiceMessage& a_SnetServiceMessage);
    void InterpretDeserializedToolFrame(const std::shared_ptr<ToolFrame> a_ToolFrame);
    
private:
    // Internal helpers
    bool SendHelper(const SnetServiceMessage& a_SnetServiceMessage);
    bool Send(const ToolFrame& a_ToolFrame);
    void ReadChunkFromSocket();
    void DoWrite();

    // Members
    std::shared_ptr<Routing> m_RoutingEntity;
    std::shared_ptr<ToolHandlerCollection> m_ToolHandlerCollection;
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

#endif // TOOL_HANDLER_H
