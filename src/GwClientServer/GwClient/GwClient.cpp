/**
 * \file      GwClient.cpp
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

#include "GwClient.h"
#include "GwClientFrameEscaped.h"
#include "GwClientFrameLength.h"
//include "CommandResponseFrame0100.h"
#include "CommandResponseFrame0101.h"
//include "CommandResponseFrame0110.h"
#include "CommandResponseFrame0111.h"
#include "CommandResponseFrame0300.h"
#include "CommandResponseFrame0301.h"
#include "CommandResponseFrame0302.h"
#include <assert.h>

GwClient::GwClient(boost::asio::io_service& a_IOService, boost::asio::ip::tcp::tcp::socket& a_TcpSocket, E_FRAMING_MODE a_eFramingMode): m_IOService(a_IOService), m_eFramingMode(a_eFramingMode) {
    // Init the frame endpoint
    m_FrameEndpoint = std::make_shared<FrameEndpoint>(a_IOService, a_TcpSocket, 0xF0); // Only check the upper nibble of each first byte
    m_FrameEndpoint->RegisterFrameFactory(GWCLIENT_FRAME_ESCAPED, []()->std::shared_ptr<Frame>{ return GwClientFrameEscaped::CreateDeserializedFrame(); });
    m_FrameEndpoint->RegisterFrameFactory(GWCLIENT_FRAME_LENGTH,  []()->std::shared_ptr<Frame>{ return GwClientFrameLength::CreateDeserializedFrame (); });
    m_FrameEndpoint->SetOnFrameCallback  ([this](std::shared_ptr<Frame> a_Frame)->bool{ return OnFrame(a_Frame); });
    m_FrameEndpoint->SetOnClosedCallback ([this](){ OnClosed(); });
}

void GwClient::Start() {
    m_FrameEndpoint->Start();
}

void GwClient::Close() {
    // Close entities
    m_FrameEndpoint->Close();
    if (m_OnClosedCallback) {
        m_OnClosedCallback();
    } // if
}

bool GwClient::Send(const std::vector<unsigned char> &a_HigherLayerPayload, std::function<void()> a_OnSendDoneCallback) {
    bool l_bRetVal = false;
    if (m_eFramingMode == FRAMING_MODE_UNKNOWN) {
        // The framing mode is still unknown. Consume the packet without transmission.
        l_bRetVal = true;
        if (a_OnSendDoneCallback) {
            m_IOService.post([a_OnSendDoneCallback](){ a_OnSendDoneCallback(); });
        } // if
    } else if (m_eFramingMode == FRAMING_MODE_ESCAPING) {
        // Send the provided payload via an escaping-based frame
        if (m_FrameEndpoint) {
            CommandResponseFrame0302 l_CommandResponseFrame0302;
            l_CommandResponseFrame0302.m_Payload = std::move(a_HigherLayerPayload);
            l_bRetVal = m_FrameEndpoint->SendFrame(GwClientFrameEscaped::Create(l_CommandResponseFrame0302.Serialize()), a_OnSendDoneCallback);
        } else {
            l_bRetVal = true;
            if (a_OnSendDoneCallback) {
                m_IOService.post([a_OnSendDoneCallback](){ a_OnSendDoneCallback(); });
            } // if
        } // else
    } else if (m_eFramingMode == FRAMING_MODE_LENGTH) {
        // Send the provided payload via a length-based frame
        if (m_FrameEndpoint) {
            l_bRetVal = m_FrameEndpoint->SendFrame(GwClientFrameLength::Create(a_HigherLayerPayload), a_OnSendDoneCallback);
        } else {
            l_bRetVal = true;
            if (a_OnSendDoneCallback) {
                m_IOService.post([a_OnSendDoneCallback](){ a_OnSendDoneCallback(); });
            } // if
        } // else
    } else {
        // Invalid framing mode
        assert(false);
        l_bRetVal = true;
        if (a_OnSendDoneCallback) {
            m_IOService.post([a_OnSendDoneCallback](){ a_OnSendDoneCallback(); });
        } // if
    } // else

    return l_bRetVal;
}

bool GwClient::OnFrame(std::shared_ptr<Frame> a_Frame) {
    assert(a_Frame);
    auto l_GwClientFrame = std::dynamic_pointer_cast<GwClientFrame>(a_Frame);
    assert(l_GwClientFrame);
    switch (l_GwClientFrame->GetGwClientFrameType()) {
        case GWCLIENT_FRAME_ESCAPED: {
            // Check and set mode
            if ((m_eFramingMode == FRAMING_MODE_ESCAPING) || (m_eFramingMode == FRAMING_MODE_UNKNOWN)) {
                m_eFramingMode = FRAMING_MODE_ESCAPING;

                // Parse the received byte buffer to get the command response frame
                auto l_GwClientFrameEscaped = std::dynamic_pointer_cast<GwClientFrameEscaped>(a_Frame);
                assert(l_GwClientFrameEscaped);
                uint16_t l_RequestId = ((uint16_t(l_GwClientFrameEscaped->GetPayload()[0]) << 8) + l_GwClientFrameEscaped->GetPayload()[1]);
                if (l_RequestId == 0x0100) {
                    // We have to send a respose now. TODO: congestions?
                    if (m_FrameEndpoint) {
                        CommandResponseFrame0101 l_CommandResponseFrame0101;
                        (void)m_FrameEndpoint->SendFrame(GwClientFrameEscaped::Create(l_CommandResponseFrame0101.Serialize()));
                    } // if
                } else if (l_RequestId == 0x0110) {
                    // We have to send a respose now. TODO: congestions?
                    CommandResponseFrame0111 l_CommandResponseFrame0111;
                    (void)m_FrameEndpoint->SendFrame(GwClientFrameEscaped::Create(l_CommandResponseFrame0111.Serialize()));
                } else if (l_RequestId == 0x0300) {
                    // We have to send a respose now. TODO: congestions?
                    CommandResponseFrame0301 l_CommandResponseFrame0301;
                    (void)m_FrameEndpoint->SendFrame(GwClientFrameEscaped::Create(l_CommandResponseFrame0301.Serialize()));
                    
                    // Deliver higher-layer payload via callback
                    if (m_OnDataCallback) {
                        CommandResponseFrame0300 l_CommandResponseFrame0300;
                        l_CommandResponseFrame0300.m_Payload = l_GwClientFrameEscaped->GetPayload();
                        m_OnDataCallback(l_CommandResponseFrame0300.GetPayload());
                    } // if
                } else {
                    // Error: Invalid request ID
                    Close();
                } // else
            } else {
                // Error: protocol violation!
                Close();
            } // else

            break;
        }
        case GWCLIENT_FRAME_LENGTH: {
            // Check and set mode
            if ((m_eFramingMode == FRAMING_MODE_LENGTH) || (m_eFramingMode == FRAMING_MODE_UNKNOWN)) {
                m_eFramingMode = FRAMING_MODE_LENGTH;
                
                // Deliver higher-layer payload via callback
                if (m_OnDataCallback) {
                    auto l_GwClientFrameLength = std::dynamic_pointer_cast<GwClientFrameLength>(a_Frame);
                    assert(l_GwClientFrameLength);
                    m_OnDataCallback(l_GwClientFrameLength->GetPayload());
                } // if
            } else {
                // Error: protocol violation!
                Close();
            } // else
            
            break;
        }
        default:
            // Error: wrong frame
            assert(false);
            break;
    } // switch
    
    // TODO: add congestion control
    return true;
}

void GwClient::OnClosed() {
    Close();
}
