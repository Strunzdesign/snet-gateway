/**
 * \file      GatewayAccessFrameEscaped.h
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

#ifndef GATEWAY_ACCESS_FRAME_ESCAPED_H
#define GATEWAY_ACCESS_FRAME_ESCAPED_H

#include "GatewayAccessFrame.h"
#include <memory>
#include <assert.h>

class GatewayAccessFrameEscaped: public GatewayAccessFrame {
public:
    static GatewayAccessFrameEscaped Create(const std::vector<unsigned char> &a_Payload) {
        assert(a_Payload.size() < 4096);
        GatewayAccessFrameEscaped l_GatewayAccessFrameEscaped;
        l_GatewayAccessFrameEscaped.m_Buffer = a_Payload;
        return l_GatewayAccessFrameEscaped;
    }

    static std::shared_ptr<GatewayAccessFrameEscaped> CreateDeserializedFrame() {
        auto l_GatewayAccessFrameEscaped(std::shared_ptr<GatewayAccessFrameEscaped>(new GatewayAccessFrameEscaped));
        l_GatewayAccessFrameEscaped->m_eDeserialize = DESERIALIZE_FRAMESTART;
        l_GatewayAccessFrameEscaped->m_BytesRemaining = 1; // Next: consume the start-of-frame delimiter
        return l_GatewayAccessFrameEscaped;
    }

    const std::vector<unsigned char>& GetPayload() const {
        assert(m_eDeserialize == DESERIALIZE_FULL);
        return m_Buffer;
    }

private:
    // Private CTOR
    GatewayAccessFrameEscaped(): m_eDeserialize(DESERIALIZE_FULL) {
    }

    // Methods
    E_GATEWAY_ACCESS_FRAME GetGatewayAccessFrameType() const { return GATEWAY_ACCESS_FRAME_ESCAPED; }

    // Serializer
    const std::vector<unsigned char> Serialize() const {
        assert(m_eDeserialize == DESERIALIZE_FULL);
        // Obtain required amount of memory for the fully escaped frame
        size_t l_NbrOfBytesToEscapeMax = 2; // Space for the two frame delimiters
        for (size_t l_Index = 0; l_Index < m_Buffer.size(); ++l_Index) {
            if ((m_Buffer[l_Index] == 0x7D) || (m_Buffer[l_Index] == 0x7E)) {
                ++l_NbrOfBytesToEscapeMax;
            } // if
        } // for

        std::vector<unsigned char> l_Buffer;
        l_Buffer.reserve(m_Buffer.size() + l_NbrOfBytesToEscapeMax);
        l_Buffer.emplace_back(0x7E);
        for (std::vector<unsigned char>::const_iterator it = m_Buffer.begin(); it < m_Buffer.end(); ++it) {
            if (*it == 0x7D) {
                l_Buffer.emplace_back(0x7D);
                l_Buffer.emplace_back(0x5D);
            } else if (*it == 0x7E) {
                l_Buffer.emplace_back(0x7D);
                l_Buffer.emplace_back(0x5E);
            } else {
                l_Buffer.emplace_back(*it);
            } // else
        } // for

        l_Buffer.emplace_back(0x7E);
        return l_Buffer;
    }

    // Deserializer
    bool Deserialize() {
        // All requested bytes are available
        uint8_t& l_LastReceivedByte = m_Buffer[m_Buffer.size() - 1];
        switch (m_eDeserialize) {
        case DESERIALIZE_FRAMESTART: {
            if (l_LastReceivedByte != GATEWAY_ACCESS_FRAME_ESCAPED) {
                // Error: the start-of-frame delimiter 0x7E was expected!
                m_eDeserialize = DESERIALIZE_ERROR;
                return false;
            } else {
                // The start-of-frame delimiter was found... go ahead!
                m_eDeserialize = DESERIALIZE_UNESCAPED;
                m_BytesRemaining = 1;
                m_Buffer.pop_back();
            } // else
            
            break;
        }
        case DESERIALIZE_UNESCAPED: {
            if (l_LastReceivedByte == 0x7E) {
                // This is the end-of-frame delimiter. The frame is complete now.
                m_eDeserialize = DESERIALIZE_FULL;
                m_Buffer.pop_back();
            } else if (l_LastReceivedByte == 0x7D) {
                // This is the escape character. The next byte in the buffer indicates the escaped character.
                m_eDeserialize = DESERIALIZE_ESCAPED;
                m_BytesRemaining = 1;
                m_Buffer.pop_back();
            } else {
                // This is a non-escaped character. Keep it.
                if (m_Buffer.size() < 4096) {
                    m_BytesRemaining = 1;
                } else {
                    // Error: the payload exceeded the MTU!
                    m_eDeserialize = DESERIALIZE_ERROR;
                    return false;
                } // else
            } // else
            
            break;
        }
        case DESERIALIZE_ESCAPED: {
            // The previous character was the escape character
            if (l_LastReceivedByte == 0x5E) {
                l_LastReceivedByte = 0x7E;
                m_BytesRemaining = 1;
            } else if (l_LastReceivedByte == 0x5D) {
                l_LastReceivedByte = 0x7D;
                m_BytesRemaining = 1;
            } else {
                // Invalid character
                m_eDeserialize = DESERIALIZE_ERROR;
                return false;
            } // else
            
            if (m_Buffer.size() >= 4096) {
                // Error: the payload exceeded the MTU!
                m_eDeserialize = DESERIALIZE_ERROR;
                return false;
            } // else

            // The next byte will be an unescaped character again
            m_eDeserialize = DESERIALIZE_UNESCAPED;
            break;
        }
        case DESERIALIZE_ERROR:
        case DESERIALIZE_FULL:
        default:
            assert(false);
        } // switch

        // No error
        return true;
    }

    // Members
    typedef enum {
        DESERIALIZE_ERROR      = 0,
        DESERIALIZE_FRAMESTART = 1,
        DESERIALIZE_UNESCAPED  = 2,
        DESERIALIZE_ESCAPED    = 3,
        DESERIALIZE_FULL       = 4
    } E_DESERIALIZE;
    E_DESERIALIZE m_eDeserialize;
};

#endif // GATEWAY_ACCESS_FRAME_ESCAPED_H
