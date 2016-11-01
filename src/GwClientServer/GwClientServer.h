/**
 * \file      GwClientServer.h
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

#ifndef GWCLIENT_SERVER_H
#define GWCLIENT_SERVER_H

#include <memory>
#include <boost/asio.hpp>
#include "FrameEndpoint.h"

class GwClientServer {
public:
    // CTOR, initializer, and resetter
    GwClientServer(boost::asio::io_service& a_IOService, boost::asio::ip::tcp::tcp::socket& a_TcpSocket);
    void SetOnClosedCallback(std::function<void()> a_OnClosedCallback) {
        m_OnClosedCallback = a_OnClosedCallback;
    }
    
    void Start()
    void Close();
    
    // Methods to be called by a gateway client entity

private:
    // Internal helpers
    bool OnFrame(std::shared_ptr<Frame> a_Frame);
    void OnClosed();
    
    // Members
    boost::asio::io_service& m_IOService;

    // The communication end point
    std::shared_ptr<FrameEndpoint> m_FrameEndpoint;
    std::function<void()> m_OnClosedCallback;
};

#endif // GWCLIENT_SERVER_H
