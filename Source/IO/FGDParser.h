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

#ifndef __TrenchBroom__FGDParser__
#define __TrenchBroom__FGDParser__

#include "IO/AbstractTokenizer.h"
#include "Utility/VecMath.h"

#include <iostream>
#include <memory>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class EntityDefinition;
    }
    
    namespace IO {
        namespace TokenType {
            static const unsigned int Integer           = 1 <<  0; // integer number
            static const unsigned int Decimal           = 1 <<  1; // decimal number
            static const unsigned int Word              = 1 <<  2; // letter or digits, no whitespace
            static const unsigned int String            = 1 <<  3; // "anything but quotes"
            static const unsigned int OParenthesis      = 1 <<  4; // opening parenthesis: (
            static const unsigned int CParenthesis      = 1 <<  5; // closing parenthesis: )
            static const unsigned int OBracket          = 1 <<  6; // opening bracket: [
            static const unsigned int CBracket          = 1 <<  7; // closing bracket: ]
            static const unsigned int Equality          = 1 <<  8; // equality sign: =
            static const unsigned int Colon             = 1 <<  9; // colon: :
        }
        
        namespace TokenizerState {
            static const unsigned int Outside    = 0; // currently outside of a definition
            static const unsigned int Inside     = 1; // currently parsing a definition
            static const unsigned int Comment    = 2; // currently reading a comment
            static const unsigned int Integer    = 3; // currently reading an integer number
            static const unsigned int Decimal    = 4; // currently reading a decimal number
            static const unsigned int Word       = 5; // currently reading a single word
            static const unsigned int String     = 6; // currently reading a quoted string
            static const unsigned int Eof        = 7; // reached end of file
        }
        
        class FGDTokenizer : public AbstractTokenizer {
        public:
            class Token : public AbstractToken<unsigned int, Token> {
            public:
                Token(unsigned int type, const String& data, size_t position, size_t line, size_t column) : AbstractToken(type, data, position, line, column) {}
            };
            
            typedef std::auto_ptr<Token> TokenPtr;
        protected:
            StringStream m_buffer;
            unsigned int m_state;
            
            inline TokenPtr token(unsigned int type, const String& data) {
                return TokenPtr(new Token(type, data, m_position, m_line, m_column));
            }
        public:
            FGDTokenizer(std::istream& stream) :
            AbstractTokenizer(stream),
            m_state(TokenizerState::Outside) {}
            
            TokenPtr nextToken();
            TokenPtr peekToken();
        };
        
        class FGDParser {
        protected:
            FGDTokenizer m_tokenizer;
            
            Vec3f parseVec();
            BBox parseBounds();
        public:
            FGDParser(std::istream& stream) : m_tokenizer(stream) {}
            ~FGDParser();
            
            Model::EntityDefinition* nextDefinition();
        };
    }
}

#endif /* defined(__TrenchBroom__FGDParser__) */
