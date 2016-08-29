/**
 * \file      ToolHandlerCollection.h
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

#ifndef TOOL_HANDLER_COLLECTION_H
#define TOOL_HANDLER_COLLECTION_H

#include <memory>
#include <list>
#include "AddressPool.h"
class ToolHandler;
class Routing;
class SnetServiceMessage;
class AddressLease;

class ToolHandlerCollection {
public:
    ToolHandlerCollection();
    std::shared_ptr<AddressLease> RegisterToolHandler(std::shared_ptr<ToolHandler> a_ToolHandler);
    void DeregisterToolHandler(std::shared_ptr<ToolHandler> a_ToolHandler);

    void RegisterRoutingEntity(Routing* a_pRoutingEntity);
    void Send(SnetServiceMessage* a_pSnetServiceMessage);
    
private:
    // Members
    std::shared_ptr<AddressPool> m_AddressPool;
    std::list<std::shared_ptr<ToolHandler>> m_ToolHandlerList;
    Routing* m_pRoutingEntity;
};

#endif // TOOL_HANDLER_COLLECTION_H
