/**
 * \file      GwClient.h
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

#ifndef GWCLIENT_H
#define GWCLIENT_H

#include <memory>
#include <boost/asio.hpp>
#include "FrameEndpoint.h"

class GwClient {
public:
    // Supported framing modes
    typedef enum {
        FRAMING_MODE_UNKNOWN  = 0,
        FRAMING_MODE_ESCAPING = 1,
        FRAMING_MODE_LENGTH   = 2
    } E_FRAMING_MODE;

    // CTOR, initializer, and resetter
    GwClient(boost::asio::io_service& a_IOService, boost::asio::ip::tcp::tcp::socket& a_TcpSocket, E_FRAMING_MODE a_eFramingMode = FRAMING_MODE_UNKNOWN);
    
    void SetOnDataCallback(std::function<void(const std::vector<unsigned char> &)> a_OnDataCallback = nullptr) {
        m_OnDataCallback = a_OnDataCallback;
    }
    
    void SetOnClosedCallback(std::function<void()> a_OnClosedCallback = nullptr) {
        m_OnClosedCallback = a_OnClosedCallback;
    }
    
    void Start();
    void Close();
    bool Send(const std::vector<unsigned char> &a_HigherLayerPayload, std::function<void()> a_OnSendDoneCallback = nullptr);

private:
    // Internal helpers
    bool OnFrame(std::shared_ptr<Frame> a_Frame);
    void OnClosed();
    
    // Members
    boost::asio::io_service& m_IOService;

    // The communication end point
    std::shared_ptr<FrameEndpoint> m_FrameEndpoint;
    std::function<void(const std::vector<unsigned char>&)> m_OnDataCallback;
    std::function<void()> m_OnClosedCallback;
    E_FRAMING_MODE m_eFramingMode;
};

#endif // GWCLIENT_H
