/**
 * \file      main-snet-gateway.cpp
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

#include "Config.h"
#include <iostream>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <boost/regex.hpp>
#include "HdlcdClientHandlerCollection.h"
#include "ToolHandlerCollection.h"
#include "Routing.h"

int main(int argc, char* argv[]) {
    try {
        // Declare the supported options.
        boost::program_options::options_description l_Description("Allowed options");
        l_Description.add_options()
            ("help,h",    "produce this help message")
            ("version,v", "show version information")
            ("port,p",    boost::program_options::value<uint16_t>(),
                          "the TCP port to accept clients on")
            ("connect,c", boost::program_options::value<std::vector<std::string>>()->multitoken(),
                          "connect to devices via the HDLCd\n"
                          "syntax: SerialPort@IPAddess:PortNbr\n"
                          "  linux:   /dev/ttyUSB0@localhost:5001\n"
                          "  windows: //./COM1@example.com:5001")
            ("trace,t",   "each relayed packet is dissected and printed")
        ;

        // Parse the command line
        boost::program_options::variables_map l_VariablesMap;
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, l_Description), l_VariablesMap);
        boost::program_options::notify(l_VariablesMap);
        if (l_VariablesMap.count("version")) {
            std::cerr << "s-net(r) gateway version " << SNET_GATEWAY_VERSION_MAJOR << "." << SNET_GATEWAY_VERSION_MINOR
                      << " built with hdlcd-devel version " << HDLCD_DEVEL_VERSION_MAJOR << "." << HDLCD_DEVEL_VERSION_MINOR
                      << " and snet-devel version " << SNET_DEVEL_VERSION_MAJOR << "." << SNET_DEVEL_VERSION_MINOR << std::endl;
        } // if

        if (l_VariablesMap.count("help")) {
            std::cout << l_Description << std::endl;
            std::cout << "The s-net(r) gateway software for the HDLC Daemon is Copyright (C) 2016, and GNU GPL'd, by Florian Evers." << std::endl;
            std::cout << "Bug reports, feedback, admiration, abuse, etc, to: https://github.com/Strunzdesign/satellite-gateway" << std::endl;
            return 1;
        } // if
                
        if (!l_VariablesMap.count("port")) {
            std::cout << "you have to specify the TCP listener port" << std::endl;
            return 1;
        } // if
        
        if (!l_VariablesMap.count("connect")) {
            std::cout << "you have to specify at least one device to connect to!" << std::endl;
            return 1;
        } // if
        
        // Install signal handlers
        boost::asio::io_service l_IoService;
        boost::asio::signal_set l_Signals(l_IoService);
        l_Signals.add(SIGINT);
        l_Signals.add(SIGTERM);
        l_Signals.async_wait([&l_IoService](boost::system::error_code, int){ l_IoService.stop(); });

        // Create main components
        auto l_ToolHandlerCollection        = std::make_shared<ToolHandlerCollection>(l_IoService, l_VariablesMap["port"].as<uint16_t>());
        auto l_HdlcdClientHandlerCollection = std::make_shared<HdlcdClientHandlerCollection>(l_IoService);
        
        // Routing entity
        auto l_RoutingEntity = std::make_shared<Routing>(l_ToolHandlerCollection, l_HdlcdClientHandlerCollection, l_VariablesMap.count("trace"));
        l_ToolHandlerCollection->Initialize(l_RoutingEntity);
        l_HdlcdClientHandlerCollection->Initialize(l_RoutingEntity);
        
        // Create HDLCd client entities
        auto l_DestSpecifiers(l_VariablesMap["connect"].as<std::vector<std::string>>());
        for (auto l_DestSpecifier = l_DestSpecifiers.begin(); l_DestSpecifier != l_DestSpecifiers.end(); ++l_DestSpecifier) {
            // Parse each destination specifier
            static boost::regex s_RegEx("^(.*?)@(.*?):(.*?)$");
            boost::smatch l_Match;
            if (boost::regex_match(*l_DestSpecifier, l_Match, s_RegEx)) {
                l_HdlcdClientHandlerCollection->CreateHdlcdClientHandler(l_Match[2], l_Match[3], l_Match[1]);
            } else {
                throw boost::program_options::validation_error(boost::program_options::validation_error::invalid_option_value);
            } // else
        } // for
        
        // Start event processing
        l_IoService.run();
        
        // Shutdown
        l_HdlcdClientHandlerCollection->SystemShutdown();
        l_ToolHandlerCollection->SystemShutdown();
        l_RoutingEntity->SystemShutdown();
    } catch (std::exception& a_Error) {
        std::cerr << "Exception: " << a_Error.what() << "\n";
        return 1;
    } // catch

    return 0;
}
