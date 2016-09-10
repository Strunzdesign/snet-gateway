/**
 * \file      AddressService.cpp
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

#include "AddressService.h"
#include "AddressLease.h"

SnetServiceMessage AddressService::ProcessRequest(const SnetServiceMessage& a_ServiceMessage, std::shared_ptr<AddressLease> a_AddressLease) {
    if ((a_ServiceMessage.GetSrcSSA() == 0x3fff) &&
        (a_ServiceMessage.GetDstSSA() == 0x3ffc) &&
        (a_ServiceMessage.GetSrcServiceId() == 0xaf)   &&
        (a_ServiceMessage.GetDstServiceId() == 0xae)   &&
        (a_ServiceMessage.GetToken() == 0x10)) {
        // Address assignment request, 7 bytes payload:
        // 25: flag 0x20 bit = from non-wsn node, "7f 00 00 01 80 d4" 6 bytes UUID
        SnetServiceMessage l_AddressAssignmentReply(0xAE, 0xAF, 0x20, 0x4000, 0x3FF8, false);
        std::vector<unsigned char> l_Payload = { 0x05 };
        auto l_OldPayload = a_ServiceMessage.GetPayload();
        l_Payload.insert(l_Payload.end(), l_OldPayload.end() - 6, l_OldPayload.end());
        l_Payload.emplace_back(0x00); // status
        l_Payload.emplace_back((a_AddressLease->GetAddress() >> 8) & 0x00FF);
        l_Payload.emplace_back((a_AddressLease->GetAddress() >> 0) & 0x00FF);
        l_AddressAssignmentReply.SetPayload(l_Payload);
        return l_AddressAssignmentReply;
    } // if
    
    return SnetServiceMessage();
}
