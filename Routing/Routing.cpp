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
#include "../ToolServer/ToolFrame.h"
#include <assert.h>

Routing::Routing(ToolHandlerCollection &a_ToolHandlerCollection, HdlcdClientHandlerCollection &a_HdlcdClientHandlerCollection):
    m_ToolHandlerCollection(a_ToolHandlerCollection), m_HdlcdClientHandlerCollection(a_HdlcdClientHandlerCollection) {
}

void Routing::RouteIncomingSnetPacket(SnetServiceMessage* a_pSnetServiceMessage) {
    std::cout << "Revd packet " << a_pSnetServiceMessage->Dissect() << std::endl;
    
    uint16_t l_SrcSSA = a_pSnetServiceMessage->GetSrcSSA();
    uint16_t l_DstSSA = a_pSnetServiceMessage->GetDstSSA();
    if ((l_DstSSA == 0x3FFc) || ((l_DstSSA == 0x4000) && (l_SrcSSA >= 0x4000))) {
        if ((a_pSnetServiceMessage->GetSrcSSA() == 0x3fff) &&
            (a_pSnetServiceMessage->GetDstSSA() == 0x3ffc) &&
            (a_pSnetServiceMessage->GetSrcServiceId() == 0xaf)   &&
            (a_pSnetServiceMessage->GetDstServiceId() == 0xae)   &&
            (a_pSnetServiceMessage->GetToken() == 0x10)) {
            // Address assignment request, 7 bytes payload:
            // 25: flag 0x20 bit = from non-wsn node, "7f 00 00 01 80 d4" 6 bytes UUID
            SnetServiceMessage l_AddressAssignmentReply(0xAE, 0xAF, 0x20, 0x4000, 0x3FF8, false);
            std::vector<unsigned char> l_Payload = { 0x05 };
            auto l_OldPayload = a_pSnetServiceMessage->GetPayload();
            l_Payload.insert(l_Payload.end(), l_OldPayload.end() - 6, l_OldPayload.end());
            l_Payload.emplace_back(0x00); // status
            l_Payload.emplace_back(0x40);
            l_Payload.emplace_back(0x01);
            l_AddressAssignmentReply.SetPayload(l_Payload);
            
            std::cout << "Send packet " << l_AddressAssignmentReply.Dissect() << std::endl;
            m_ToolHandlerCollection.Send(&l_AddressAssignmentReply);
        } // if
        
        if ((a_pSnetServiceMessage->GetSrcSSA() == 0x4001) &&
            (a_pSnetServiceMessage->GetDstSSA() == 0x4000) &&
            (a_pSnetServiceMessage->GetSrcServiceId() == 0x00)   &&
            (a_pSnetServiceMessage->GetDstServiceId() == 0xB0)   &&
            (a_pSnetServiceMessage->GetToken() == 0x10)) {
            // Subscribe request
            uint8_t l_ServiceId = a_pSnetServiceMessage->GetPayload()[0];
        
            SnetServiceMessage l_ServiceConfirmation(0xB0, 0xB0, 0x11, 0x4000, 0x4001, false);
            std::vector<unsigned char> l_Payload;
            l_Payload.emplace_back(l_ServiceId);
            l_Payload.emplace_back(00);
            l_ServiceConfirmation.SetPayload(l_Payload);
            
            std::cout << "Send packet " << l_ServiceConfirmation.Dissect() << std::endl;
            m_ToolHandlerCollection.Send(&l_ServiceConfirmation);
        } // if
    } else if (l_DstSSA < 0x4000) {
        // To the HDLCd
        a_pSnetServiceMessage->SetOnAirARQ(true);
        m_HdlcdClientHandlerCollection.Send(std::move(HdlcdPacketData::CreatePacket(a_pSnetServiceMessage->Serialize(), true)));
    } else {
        // To the tools
        m_ToolHandlerCollection.Send(a_pSnetServiceMessage);
    } // else
}
