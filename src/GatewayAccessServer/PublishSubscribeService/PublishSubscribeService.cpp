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

// Application-layer tokens
typedef enum {
    PS_TOKEN_SUBSCRIBE_REQUEST    = 0x10,
    PS_TOKEN_SUBSCRIBE_REPLY      = 0x11,
    PS_TOKEN_UNSUBSCRIBE_REQUEST  = 0x20,
    PS_TOKEN_UNSUBSCRIBE_REPLY    = 0x21
} E_PS_TOKEN;

SnetServiceMessage PublishSubscribeService::ProcessRequest(const SnetServiceMessage& a_ServiceMessage, std::shared_ptr<AddressLease> a_AddressLease) {
    if ((a_ServiceMessage.GetDstSSA() == 0x4000)     &&
        (a_ServiceMessage.GetDstServiceId() == 0xB0) &&
        (a_ServiceMessage.GetPayload().size() == 1)) {
        // What kind of a request message is that?
        uint8_t l_ServiceIdentifier = a_ServiceMessage.GetPayload()[0];
        if (a_ServiceMessage.GetToken() == PS_TOKEN_SUBSCRIBE_REQUEST) {
            // A subscribe service request message
            SnetServiceMessage l_PublishSubscribeReply(0xB0, a_ServiceMessage.GetSrcServiceId(), PS_TOKEN_SUBSCRIBE_REPLY, 0x4000, a_AddressLease->GetAddress(), false);
            std::vector<unsigned char> l_Payload;
            l_Payload.emplace_back(l_ServiceIdentifier);
            l_Payload.emplace_back(00); // status byte
            l_PublishSubscribeReply.SetPayload(l_Payload);
            
            // Add a subscription
            if (l_ServiceIdentifier == 0xFF) {
                // Wildcard: all service IDs are demanded for subscription
                m_SubscribedServiceIds.set();
            } else {
                // A specific service ID was demanded for, add it to the bitset
                m_SubscribedServiceIds.set(l_ServiceIdentifier);
            } // else

            return l_PublishSubscribeReply;
        } else if (a_ServiceMessage.GetToken() == PS_TOKEN_UNSUBSCRIBE_REQUEST) {
            // A subscribe service request message
            SnetServiceMessage l_PublishSubscribeReply(0xB0, a_ServiceMessage.GetSrcServiceId(), PS_TOKEN_UNSUBSCRIBE_REPLY, 0x4000, a_AddressLease->GetAddress(), false);
            std::vector<unsigned char> l_Payload;
            l_Payload.emplace_back(l_ServiceIdentifier);
            l_Payload.emplace_back(00); // status byte
            l_PublishSubscribeReply.SetPayload(l_Payload);

            // Remove a subscription
            if (l_ServiceIdentifier == 0xFF) {
                // Wildcard: all service IDs are demanded for removal
                m_SubscribedServiceIds.reset();
            } else {
                // A specific service ID was demanded for removal, remove it from the bitset
                m_SubscribedServiceIds.reset(l_ServiceIdentifier);
            } // else

            return l_PublishSubscribeReply;
        } // else if
    } // if            
    
    // Not interpreted... return an empty reply message
    return SnetServiceMessage();
}

bool PublishSubscribeService::IsServiceIdForMe(uint8_t a_ServiceId) const {
    return (m_SubscribedServiceIds[a_ServiceId]);
}
