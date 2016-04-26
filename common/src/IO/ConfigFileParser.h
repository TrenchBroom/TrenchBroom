/*
 Copyright (C) 2010-2016 Kristian Duske
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_ConfigFileParser
#define TrenchBroom_ConfigFileParser

#include "ConfigTypes.h"
#include "IO/Parser.h"
#include "IO/Token.h"
#include "IO/Tokenizer.h"

namespace TrenchBroom {
    namespace IO {
        namespace ConfigFileToken {
            typedef size_t Type;
            static const Type Identifier    = 1 <<  1; // string
            static const Type String        = 1 <<  2; // "string"
            static const Type OBrace        = 1 <<  3; // opening brace: {
            static const Type CBrace        = 1 <<  4; // closing brace: }
            static const Type Comma         = 1 <<  5; // comma: ,
            static const Type Equals        = 1 <<  6; // equals: =
            static const Type Comment       = 1 <<  7; // line comment starting with //
            static const Type Eof           = 1 <<  8; // end of file
        }

        class ConfigFileTokenizer : public Tokenizer<ConfigFileToken::Type> {
        public:
            ConfigFileTokenizer(const char* begin, const char* end);
            ConfigFileTokenizer(const String& str);
        private:
            Token emitToken();
        };

        class ConfigFileParser :  public Parser<ConfigFileToken::Type> {
        private:
            ConfigFileTokenizer m_tokenizer;
            typedef ConfigFileTokenizer::Token Token;
        public:
            ConfigFileParser(const char* begin, const char* end);
            ConfigFileParser(const String& str);
            
            ConfigEntry* parse();
        private:
            ConfigEntry* parseEntry();
            ConfigEntry::Type detectEntryType();
            ConfigEntry* parseValue();
            ConfigEntry* parseList();
            ConfigEntry* parseTable();
            TokenNameMap tokenNames() const;
        };
    }
}

#endif /* defined(TrenchBroom_ConfigFileParser) */
