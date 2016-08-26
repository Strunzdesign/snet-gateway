/**
 * \file      AddressPool.cpp
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

#include "AddressPool.h"
#include "AddressLease.h"

AddressPool::AddressPool() {
    m_NextFreeAddress = 0x4001;
}

std::shared_ptr<AddressLease> AddressPool::ObtainAddressLease() {
    uint16_t l_NextAddress;
    if (m_ReleasedAddresses.empty()) {
        l_NextAddress = (m_NextFreeAddress++);
    } else {
        l_NextAddress = m_ReleasedAddresses.front();
        m_ReleasedAddresses.pop_front();
    } // else
        
    auto l_AddressLease = std::make_shared<AddressLease>(shared_from_this(), l_NextAddress);
    return l_AddressLease;
}

void AddressPool::ReleaseAddressLease(uint16_t a_ToolAddress) {
    m_ReleasedAddresses.push_back(a_ToolAddress);
}
