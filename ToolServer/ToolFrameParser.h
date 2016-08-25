/**
 * \file      ToolFrameParser.h
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

#ifndef TOOL_FRAME_PARSER_H
#define TOOL_FRAME_PARSER_H

#include <memory>
#include <vector>
#include "ToolFrame.h"
class ToolHandler;

class ToolFrameParser {
public:
    ToolFrameParser(ToolHandler& a_ToolHandler);
    void Reset();
    void AddReceivedRawBytes(const unsigned char* a_Buffer, size_t a_Bytes);
    
private:
    // Interal helpers
    size_t AddChunk(const unsigned char* a_Buffer, size_t a_Bytes);
    bool RemoveEscapeCharacters();
    std::shared_ptr<ToolFrame> DeserializeToolFrame(const std::vector<unsigned char> &a_UnescapedBuffer) const;
    
    // Members
    ToolHandler& m_ToolHandler;

    enum { max_length = 1024 };
    std::vector<unsigned char> m_Buffer;
    bool m_bStartTokenSeen;
};

#endif // TOOL_FRAME_PARSER_H

