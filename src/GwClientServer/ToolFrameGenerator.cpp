/**
 * \file      ToolFrameGenerator.cpp
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

#include "ToolFrameGenerator.h"
#include <assert.h>
#include <iostream>

std::vector<unsigned char> ToolFrameGenerator::EscapeFrame(const std::vector<unsigned char> &a_CommandResponseFrame) {
    // Obtain required amount of memory for the fully escaped HDLC frame
    size_t l_NbrOfBytesToEscapeMax = 0;
    for (size_t l_Index = 1; l_Index < (a_CommandResponseFrame.size() - 1); ++l_Index) {
        if ((a_CommandResponseFrame[l_Index] == 0x7D) || (a_CommandResponseFrame[l_Index] == 0x7E)) {
            ++l_NbrOfBytesToEscapeMax;
        } // if
    } // for

    // Prepare return buffer
    std::vector<unsigned char> l_EscapedToolFrame;
    l_EscapedToolFrame.reserve(a_CommandResponseFrame.size() + l_NbrOfBytesToEscapeMax);
    l_EscapedToolFrame.emplace_back(0x7E);
    for (std::vector<unsigned char>::const_iterator it = (a_CommandResponseFrame.begin() + 1); it < (a_CommandResponseFrame.end() - 1); ++it) {
        if (*it == 0x7D) {
            l_EscapedToolFrame.emplace_back(0x7D);
            l_EscapedToolFrame.emplace_back(0x5D);
        } else if (*it == 0x7E) {
            l_EscapedToolFrame.emplace_back(0x7D);
            l_EscapedToolFrame.emplace_back(0x5E);
        } else {
            l_EscapedToolFrame.emplace_back(*it);
        } // else
    } // for
    
    l_EscapedToolFrame.emplace_back(0x7E);
    return l_EscapedToolFrame;
}

