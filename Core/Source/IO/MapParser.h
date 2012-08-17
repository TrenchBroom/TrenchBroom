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

#include "Model/Map/EntityTypes.h"
#include "Model/Map/BrushTypes.h"
#include "Model/Map/FaceTypes.h"
#include "Utilities/Console.h"
#include "Utilities/MessageException.h"
#include "Utilities/VecMath.h"

#include <istream>
#include <sstream>
#include <string>

namespace TrenchBroom {
    namespace Controller {
        class ProgressIndicator;
    }

    namespace Model {
        class Map;
        class Entity;
        class Brush;
        class Face;
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
            std::string data;
            unsigned int line;
            unsigned int column;
            unsigned int charsRead;
        };
        
        class MapTokenizer {
            std::vector<char> m_chars;
            unsigned int m_index;
            char m_buffer[1024];
            unsigned int m_bufferIndex;
            ETokenizerState m_state;
            unsigned int m_line;
            unsigned int m_column;
            unsigned int m_startLine;
            unsigned int m_startColumn;
            MapToken m_token;
            char nextChar();
            char peekChar();
            MapToken* token(ETokenType type, char* data, unsigned int index, unsigned int line, unsigned int column);
        public:
            MapTokenizer(std::istream& stream);
            MapToken* next();
            int size();
            void reset();
        };
        
        class MapParserException : public MessageException {
        private:
            std::string type(int type) {
                std::vector<std::string> names;
                if ((type & TB_TT_FRAC) != 0)
                    names.push_back("fractional number");
                if ((type & TB_TT_DEC) != 0)
                    names.push_back("decimal number");
                if ((type & TB_TT_STR) != 0)
                    names.push_back("string");
                if ((type & TB_TT_B_O) != 0)
                    names.push_back("opening parenthesis");
                if ((type & TB_TT_B_C) != 0)
                    names.push_back("closing parenthesis");
                if ((type & TB_TT_CB_O) != 0)
                    names.push_back("opening brace");
                if ((type & TB_TT_CB_C) != 0)
                    names.push_back("closing brace");
                if ((type & TB_TT_SB_O) != 0)
                    names.push_back("opening bracket");
                if ((type & TB_TT_SB_C) != 0)
                    names.push_back("closing bracket");
                if ((type & TB_TT_COM) != 0)
                    names.push_back("comment");
                
                if (names.empty())
                    return "unknown token type";
                if (names.size() == 1)
                    return names[0];
                
                std::stringstream str;
                str << names[0];
                for (unsigned int i = 1; i < names.size() - 1; i++)
                    str << ", " << names[i];
                str << ", or " << names[names.size() - 1];
                return str.str();
            }
            
            std::string buildMessage(const MapToken& token, int expectedType) {
                std::stringstream msgStream;
                msgStream << "Malformed map file: expected token of type " << type(expectedType) << ", but found " << type(token.type) << " at line " << token.line << ", column " << token.column;
                return msgStream.str();
            }
        public:
            MapParserException() : MessageException("Reached unexpected end of file") {}
            MapParserException(const MapToken& token, int expectedType) : MessageException(buildMessage(token, expectedType)) {}
        };
        
        class MapParser {
        private:
            size_t m_size;
            EMapFormat m_format;
            MapTokenizer* m_tokenizer;
            std::vector<MapToken*> m_tokenStack;
            
            inline void expect(int expectedType, const MapToken* actualToken) const {
                if (actualToken == NULL)
                    throw MapParserException();
                
                if ((actualToken->type & expectedType) == 0)
                    throw MapParserException(*actualToken, expectedType);
            }

            MapToken* nextToken();
            void pushToken(MapToken* token);
        public:
            MapParser(std::istream& stream);
            ~MapParser();
            void parseMap(Model::Map& map, Controller::ProgressIndicator* indicator);
            Model::Entity* parseEntity(const BBox& worldBounds, Controller::ProgressIndicator* indicator);
            Model::Brush* parseBrush(const BBox& worldBounds, Controller::ProgressIndicator* indicator);
            Model::Face* parseFace(const BBox& worldBounds);
            
            bool parseEntities(const BBox& worldBounds, Model::EntityList& entities);
            bool parseBrushes(const BBox& worldBounds, Model::BrushList& brushes);
            bool parseFaces(const BBox& worldBounds, Model::FaceList& faces);
        };
    }
}
#endif
