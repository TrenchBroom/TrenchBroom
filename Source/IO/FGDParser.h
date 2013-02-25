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

#include "IO/Tokenizer.h"
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
        
        class FGDParser {
        protected:
//            Tokenizer m_tokenizer;
            
            Vec3f parseVec();
            BBox parseBounds();
        public:
            FGDParser(std::istream& stream) {}
            ~FGDParser();
            
            Model::EntityDefinition* nextDefinition();
        };
    }
}

#endif /* defined(__TrenchBroom__FGDParser__) */
