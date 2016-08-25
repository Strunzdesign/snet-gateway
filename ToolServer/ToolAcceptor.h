/**
 * \file      ToolAcceptor.h
 * \brief     This file contains the header declaration of class ClientAcceptor
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

#ifndef TOOL_ACCEPTOR_H
#define TOOL_ACCEPTOR_H

#include <boost/asio.hpp>
#include "ToolHandlerCollection.h"
#include "ToolHandler.h"

using boost::asio::ip::tcp;

/*! \class ToolAcceptor
 *  \brief Class ToolAcceptor
 * 
 *  This class is responsible to accept incoming TCP connections. For each inbound TCP connection, a ClientHandler object is creating
 *  taking full responsibility of the connection. These TCP connections originate from clients that access the HDLCd and contain PDUs
 *  of the HDLCd access protocol.
 */
class ToolAcceptor {
public:
    /*! \brief The constructor of ClientAcceptor objects
     * 
     *  Listener is started directly on instantiation (RAII)
     * 
     *  \param a_IOService the boost IOService object
     *  \param a_usPortNbr the TCP port number to wait for incoming TCP connections
     *  \param a_SerialPortHandlerCollection the collection helper class of serial port handlers responsible for talking to devices using the HDLC protocol 
     */
    ToolAcceptor(boost::asio::io_service& a_IOService, unsigned short a_usPortNbr, ToolHandlerCollection& a_ToolHandlerCollection): m_ToolHandlerCollection(a_ToolHandlerCollection), m_TCPAcceptor(a_IOService, tcp::endpoint(tcp::v4(), a_usPortNbr)), m_TCPSocket(a_IOService) {
        // Create the collection helper regarding accepted clients
        do_accept(); // start accepting TCP connections
    }

private:
    /*! \brief Internal callback to handle a single incoming TCP connection
     * 
     *  In this internal callback function, for each incoming TCP connection a ClientHandler object is created. This handler consumes the TCP connection and adds itself
     *  to the collection of client handlers.
     */
    void do_accept() {
        m_TCPAcceptor.async_accept(m_TCPSocket, [this](boost::system::error_code a_ErrorCode) {
            if (!a_ErrorCode) {
                // Create ClientHandler, store, and start it
                auto l_ToolHandler = std::make_shared<ToolHandler>(m_ToolHandlerCollection, std::move(m_TCPSocket));
                l_ToolHandler->Start();
            } // if

            // Wait for subsequent TCP connections
            do_accept();
        });
    }

    // Members
    ToolHandlerCollection&        m_ToolHandlerCollection;
    tcp::acceptor m_TCPAcceptor; //!< The TCP listener
    tcp::socket m_TCPSocket; //!< One incoming TCP socket
};

#endif // TOOL_ACCEPTOR_H
