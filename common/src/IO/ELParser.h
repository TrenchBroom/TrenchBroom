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

#ifndef ELParser_h
#define ELParser_h

#include "IO/Parser.h"
#include "IO/Token.h"
#include "IO/Tokenizer.h"

namespace TrenchBroom {
    namespace IO {
        namespace ELToken {
            typedef size_t Type;
            static const Type Variable    = 1 <<  1;
            static const Type String = 1 << 2;
            static const Type Integer = 1 << 3;
            static const Type Decimal = 1 << 4;
            static const Type OBracket = 1 << 5;
            static const Type CBracket = 1 << 6;
            static const Type OBrace = 1 << 7;
            static const Type CBrace = 1 << 8;
            static const Type OParen = 1 << 9;
            static const Type CParen = 1 << 10;
            static const Type Plus = 1 << 11;
            static const Type Minus = 1 << 12;
            static const Type Times = 1 << 13;
            static const Type Over = 1 << 14;
            static const Type Colon = 1 << 15;
            static const Type Eof = 1 << 16;
        }
        
        class ELTokenizer : public Tokenizer<ELToken::Type> {
        public:
            ELTokenizer(const char* begin, const char* end);
            ELTokenizer(const String& str);
        private:
            Token emitToken();
        };
        
    }
}

#endif /* ELParser_h */
