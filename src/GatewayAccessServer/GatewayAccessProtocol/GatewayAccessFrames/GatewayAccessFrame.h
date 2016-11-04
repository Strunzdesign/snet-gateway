/**
 * \file      GatewayAccessFrame.h
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

#ifndef GATEWAY_ACCESS_FRAME_H
#define GATEWAY_ACCESS_FRAME_H

#include "Frame.h"

typedef enum {
    GATEWAY_ACCESS_FRAME_UNKNOWN  = 0x00,
    GATEWAY_ACCESS_FRAME_ESCAPED  = 0x7E,
    GATEWAY_ACCESS_FRAME_LENGTH   = 0x80,

    // Book keeping
    GATEWAY_ACCESS_FRAME_MASK     = 0x80
} E_GATEWAY_ACCESS_FRAME;

class GatewayAccessFrame: public Frame {
public:
    virtual E_GATEWAY_ACCESS_FRAME GetGatewayAccessFrameType() const { return GATEWAY_ACCESS_FRAME_UNKNOWN; }
};

#endif // GATEWAY_ACCESS_FRAME_H
