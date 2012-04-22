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
#include "Model/Map/Map.h"
#include "Model/Map/Entity.h"
#include "Model/Map/Brush.h"
#include "Model/Map/Face.h"
#include "Model/Assets/Texture.h"

using namespace std;
using namespace TrenchBroom::Model;

namespace TrenchBroom {
    namespace Controller {
        class ProgressIndicator;
    }

    namespace IO {
        typedef enum {
            TB_TT_FRAC = 1 << 0, // fractional number
            TB_TT_DEC  = 1 << 1, // decimal number
            TB_TT_STR  = 1 << 2, // string
            TB_TT_B_O  = 1 << 3, // opening parenthesis
            TB_TT_B_C  = 1 << 4, // closing parenthesis
            TB_TT_CB_O = 1 << 5, // opening curly bracket
            TB_TT_CB_C = 1 << 6, // closing curly bracket
            TB_TT_SB_O = 1 << 7, // opening square bracket
            TB_TT_SB_C = 1 << 8, // closing square bracket
            TB_TT_COM  = 1 << 9 // comment
        } ETokenType;
        
        typedef enum {
            TB_TS_DEF, // default state
            TB_TS_DEC, // current token is a decimal number
            TB_TS_FRAC, // current token is a fractional number
            TB_TS_STR, // current token is a string
            TB_TS_Q_STR, // current token is a quoted string
            TB_TS_COM,
            TB_TS_EOF // parsing is complete
        } ETokenizerState;
        
        typedef enum {
            TB_PS_DEF, // default state
            TB_PS_ENT, // currently parsing an entity
            TB_PS_BRUSH, // currently parsing a brush
        } EParserState;
        
        typedef enum {
            TB_MF_STANDARD,
            TB_MF_VALVE,
            TB_MF_UNDEFINED
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
            int size();
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
            void parseMap(Map& map, Controller::ProgressIndicator* indicator);
            Entity* parseEntity(Controller::ProgressIndicator* indicator);
            Brush* parseBrush(Controller::ProgressIndicator* indicator);
            Face* parseFace();
        };
    }
}
#endif
