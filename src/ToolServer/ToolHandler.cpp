/**
 * \file      ToolHandler.cpp
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

#include "ToolHandler.h"
#include "ToolFrameGenerator.h"
#include "Routing.h"
#include "SnetServiceMessage.h"
#include "AddressLease.h"
#include "AddressService.h"
#include <iostream>
#include <assert.h>

ToolHandler::ToolHandler(std::shared_ptr<ToolHandlerCollection> a_ToolHandlerCollection, boost::asio::ip::tcp::socket& a_TCPSocket):
    m_ToolHandlerCollection(a_ToolHandlerCollection),
    m_TCPSocket(std::move(a_TCPSocket)),
    m_ToolFrameParser(*this) {
    // Checks
    assert(m_ToolHandlerCollection);
        
    m_Registered = false;
    m_bWriteInProgress = false;
    m_SendBufferOffset = 0;
}

void ToolHandler::RegisterRoutingEntity(std::shared_ptr<Routing> a_RoutingEntity) {
    assert(a_RoutingEntity);
    m_RoutingEntity = a_RoutingEntity;
}

void ToolHandler::Start() {
    assert(m_Registered == false);
    m_Registered = true;
    m_SendBufferOffset = 0;
    assert(!m_AddressLease);
    m_AddressLease = m_ToolHandlerCollection->RegisterToolHandler(shared_from_this());
    ReadChunkFromSocket();
}

void ToolHandler::Close() {
    // Keep this object alive
    auto self(shared_from_this());
    if (m_Registered) {
        m_Registered = false;
        m_TCPSocket.cancel();
        m_TCPSocket.close();
        assert(m_AddressLease);
        m_AddressLease.reset(); // Deregisters itself
        m_ToolHandlerCollection->DeregisterToolHandler(self);
        m_ToolHandlerCollection.reset();
    } // if
}

bool ToolHandler::Send(const SnetServiceMessage& a_SnetServiceMessage) {
    // Check destination address
    if (((a_SnetServiceMessage.GetDstSSA() > 0x4000) && (a_SnetServiceMessage.GetDstSSA() < 0xFFF0)) && (a_SnetServiceMessage.GetDstSSA() != m_AddressLease->GetAddress())) {
        // Not for this tool!
        return false;
    } // if
    
    // Check subscription
    if (m_PublishSubscribeService.IsServiceIdForMe(a_SnetServiceMessage.GetSrcServiceId()) == false) {
        // Subscription does not fit!
        return false;
    } // if
        
    // Create a copy and change the destination address
    auto l_SnetServiceMessage(a_SnetServiceMessage);
    l_SnetServiceMessage.SetDstSSA(m_AddressLease->GetAddress());
    return SendHelper(l_SnetServiceMessage);
}

bool ToolHandler::SendHelper(const SnetServiceMessage& a_SnetServiceMessage) {
    // Queue for transmission :-)
    ToolFrame0302 l_ToolFrame0302;
    l_ToolFrame0302.m_Payload = a_SnetServiceMessage.Serialize();
    return Send(l_ToolFrame0302);
}

bool ToolHandler::Send(const ToolFrame& a_ToolFrame) {
    // TODO: check size of the queue. If it reaches a specific limit: kill the socket to prevent DoS attacks
    if (m_SendQueue.size() >= 100) {
        return false;
    } // if
    
    m_SendQueue.emplace_back(ToolFrameGenerator::EscapeFrame(a_ToolFrame.SerializeFrame()));
    if ((!m_bWriteInProgress) && (!m_SendQueue.empty())) {
        DoWrite();
    } // if
    
    return true;
}
    
void ToolHandler::ReadChunkFromSocket() {
    m_TCPSocket.async_read_some(boost::asio::buffer(m_ReadBuffer, E_MAX_LENGTH),[this](boost::system::error_code a_ErrorCode, std::size_t a_ReadBytes) {
        if (a_ErrorCode == boost::asio::error::operation_aborted) return;
        if (!m_Registered) return;
        if (!a_ErrorCode) {
            m_ToolFrameParser.AddReceivedRawBytes(m_ReadBuffer, a_ReadBytes);
            ReadChunkFromSocket();
        } else {
            std::cerr << "TCP read error on gateway client side, error=" << a_ErrorCode << std::endl;
            Close();
        } // else
    }); // async_read
}

void ToolHandler::InterpretDeserializedToolFrame(const std::shared_ptr<ToolFrame> a_ToolFrame) {
    if (!a_ToolFrame) { return; }
    if (a_ToolFrame->GetRequestId() == 0x0100) {
        // We have to send a respose now
        ToolFrame0101 l_ToolFrame0101;
        Send(l_ToolFrame0101);
    } // if
    
    if (a_ToolFrame->GetRequestId() == 0x0110) {
        // We have to send a respose now
        ToolFrame0111 l_ToolFrame0111;
        Send(l_ToolFrame0111);
    } // if
    
    if (a_ToolFrame->GetRequestId() == 0x0300) {
        // We have to send a respose now
        ToolFrame0301 l_ToolFrame0301;
        Send(l_ToolFrame0301);
        
        // Relay the payload
        SnetServiceMessage l_ServiceMessage;
        if (l_ServiceMessage.Deserialize(a_ToolFrame->GetPayload())) {
            // Check if it is directed to the address service
            if (l_ServiceMessage.GetDstSSA() == 0x3FFC) {
                auto l_AddressAssignmentReply = AddressService::ProcessRequest(l_ServiceMessage, m_AddressLease);
                if (l_AddressAssignmentReply.GetSrcServiceId() == 0xAE) {
                    SendHelper(l_AddressAssignmentReply);
                } // if

                return;
            } // if
            
            // Check if it is directed to the publish / subsribe service
            if (l_ServiceMessage.GetDstSSA() == 0x4000) {
                auto l_PublishSubscribeConfirmation = m_PublishSubscribeService.ProcessRequest(l_ServiceMessage, m_AddressLease);
                if (l_PublishSubscribeConfirmation.GetDstServiceId() == 0xB0) {
                    SendHelper(l_PublishSubscribeConfirmation);
                } // if

                return;
            } // if
            
            // Remap the source address
            if (l_ServiceMessage.GetSrcSSA() != 0xFFFE) {
                l_ServiceMessage.SetSrcSSA(m_AddressLease->GetAddress());
            } // if
            
            // Route this packet
            m_RoutingEntity->RouteSnetPacket(l_ServiceMessage);
        } // if
    } // if
}

void ToolHandler::DoWrite() {
    auto self(shared_from_this());
    if (!m_Registered) return;
    m_bWriteInProgress = true;
    boost::asio::async_write(m_TCPSocket, boost::asio::buffer(&(m_SendQueue.front().data()[m_SendBufferOffset]), (m_SendQueue.front().size() - m_SendBufferOffset)),
                                 [this, self](boost::system::error_code a_ErrorCode, std::size_t a_BytesSent) {
        if (a_ErrorCode == boost::asio::error::operation_aborted) return;
        if (!m_Registered) return;
        if (!a_ErrorCode) {
            m_SendBufferOffset += a_BytesSent;
            if (m_SendBufferOffset == m_SendQueue.front().size()) {
                // Completed transmission. Remove the transmitted packet
                m_SendQueue.pop_front();
                m_SendBufferOffset = 0;
                if (!m_SendQueue.empty()) {
                    DoWrite();
                } else {
                    m_bWriteInProgress = false;
                } // else
            } else {
                // Only a partial transmission. We are not done yet.
                DoWrite();
            } // else
        } else {
            std::cerr << "TCP write error on gateway client side, error=" << a_ErrorCode << std::endl;
            Close();
        } // else
    }); // async_write
}
