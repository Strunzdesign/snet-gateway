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
class Routing;

class ToolHandler: public std::enable_shared_from_this<ToolHandler> {
public:
    ToolHandler(ToolHandlerCollection& a_ToolHandlerCollection, boost::asio::ip::tcp::socket a_TCPSocket);
    ~ToolHandler();
    void RegisterRoutingEntity(Routing* a_pRoutingEntity);
    
    void DeliverBufferToTools(const std::vector<unsigned char> &a_Payload);
    void QueryForPayload();
    
    void Start();
    void Stop();
    bool Send(const ToolFrame* a_pToolFrame, std::function<void()> a_OnSendDoneCallback = std::function<void()>());
    
    void InterpretDeserializedToolFrame(std::shared_ptr<ToolFrame> a_ToolFrame);
    
private:
    // Internal helpers
    void ReadChunkFromSocket();
    void DoWrite();

    // Members
    Routing* m_pRoutingEntity;
    ToolHandlerCollection& m_ToolHandlerCollection;
    boost::asio::ip::tcp::socket m_TCPSocket;
    bool m_Registered;

    unsigned char m_ReadBuffer[1];
    ToolFrameParser m_ToolFrameParser;
    
    bool m_bWriteInProgress;
    std::deque<std::pair<std::vector<unsigned char>, std::function<void()>>> m_SendQueue; // To be transmitted
};

#endif // TOOL_HANDLER_H
