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
#include "IO/Token.h"
#include "IO/Tokenizer.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/Map.h"

namespace TrenchBroom {
    namespace Model {
        class Map;
    }
    
    namespace IO {
        namespace QuakeMapToken {
            typedef unsigned int Type;
            static const Type Integer       = 1 <<  0; // integer number
            static const Type Decimal       = 1 <<  1; // decimal number
            static const Type String        = 1 <<  2; // string
            static const Type OParenthesis  = 1 <<  3; // opening parenthesis: (
            static const Type CParenthesis  = 1 <<  4; // closing parenthesis: )
            static const Type OBrace        = 1 <<  5; // opening brace: {
            static const Type CBrace        = 1 <<  6; // closing brace: }
            static const Type OBracket      = 1 <<  7; // opening bracket: [
            static const Type CBracket      = 1 <<  8; // closing bracket: ]
            static const Type Comment       = 1 <<  9; // line comment starting with //
            static const Type Eof           = 1 << 10; // end of file
        }

        class QuakeMapTokenizer : public Tokenizer<QuakeMapToken::Type> {
        public:
            QuakeMapTokenizer(const char* begin, const char* end);
            QuakeMapTokenizer(const String& str);
        private:
            Token emitToken();
        };
        
        class QuakeMapParser : public MapParser, public Parser<QuakeMapToken::Type> {
        private:
            QuakeMapTokenizer m_tokenizer;
            typedef QuakeMapTokenizer::Token Token;
        public:
            QuakeMapParser(const char* begin, const char* end);
            QuakeMapParser(const String& str);
        private:
            String tokenName(const QuakeMapToken::Type typeMask) const;
            Model::Map::Ptr doParseMap(const BBox3& worldBounds);
            
            Model::Entity::Ptr parseEntity(const BBox3& worldBounds);
            Model::Brush::Ptr parseBrush(const BBox3& worldBounds);
            Model::BrushFace::Ptr parseFace(const BBox3& worldBounds);
            const Vec3 parseVector();
        };
    }
}

#endif /* defined(__TrenchBroom__QuakeMapParser__) */
