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

#ifndef TrenchBroom_EntityDefinitionParser_h
#define TrenchBroom_EntityDefinitionParser_h

#include <fstream>
#include "EntityDefinition.h"

using namespace std;

namespace TrenchBroom {
    namespace IO {
        typedef enum {
            TT_FRAC = 1 <<  0, // fractional number
            TT_DEC  = 1 <<  1, // decimal number
            TT_STR  = 1 <<  2, // string
            TT_B_O  = 1 <<  3, // opening brace
            TT_B_C  = 1 <<  4, // closing brace
            TT_CB_O = 1 <<  5, // opening curly brace
            TT_CB_C = 1 <<  6, // closing curly brace
            TT_WORD = 1 <<  7, // word
            TT_QM   = 1 <<  8, // question mark
            TT_ED_O = 1 <<  9, // entity definition open
            TT_ED_C = 1 << 10, // entity definition close
            TT_SC   = 1 << 11, // semicolon
            TT_NL   = 1 << 12, // newline
            TT_C    = 1 << 13  // comma
        } ETokenType;
        
        typedef enum {
            TS_OUTDEF, // currently between definitions
            TS_INDEF, // currently parsing a definition
            TS_COM, // comment
            TS_DEC, // current token is a decimal number
            TS_FRAC, // current token is a fractional number
            TS_WORD, // current token is a word
            TS_Q_STR, // current token is a quoted string
            TS_EOF // parsing is complete
        } ETokenizerState;
        
        class EntityDefinitionToken {
        public:
            ETokenType type;
            string data;
            int line;
            int column;
            int charsRead;
        };
        
        class EntityDefinitionTokenizer {
        private:
            istream& m_stream;
            ETokenizerState m_state;
            int m_line;
            int m_column;
            char m_char;
            EntityDefinitionToken m_token;
            bool nextChar();
            void pushChar();
            char peekChar();
            EntityDefinitionToken* token(ETokenType type, string* data);
        public:
            EntityDefinitionTokenizer(istream& stream);
            EntityDefinitionToken* next();
            EntityDefinitionToken* peek();
            string remainder();
        };
        
        class EntityDefinitionParser {
        private:
            ifstream m_stream;
            EntityDefinitionTokenizer* m_tokenizer;
            void expect(int expectedType, const EntityDefinitionToken* actualToken) const;
            EntityDefinitionToken* nextTokenIgnoringNewlines();
            Vec4f parseColor();
            BBox parseBounds();
            map<string, Model::SpawnFlag> parseFlags();
            vector<Model::Property*> parseProperties();
            Model::Property* parseProperty();
            string parseDescription();
        public:
            EntityDefinitionParser(string path);
            ~EntityDefinitionParser();
            Model::EntityDefinition* nextDefinition();
        };
    }
}

#endif
