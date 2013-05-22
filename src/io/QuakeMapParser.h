/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef __TrenchBroom__QuakeMapParser__
#define __TrenchBroom__QuakeMapParser__

#include "IO/MapParser.h"
#include "IO/Parser.h"

namespace TrenchBroom {
    namespace IO {
        namespace QuakeMapToken {
            typedef unsigned int Type;
            static const Type Integer       = 1 <<  0; // integer number
            static const Type Decimal       = 1 <<  1; // decimal number
            static const Type String        = 1 <<  2; // string
            static const Type OBrace        = 1 <<  3; // opening brace: {
            static const Type CBrace        = 1 <<  4; // closing brace: }
            static const Type Equals        = 1 <<  5; // equals sign: =
            static const Type Semicolon     = 1 <<  6; // semicolon: ;
            static const Type Eof           = 1 <<  7; // end of file
        }

        class QuakeMapParser : public MapParser, public Parser<QuakeMapToken::Type> {
            
        };
    }
}

#endif /* defined(__TrenchBroom__QuakeMapParser__) */
