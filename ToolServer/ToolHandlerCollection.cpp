/**
 * \file ToolHandlerCollection.cpp
 * \brief 
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

#include "ToolHandlerCollection.h"
#include "ToolHandler.h"
#include "ToolFrame.h"
#include "SnetServiceMessage.h"
#include <assert.h>

ToolHandlerCollection::ToolHandlerCollection() {
    m_pRoutingEntity = NULL;
}

void ToolHandlerCollection::RegisterToolHandler(std::shared_ptr<ToolHandler> a_ToolHandler) {
    assert(m_pRoutingEntity);
    a_ToolHandler->RegisterRoutingEntity(m_pRoutingEntity);
    m_ToolHandlerList.emplace_back(std::move(a_ToolHandler));
}

void ToolHandlerCollection::DeregisterToolHandler(std::shared_ptr<ToolHandler> a_ToolHandler) {
    m_ToolHandlerList.remove(a_ToolHandler);
}

void ToolHandlerCollection::RegisterRoutingEntity(Routing* a_pRoutingEntity) {
    assert(a_pRoutingEntity);
    m_pRoutingEntity = a_pRoutingEntity;
}

void ToolHandlerCollection::Send(SnetServiceMessage* a_pSnetServiceMessage) {
    for (auto l_It = m_ToolHandlerList.begin(); l_It != m_ToolHandlerList.end(); ++l_It) {
        ToolFrame0302 l_ToolFrame0302;
        l_ToolFrame0302.m_Payload = a_pSnetServiceMessage->Serialize();
        (*l_It)->Send(&l_ToolFrame0302);
    } // for
}
