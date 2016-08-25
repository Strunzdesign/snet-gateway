/**
 * \file ToolFrameGenerator.cpp
 * \brief 
 *
 * Copyright (c) 2016, Florian Evers, florian-evers@gmx.de
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     (1) Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 
 *     (2) Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.  
 *     
 *     (3)The name of the author may not be used to
 *     endorse or promote products derived from this software without
 *     specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "ToolFrameGenerator.h"
#include <assert.h>
#include <iostream>

std::vector<unsigned char> ToolFrameGenerator::EscapeFrame(const std::vector<unsigned char> &a_ToolFrame) {
    // Obtain required amount of memory for the fully escaped HDLC frame
    size_t l_NbrOfBytesToEscapeMax = 0;
    for (size_t l_Index = 1; l_Index < (a_ToolFrame.size() - 1); ++l_Index) {
        if ((a_ToolFrame[l_Index] == 0x7D) || (a_ToolFrame[l_Index] == 0x7E)) {
            ++l_NbrOfBytesToEscapeMax;
        } // if
    } // for

    // Prepare return buffer
    std::vector<unsigned char> l_EscapedToolFrame;
    l_EscapedToolFrame.reserve(a_ToolFrame.size() + l_NbrOfBytesToEscapeMax);
    l_EscapedToolFrame.emplace_back(0x7E);
    for (std::vector<unsigned char>::const_iterator it = (a_ToolFrame.begin() + 1); it < (a_ToolFrame.end() - 1); ++it) {
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
    return std::move(l_EscapedToolFrame);
}

