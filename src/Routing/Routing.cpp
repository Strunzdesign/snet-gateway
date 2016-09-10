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
#include "../ToolServer/ToolHandlerCollection.h"
#include "../HdlcdClient/HdlcdClientHandlerCollection.h"
#include <assert.h>

Routing::Routing(std::shared_ptr<ToolHandlerCollection> a_ToolHandlerCollection, std::shared_ptr<HdlcdClientHandlerCollection> a_HdlcdClientHandlerCollection, bool a_bTrace):
    m_ToolHandlerCollection(a_ToolHandlerCollection), m_HdlcdClientHandlerCollection(a_HdlcdClientHandlerCollection), m_bTrace(a_bTrace) {
    // Checks
    assert(m_ToolHandlerCollection);
    assert(m_HdlcdClientHandlerCollection);
}

void Routing::SystemShutdown() {
    // Drop all shared pointers
    m_ToolHandlerCollection.reset();
    m_HdlcdClientHandlerCollection.reset();
}

void Routing::RouteSnetPacket(SnetServiceMessage* a_pSnetServiceMessage) {
    uint16_t l_SrcSSA = a_pSnetServiceMessage->GetSrcSSA();
    uint16_t l_DstSSA = a_pSnetServiceMessage->GetDstSSA();
    if ((l_DstSSA == 0x3FFc) || ((l_DstSSA == 0x4000) && (l_SrcSSA >= 0x4000))) {
        // Not caught yet
    } else if (l_DstSSA < 0x4000) {
        // To the HDLCd: explicitely demand for OnAirARQ for all outgoing snet packets :-)
        a_pSnetServiceMessage->SetOnAirARQ(true);
        if (m_bTrace) {
            std::cout << "To the HDLCd: " << a_pSnetServiceMessage->Dissect() << std::endl;
        } // if
        
        if (m_HdlcdClientHandlerCollection) {
            m_HdlcdClientHandlerCollection->Send(std::move(HdlcdPacketData::CreatePacket(a_pSnetServiceMessage->Serialize(), true)));
        } // if
    } else {
        // To the tools
        if (m_bTrace) {
            std::cout << "To clients:   " << a_pSnetServiceMessage->Dissect() << std::endl;
        } // if

        if (m_ToolHandlerCollection) {
            m_ToolHandlerCollection->Send(a_pSnetServiceMessage);
        } // if
    } // else
}
