/**
 * \file      Routing.cpp
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

#include "Routing.h"
#include "SnetServiceMessage.h"
#include "ToolHandlerCollection.h"
#include "HdlcdClientHandlerCollection.h"
#include <assert.h>

Routing::Routing(std::shared_ptr<ToolHandlerCollection> a_ToolHandlerCollection, std::shared_ptr<HdlcdClientHandlerCollection> a_HdlcdClientHandlerCollection, bool a_bTrace, bool a_bReliable):
    m_ToolHandlerCollection(a_ToolHandlerCollection), m_HdlcdClientHandlerCollection(a_HdlcdClientHandlerCollection), m_bTrace(a_bTrace), m_bReliable(a_bReliable) {
    // Checks
    assert(m_ToolHandlerCollection);
    assert(m_HdlcdClientHandlerCollection);
}

void Routing::SystemShutdown() {
    // Drop all shared pointers
    m_ToolHandlerCollection.reset();
    m_HdlcdClientHandlerCollection.reset();
}

void Routing::RouteSnetPacket(SnetServiceMessage& a_SnetServiceMessage, E_COMPONENT a_eSourceComponent) const {
    auto l_eDestination = PerformRouting(a_eSourceComponent, a_SnetServiceMessage.GetSrcSSA(), a_SnetServiceMessage.GetDstSSA());
    if (l_eDestination == COMPONENT_HDLCD) {
        // To the HDLCd
        if (m_bReliable) {
            // Explicitely demand for OnAirARQ for all outgoing s-net packets
            a_SnetServiceMessage.SetOnAirARQ(true);
        } // if

        if (m_bTrace) {
            std::cout << "To the HDLCd: " << a_SnetServiceMessage.Dissect() << std::endl;
        } // if
        
        if (m_HdlcdClientHandlerCollection) {
            m_HdlcdClientHandlerCollection->Send(HdlcdPacketData::CreatePacket(a_SnetServiceMessage.Serialize(), true));
        } // if
    } else if (l_eDestination == COMPONENT_TOOLS) {
        // To the tools
        if (m_bTrace) {
            std::cout << "To clients:   " << a_SnetServiceMessage.Dissect() << std::endl;
        } // if

        if (m_ToolHandlerCollection) {
            m_ToolHandlerCollection->Send(a_SnetServiceMessage);
        } // if
    } else {
        // Not caught
    } // else
}

E_COMPONENT Routing::PerformRouting(E_COMPONENT a_eSourceComponent, uint16_t a_SrcSSA, uint16_t a_DstSSA) const {
    if ((a_DstSSA == 0x3FFc) || ((a_DstSSA == 0x4000) && (a_SrcSSA >= 0x4000))) {
        // Not caught (yet)
        return COMPONENT_UNKNOWN;
    } else if ((a_DstSSA < 0x4000) || ((a_DstSSA == 0xFFFE) && (a_eSourceComponent == COMPONENT_TOOLS))) {
        // Deliver to the HDLCd
        return COMPONENT_HDLCD;
    } else {
        // Deliver to the tools
        return COMPONENT_TOOLS;
    } // else
}
