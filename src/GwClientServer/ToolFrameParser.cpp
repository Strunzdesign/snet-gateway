/**
 * \file      ToolFrameParser.cpp
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

#include "ToolFrameParser.h"
#include "GwClientHandler.h"
#include "CommandResponseFrame.h"
#include "CommandResponseFrame0100.h"
#include "CommandResponseFrame0110.h"
#include "CommandResponseFrame0300.h"

ToolFrameParser::ToolFrameParser(GwClientHandler& a_GwClientHandler): m_GwClientHandler(a_GwClientHandler) {
    Reset();
}

void ToolFrameParser::Reset() {
    // Prepare assembly buffer
    m_Buffer.clear();
    m_Buffer.reserve(max_length);
    m_Buffer.emplace_back(0x7E);
    m_bStartTokenSeen = false;
}

void ToolFrameParser::AddReceivedRawBytes(const unsigned char* a_Buffer, size_t a_Bytes) {
    while (a_Bytes) {
        size_t l_ConsumedBytes = AddChunk(a_Buffer, a_Bytes);
        a_Buffer += l_ConsumedBytes;
        a_Bytes  -= l_ConsumedBytes;
    } // while
}

size_t ToolFrameParser::AddChunk(const unsigned char* a_Buffer, size_t a_Bytes) {
    if (m_bStartTokenSeen == false) {
        // No start token seen yet. Check if there is the start token available in the input buffer.
        const void* l_pStartTokenPtr = memchr((const void*)a_Buffer, 0x7E, a_Bytes);
        if (l_pStartTokenPtr) {
            // The start token was found in the input buffer. 
            m_bStartTokenSeen = true;
            if (l_pStartTokenPtr == a_Buffer) {
                // The start token is at the beginning of the buffer. Clip it.
                return 1;
            } else {
                // Clip front of buffer containing junk, including the start token.
                return ((const unsigned char*)l_pStartTokenPtr - a_Buffer + 1);
            } // else
        } else {
            // No token found, and no token was seen yet. Dropping received buffer now.
            return a_Bytes;
        } // else
    } else {
        // We already have seen the start token. Check if there is the end token available in the input buffer.
        const void* l_pEndTokenPtr = memchr((const void*)a_Buffer, 0x7E, a_Bytes);
        if (l_pEndTokenPtr) {
            // The end token was found in the input buffer. At first, check if we receive to much data.
            size_t l_NbrOfBytes = ((const unsigned char*)l_pEndTokenPtr - a_Buffer + 1);
            if ((m_Buffer.size() + l_NbrOfBytes) <= (2 * max_length)) {
                // We did not exceed the maximum frame size yet. Copy all bytes including the end token.
                m_Buffer.insert(m_Buffer.end(), a_Buffer, a_Buffer + l_NbrOfBytes);
                if (RemoveEscapeCharacters()) {
                    // The complete frame was valid and was consumed.
                    m_bStartTokenSeen = false;
                } // if
            } // else

            m_Buffer.resize(1); // Already contains start token 0x7E
            return (l_NbrOfBytes);
        } else {
            // No end token found. Copy all bytes if we do not exceed the maximum frame size.
            if ((m_Buffer.size() + a_Bytes) > (2 * max_length)) {
                // Even if all these bytes were escaped, we have exceeded the maximum frame size.
                m_bStartTokenSeen = false;
                m_Buffer.resize(1); // Already contains start token 0x7E
            } else {
                // Add all bytes
                m_Buffer.insert(m_Buffer.end(), a_Buffer, a_Buffer + a_Bytes);
            } // else
            
            return a_Bytes;
        } // else
    } // else
}

bool ToolFrameParser::RemoveEscapeCharacters() {
    // Checks
    assert(m_Buffer.front() == 0x7E);
    assert(m_Buffer.back()  == 0x7E);
    assert(m_Buffer.size() >= 2);
    assert(m_bStartTokenSeen == true);

    if (m_Buffer.size() == 2) {
        // Remove junk, start again
        return false;
    } // if
    
    // Check for illegal escape sequence at the end of the buffer
    bool l_bMessageInvalid = false;
    if (m_Buffer[m_Buffer.size() - 2] == 0x7D) {
        l_bMessageInvalid = true;
    } else {
        // Remove escape sequences
        std::vector<unsigned char> l_UnescapedBuffer;
        l_UnescapedBuffer.reserve(m_Buffer.size());
        for (auto it = m_Buffer.begin(); it != m_Buffer.end(); ++it) {
            if (*it == 0x7D) {
                // This was the escape character
                ++it;
                if (*it == 0x5E) {
                    l_UnescapedBuffer.emplace_back(0x7E);
                } else if (*it == 0x5D) {
                    l_UnescapedBuffer.emplace_back(0x7D);
                } else {
                    // Invalid character. Go ahead with an invalid frame.
                    l_bMessageInvalid = true;
                    l_UnescapedBuffer.emplace_back(*it);
                } // else
            } else {
                // Normal non-escaped character, or one of the frame delimiters
                l_UnescapedBuffer.emplace_back(*it);
            } // else
        } // while
        
        // Go ahead with the unescaped buffer
        m_Buffer = std::move(l_UnescapedBuffer);
    } // if
    
    // We now have the unescaped frame at hand.
    if ((m_Buffer.size() < 2) || (m_Buffer.size() > max_length)) {
        // To short or too long for a valid HDLC frame. We consider it as junk.
        return false;
    } // if

    if (l_bMessageInvalid == false) {
        m_GwClientHandler.InterpretDeserializedToolFrame(DeserializeToolFrame(m_Buffer));
    } // if

    return (l_bMessageInvalid == false);
}

std::shared_ptr<CommandResponseFrame> ToolFrameParser::DeserializeToolFrame(const std::vector<unsigned char> &a_UnescapedBuffer) const {
    // Parse byte buffer to get the tool frame
    std::shared_ptr<CommandResponseFrame> l_CommandResponseFrame;
    uint16_t l_RequestId = ((uint16_t(a_UnescapedBuffer[1]) << 8) + a_UnescapedBuffer[2]);
    if (l_RequestId == 0x0100) {
        l_CommandResponseFrame = std::make_shared<CommandResponseFrame0100>();
    } else if (l_RequestId == 0x0110) {
        l_CommandResponseFrame = std::make_shared<CommandResponseFrame0110>();
    } else if (l_RequestId == 0x0300) {
        auto l_CommandResponseFrame0300 = std::make_shared<CommandResponseFrame0300>();
        l_CommandResponseFrame0300->m_Payload.clear();
        l_CommandResponseFrame0300->m_Payload.insert(l_CommandResponseFrame0300->m_Payload.end(), a_UnescapedBuffer.begin() + 3, a_UnescapedBuffer.end() - 1);
        l_CommandResponseFrame = l_CommandResponseFrame0300;
    } // else if
    
    return l_CommandResponseFrame;
}
