/*
 Copyright (C) 2010-2012 Kristian Duske
 
 This file is part of TrenchBroom.
 
 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_MapParser_h
#define TrenchBroom_MapParser_h

#include <string>
#include <istream>
#include <sstream>
#include "Map.h"
#include "Entity.h"
#include "Brush.h"
#include "Face.h"
#include "Texture.h"

using namespace std;
using namespace TrenchBroom::Model;

namespace TrenchBroom {
    namespace IO {
        typedef enum {
            TT_FRAC = 1 << 0, // fractional number
            TT_DEC  = 1 << 1, // decimal number
            TT_STR  = 1 << 2, // string
            TT_B_O  = 1 << 3, // opening parenthesis
            TT_B_C  = 1 << 4, // closing parenthesis
            TT_CB_O = 1 << 5, // opening curly bracket
            TT_CB_C = 1 << 6, // closing curly bracket
            TT_SB_O = 1 << 7, // opening square bracket
            TT_SB_C = 1 << 8, // closing square bracket
            TT_COM  = 1 << 9 // comment
        } ETokenType;
        
        typedef enum {
            TS_DEF, // default state
            TS_DEC, // current token is a decimal number
            TS_FRAC, // current token is a fractional number
            TS_STR, // current token is a string
            TS_Q_STR, // current token is a quoted string
            TS_COM,
            TS_EOF // parsing is complete
        } ETokenizerState;
        
        typedef enum {
            PS_DEF, // default state
            PS_ENT, // currently parsing an entity
            PS_BRUSH, // currently parsing a brush
        } EParserState;
        
        typedef enum {
            MF_STANDARD,
            MF_VALVE,
            MF_UNDEFINED
        } EMapFormat;
        
        class MapToken {
        public:
            ETokenType type;
            string data;
            int line;
            int column;
            int charsRead;
        };
        
        class MapTokenizer {
            vector<char> m_chars;
            int m_index;
            char m_buffer[1024];
            int m_bufferIndex;
            ETokenizerState m_state;
            int m_line;
            int m_column;
            int m_startLine;
            int m_startColumn;
            MapToken m_token;
            char nextChar();
            char peekChar();
            MapToken* token(ETokenType type, char* data, int index, int line, int column);
        public:
            MapTokenizer(istream& stream);
            MapToken* next();
        };
        
        class MapParser {
        private:
            const BBox& m_worldBounds;
            Assets::TextureManager& m_textureManager;
            int m_size;
            EMapFormat m_format;
            MapTokenizer* m_tokenizer;
            vector<MapToken*> m_tokenStack;
            
            void expect(int expectedType, const MapToken* actualToken) const;
            MapToken* nextToken();
            void pushToken(MapToken* token);
        public:
            MapParser(istream& stream, const BBox& worldBounds, Assets::TextureManager& textureManager);
            ~MapParser();
            Map* parseMap(const string& entityDefinitionFilePath);
            Entity* parseEntity();
            Brush* parseBrush();
            Face* parseFace();
        };
    }
}
#endif
