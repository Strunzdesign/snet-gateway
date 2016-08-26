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
#include "../Routing/Routing.h"
#include "SnetServiceMessage.h"
#include "AddressLease.h"
#include "AddressService.h"
#include <iostream>
#include <assert.h>

ToolHandler::ToolHandler(ToolHandlerCollection& a_ToolHandlerCollection, boost::asio::ip::tcp::socket a_TCPSocket):
    m_ToolHandlerCollection(a_ToolHandlerCollection),
    m_TCPSocket(std::move(a_TCPSocket)),
    m_ToolFrameParser(*this) {
    m_pRoutingEntity = NULL;
    m_Registered = false;
    m_bWriteInProgress = false;
    std::cout << "CTOR" << std::endl;   
}

ToolHandler::~ToolHandler() {
    std::cout << "DTOR" << std::endl;
    Stop();
}

void ToolHandler::RegisterRoutingEntity(Routing* a_pRoutingEntity) {
    assert(a_pRoutingEntity);
    m_pRoutingEntity = a_pRoutingEntity;
}

void ToolHandler::DeliverBufferToTools(const std::vector<unsigned char> &a_Payload) {
}

void ToolHandler::QueryForPayload() {
}

void ToolHandler::Start() {
    assert(m_Registered == false);
    m_Registered = true;
    assert(!m_AddressLease);
    m_AddressLease = m_ToolHandlerCollection.RegisterToolHandler(shared_from_this());
    ReadChunkFromSocket();
}

void ToolHandler::Stop() {
    if (m_Registered) {
        m_Registered = false;
        m_TCPSocket.cancel();
        m_TCPSocket.close();
        assert(m_AddressLease);
        m_AddressLease.reset(); // Deregisters itself
        m_ToolHandlerCollection.DeregisterToolHandler(shared_from_this());
    } // if
}

bool ToolHandler::Send(SnetServiceMessage* a_pSnetServiceMessage, std::function<void()> a_OnSendDoneCallback) {
    // Check destination address
    if (((a_pSnetServiceMessage->GetDstSSA() > 0x4000) && (a_pSnetServiceMessage->GetDstSSA() < 0xFFF0)) && (a_pSnetServiceMessage->GetDstSSA() != m_AddressLease->GetAddress())) {
        // Not for this tool!
        return false;
    } // if
    
    // Check subscription
    if (m_PublishSubscribeService.IsServiceIdForMe(a_pSnetServiceMessage->GetSrcServiceId()) == false) {
        // Subscription does not fit!
        return false;
    } // if
        
    // Create a copy and change the destination address
    auto l_SnetServiceMessage = *a_pSnetServiceMessage;
    l_SnetServiceMessage.SetDstSSA(m_AddressLease->GetAddress());
    return SendHelper(&l_SnetServiceMessage, a_OnSendDoneCallback);
}

bool ToolHandler::SendHelper(SnetServiceMessage* a_pSnetServiceMessage, std::function<void()> a_OnSendDoneCallback) {        
    // Queue for transmission :-)
    ToolFrame0302 l_ToolFrame0302;
    l_ToolFrame0302.m_Payload = a_pSnetServiceMessage->Serialize();
    return Send(&l_ToolFrame0302, a_OnSendDoneCallback);
}

bool ToolHandler::Send(const ToolFrame* a_pToolFrame, std::function<void()> a_OnSendDoneCallback) {
    assert(a_pToolFrame != NULL);

    // TODO: check size of the queue. If it reaches a specific limit: kill the socket to prevent DoS attacks
    if (m_SendQueue.size() >= 10) {
        if (a_OnSendDoneCallback) {
            a_OnSendDoneCallback();
        } // if

        // TODO: check what happens if this is caused by an important packet, e.g., a keep alive or an echo response packet
        return false;
    } // if
    
    m_SendQueue.emplace_back(std::make_pair(std::move(ToolFrameGenerator::EscapeFrame(a_pToolFrame->SerializeFrame())), a_OnSendDoneCallback));
    if ((!m_bWriteInProgress) && (!m_SendQueue.empty()) /*&& (m_SEPState == SEPSTATE_CONNECTED)*/) {
        DoWrite();
    } // if
    
    return true;
}
    
void ToolHandler::ReadChunkFromSocket() {
    boost::asio::async_read(m_TCPSocket, boost::asio::buffer(m_ReadBuffer, 2),[this](boost::system::error_code a_ErrorCode, std::size_t length) {
        if (a_ErrorCode == boost::asio::error::operation_aborted) return;
        if (!a_ErrorCode) {
            m_ToolFrameParser.AddReceivedRawBytes(m_ReadBuffer, length);
            ReadChunkFromSocket();
        } else {
            std::cerr << "TCP READ ERROR HEADER1:" << a_ErrorCode << std::endl;
            Stop();
        } // else
    });
}

void ToolHandler::InterpretDeserializedToolFrame(std::shared_ptr<ToolFrame> a_ToolFrame) {
    if (!a_ToolFrame) { return; }
    if (a_ToolFrame->GetRequestId() == 0x0100) {
        // We have to send a respose now
        ToolFrame0101 l_ToolFrame0101;
        Send(&l_ToolFrame0101);
    } // if
    
    if (a_ToolFrame->GetRequestId() == 0x0300) {
        // We have to send a respose now
        ToolFrame0301 l_ToolFrame0301;
        Send(&l_ToolFrame0301);
        
        // Relay message
        SnetServiceMessage l_ServiceMessage;
        if (l_ServiceMessage.Deserialize(a_ToolFrame->GetPayload())) {
            // Check if it is directed to the address service
            if (l_ServiceMessage.GetDstSSA() == 0x3FFC) {
                auto l_AddressAssignmentReply = AddressService::ProcessRequest(l_ServiceMessage, m_AddressLease);
                if (l_AddressAssignmentReply.GetSrcServiceId() == 0xAE) {
                    std::cout << "Send packet " << l_AddressAssignmentReply.Dissect() << std::endl;
                    SendHelper(&l_AddressAssignmentReply);
                } // if

                return;
            } // if
            
            // Check if it is directed to the publish / subsribe service
            if (l_ServiceMessage.GetDstSSA() == 0x4000) {
                auto l_PublishSubscribeConfirmation = m_PublishSubscribeService.ProcessRequest(l_ServiceMessage, m_AddressLease);
                if (l_PublishSubscribeConfirmation.GetDstServiceId() == 0xB0) {
                    std::cout << "Send packet " << l_PublishSubscribeConfirmation.Dissect() << std::endl;
                    SendHelper(&l_PublishSubscribeConfirmation);
                } // if

                return;
            } // if
            
            // Source address remapping
            if (l_ServiceMessage.GetSrcSSA() != 0xFFFE) {
                l_ServiceMessage.SetSrcSSA(m_AddressLease->GetAddress());
            } // if
            
            // Route this packet
            m_pRoutingEntity->RouteIncomingSnetPacket(&l_ServiceMessage);
        } // if
    } // if
}

void ToolHandler::DoWrite() {
    auto self(shared_from_this());
//    if (m_bStopped) return;
    m_bWriteInProgress = true;
    boost::asio::async_write(m_TCPSocket, boost::asio::buffer(m_SendQueue.front().first.data(), m_SendQueue.front().first.size()),
                                [this, self](boost::system::error_code a_ErrorCode, std::size_t a_BytesSent) {
        if (a_ErrorCode == boost::asio::error::operation_aborted) return;
//        if (m_bStopped) return;
        if (!a_ErrorCode) {
            // If a callback was provided, call it now to demand for a subsequent packet
            if (m_SendQueue.front().second) {
                m_SendQueue.front().second();
            } // if

            m_SendQueue.pop_front();
            if (!m_SendQueue.empty()) {
                DoWrite();
            } else {
                m_bWriteInProgress = false;
/*                if (m_bShutdown) {
                    m_SEPState = SEPSTATE_SHUTDOWN;
                    m_TCPSocket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
                    Close();
                } // if*/
            } // else
        } else {
            std::cerr << "TCP write error!" << std::endl;
//            Close();
        }
    });
}
