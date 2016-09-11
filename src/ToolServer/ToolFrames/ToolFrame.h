/**
 * \file      ToolFrame.h
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

#ifndef TOOL_FRAME_H
#define TOOL_FRAME_H

#include <string>
#include <vector>

class ToolFrame {
public:
    ToolFrame(uint16_t a_RequestId = 0): m_RequestId(a_RequestId) {}
    uint16_t GetRequestId() const { return m_RequestId; }
    virtual const std::vector<unsigned char> SerializeFrame() const {
        std::vector<unsigned char> l_ToolFrameBuffer;
        return l_ToolFrameBuffer;
    }
    
    virtual const std::vector<unsigned char> GetPayload() const = 0;

private:
    // Members
    uint16_t m_RequestId;
};

class ToolFrame0100: public ToolFrame {
public:
    ToolFrame0100(): ToolFrame(0x0100) {}
    const std::vector<unsigned char> GetPayload() const {
        // Dummy
        return std::vector<unsigned char>();
    }
};

class ToolFrame0101: public ToolFrame {
public:
    ToolFrame0101(): ToolFrame(0x0101) {}
    const std::vector<unsigned char> SerializeFrame() const {
        std::vector<unsigned char> l_ToolFrameBuffer;
        l_ToolFrameBuffer.emplace_back(0x7E);
        l_ToolFrameBuffer.emplace_back(0x01);
        l_ToolFrameBuffer.emplace_back(0x01);
        l_ToolFrameBuffer.emplace_back(0x00);
        l_ToolFrameBuffer.emplace_back(0x00);
        l_ToolFrameBuffer.emplace_back(0x7E);
        return l_ToolFrameBuffer;
    }
    
    const std::vector<unsigned char> GetPayload() const {
        // Dummy
        return std::vector<unsigned char>();
    }
};

class ToolFrame0110: public ToolFrame {
public:
    ToolFrame0110(): ToolFrame(0x0110) {}
    const std::vector<unsigned char> GetPayload() const {
        // Dummy
        return std::vector<unsigned char>();
    }
};

class ToolFrame0111: public ToolFrame {
public:
    ToolFrame0111(): ToolFrame(0x0111) {}
    const std::vector<unsigned char> SerializeFrame() const {
        std::vector<unsigned char> l_ToolFrameBuffer;
        l_ToolFrameBuffer.emplace_back(0x7E);
        l_ToolFrameBuffer.emplace_back(0x01);
        l_ToolFrameBuffer.emplace_back(0x11);
        l_ToolFrameBuffer.emplace_back(0x00);
        l_ToolFrameBuffer.emplace_back(0x00);
        l_ToolFrameBuffer.emplace_back(0x7E);
        return l_ToolFrameBuffer;
    }
    
    const std::vector<unsigned char> GetPayload() const {
        // Dummy
        return std::vector<unsigned char>();
    }
};

class ToolFrame0300: public ToolFrame {
public:
    ToolFrame0300(): ToolFrame(0x0300) {}
    const std::vector<unsigned char> GetPayload() const { return m_Payload; }
    
    std::vector<unsigned char> m_Payload;
};

class ToolFrame0301: public ToolFrame {
public:
    ToolFrame0301(): ToolFrame(0x0301) {}
    const std::vector<unsigned char> SerializeFrame() const {
        std::vector<unsigned char> l_ToolFrameBuffer;
        l_ToolFrameBuffer.emplace_back(0x7E);
        l_ToolFrameBuffer.emplace_back(0x03);
        l_ToolFrameBuffer.emplace_back(0x01);
        l_ToolFrameBuffer.emplace_back(0x00);
        l_ToolFrameBuffer.emplace_back(0x00);
        l_ToolFrameBuffer.emplace_back(0x7E);
        return l_ToolFrameBuffer;
    }
    
    const std::vector<unsigned char> GetPayload() const {
        // Dummy
        return std::vector<unsigned char>();
    }
};

class ToolFrame0302: public ToolFrame {
public:
    ToolFrame0302(): ToolFrame(0x0302) {}
    const std::vector<unsigned char> SerializeFrame() const {
        std::vector<unsigned char> l_ToolFrameBuffer;
        l_ToolFrameBuffer.emplace_back(0x7E);
        l_ToolFrameBuffer.emplace_back(0x03);
        l_ToolFrameBuffer.emplace_back(0x02);
        l_ToolFrameBuffer.insert(l_ToolFrameBuffer.end(), m_Payload.begin(), m_Payload.end());
        l_ToolFrameBuffer.emplace_back(0x7E);
        return l_ToolFrameBuffer;
    }
    const std::vector<unsigned char> GetPayload() const { return m_Payload; }
    std::vector<unsigned char> m_Payload;
};

#endif // TOOL_FRAME_H
