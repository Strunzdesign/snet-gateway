/**
 * \file      Routing.h
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

#ifndef ROUTING_H
#define ROUTING_H

#include <memory>
#include <vector>
class SnetServiceMessage;
class ToolHandlerCollection;
class HdlcdClientHandlerCollection;

class Routing {
public:
    // CTOR and resetter
    Routing(std::shared_ptr<ToolHandlerCollection> a_ToolHandlerCollection, std::shared_ptr<HdlcdClientHandlerCollection> a_HdlcdClientHandlerCollection, bool a_bTrace, bool a_bReliable);
    void SystemShutdown();

    void RouteSnetPacket(SnetServiceMessage* a_pSnetServiceMessage);

private:
    // Members
    std::shared_ptr<ToolHandlerCollection>        m_ToolHandlerCollection;
    std::shared_ptr<HdlcdClientHandlerCollection> m_HdlcdClientHandlerCollection;
    bool m_bTrace;
    bool m_bReliable;
};

#endif // ROUTING_H
