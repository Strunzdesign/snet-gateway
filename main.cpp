/**
 * \file main.cpp
 * \brief 
 *
 * A set of tools to exchange and handle packets of s-net(r) devices via the HDLC Daemon.
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

#include "Config.h"
#include <iostream>
#include <boost/asio.hpp>
#include "HdlcdClient/HdlcdClientHandlerCollection.h"
#include "ToolServer/ToolAcceptor.h"
#include "Routing/Routing.h"

int main(int argc, char* argv[]) {
    try {
        std::cerr << "s-net(r) gateway v" << SNET_TOOLS_VERSION_MAJOR << "." << SNET_TOOLS_VERSION_MINOR << std::endl;
        if (argc != 1) {
            std::cerr << "Usage: snet-gateway\n";
            return 1;
        } // if

        // Install signal handlers
        boost::asio::io_service io_service;
        boost::asio::signal_set signals_(io_service);
        signals_.add(SIGINT);
        signals_.add(SIGTERM);
        signals_.async_wait([&io_service](boost::system::error_code a_ErrorCode, int a_SignalNumber){io_service.stop();});

        ToolHandlerCollection l_ToolHandlerCollection;
        HdlcdClientHandlerCollection l_HdlcdClientHandlerCollection(io_service);
        ToolAcceptor l_ToolAcceptor(io_service, 10002, l_ToolHandlerCollection);
        
        // Routing entity
        Routing l_Routing(l_ToolHandlerCollection, l_HdlcdClientHandlerCollection);
        l_ToolHandlerCollection.RegisterRoutingEntity(&l_Routing);
        l_HdlcdClientHandlerCollection.RegisterRoutingEntity(&l_Routing);
        
        // Create HDLCd client entities
        l_HdlcdClientHandlerCollection.CreateHdlcdClientHandler("127.0.0.1", "10001", "/dev/ttyUSB0");
        //l_HdlcdClientHandlerCollection.CreateHdlcdClientHandler("127.0.0.1", "10001", "/dev/ttyUSB1");
        //l_HdlcdClientHandlerCollection.CreateHdlcdClientHandler("127.0.0.1", "10001", "/dev/ttyUSB2");
        //l_HdlcdClientHandlerCollection.CreateHdlcdClientHandler("127.0.0.1", "10001", "/dev/ttyUSB3");
        
        // Start event processing
        io_service.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    } // catch

    return 0;
}
