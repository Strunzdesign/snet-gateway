/**
 * \file ToolFrame.h
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
    const std::vector<unsigned char> GetPayload() const {}
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
    
    const std::vector<unsigned char> GetPayload() const {}
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
    const std::vector<unsigned char> GetPayload() const {}
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
