/**
 * \file      PublishSubscribeService.cpp
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

#include "PublishSubscribeService.h"
#include "AddressLease.h"

PublishSubscribeService::PublishSubscribeService() {
    m_SubscribedServiceId = 0;
}

SnetServiceMessage PublishSubscribeService::ProcessRequest(const SnetServiceMessage& a_ServiceMessage, std::shared_ptr<AddressLease> a_AddressLease) {
    std::cout << "SUBSCRIBE msg = " << a_ServiceMessage.Dissect() << std::endl;
    if ((a_ServiceMessage.GetSrcSSA() == a_AddressLease->GetAddress()) &&
        (a_ServiceMessage.GetDstSSA() == 0x4000) &&
        (a_ServiceMessage.GetSrcServiceId() == 0x00)   &&
        (a_ServiceMessage.GetDstServiceId() == 0xB0)   &&
        (a_ServiceMessage.GetToken() == 0x10)) {
        // Subscribe request
        m_SubscribedServiceId = a_ServiceMessage.GetPayload()[0];
    
        SnetServiceMessage l_PublishSubscribeConfirmation(0xB0, 0xB0, 0x11, 0x4000, a_AddressLease->GetAddress(), false);
        std::vector<unsigned char> l_Payload;
        l_Payload.emplace_back(m_SubscribedServiceId);
        l_Payload.emplace_back(00);
        l_PublishSubscribeConfirmation.SetPayload(l_Payload);
        return l_PublishSubscribeConfirmation;
    } // if
    
    return SnetServiceMessage();
}

bool PublishSubscribeService::IsServiceIdForMe(uint8_t a_SubscribedServiceId) const {
    if (m_SubscribedServiceId == 0xFF) {
        return true;
    } else {
        return (m_SubscribedServiceId == a_SubscribedServiceId);
    } // else
}
