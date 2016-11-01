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
#include "GwClientHandlerCollection.h"
#include "HdlcdClientHandlerCollection.h"
#include <assert.h>

Routing::Routing(std::shared_ptr<GwClientHandlerCollection> a_GwClientHandlerCollection, std::shared_ptr<HdlcdClientHandlerCollection> a_HdlcdClientHandlerCollection, bool a_bTrace, bool a_bReliable):
    m_GwClientHandlerCollection(a_GwClientHandlerCollection), m_HdlcdClientHandlerCollection(a_HdlcdClientHandlerCollection), m_bTrace(a_bTrace), m_bReliable(a_bReliable) {
    // Checks
    assert(m_GwClientHandlerCollection);
    assert(m_HdlcdClientHandlerCollection);
}

void Routing::SystemShutdown() {
    // Drop all shared pointers
    m_GwClientHandlerCollection.reset();
    m_HdlcdClientHandlerCollection.reset();
}

void Routing::RouteSnetPacket(SnetServiceMessage& a_SnetServiceMessage, E_COMPONENT a_eSrcComponent) const {
    auto l_eDstComponent = PerformRouting(a_eSrcComponent, a_SnetServiceMessage.GetSrcSSA(), a_SnetServiceMessage.GetDstSSA());
    if (l_eDstComponent == COMPONENT_HDLCD) {
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
    } else if (l_eDstComponent == COMPONENT_GWCLIENTS) {
        // To the tools
        if (m_bTrace) {
            std::cout << "To clients:   " << a_SnetServiceMessage.Dissect() << std::endl;
        } // if

        if (m_GwClientHandlerCollection) {
            m_GwClientHandlerCollection->Send(a_SnetServiceMessage);
        } // if
    } else {
        // Not caught
    } // else
}

E_COMPONENT Routing::PerformRouting(E_COMPONENT a_eSrcComponent, uint16_t a_SrcSSA, uint16_t a_DstSSA) const {
    E_COMPONENT l_eDstComponent(COMPONENT_UNKNOWN);
    if ((a_DstSSA == 0x3FFc) || ((a_DstSSA == 0x4000) && (a_SrcSSA >= 0x4000))) {
        // Not caught (yet)
    } else if ((a_DstSSA < 0x4000) || ((a_DstSSA == 0xFFFE) && (a_eSrcComponent == COMPONENT_GWCLIENTS))) {
        // Deliver to the HDLCd
        l_eDstComponent = COMPONENT_HDLCD;
    } else {
        // Deliver to the tools
        l_eDstComponent = COMPONENT_GWCLIENTS;
    } // else
    
    return l_eDstComponent;
}
