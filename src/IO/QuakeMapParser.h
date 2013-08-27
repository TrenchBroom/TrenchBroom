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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__QuakeMapParser__
#define __TrenchBroom__QuakeMapParser__

#include "IO/MapParser.h"
#include "IO/Parser.h"
#include "IO/Token.h"
#include "IO/Tokenizer.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    class Logger;
    
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
            class PlaneWeightOrder {
            private:
                bool m_deterministic;
            public:
                PlaneWeightOrder(const bool deterministic);
                template <typename T, size_t S>
                inline bool operator()(const Plane<T,S>& lhs, const Plane<T,S>& rhs) const {
                    int result = lhs.normal.weight() - rhs.normal.weight();
                    if (m_deterministic)
                        result += static_cast<int>(1000.0 * (lhs.distance - lhs.distance));
                    return result < 0;
                }
            };
            
            class FaceWeightOrder {
            private:
                const PlaneWeightOrder& m_planeOrder;
            public:
                FaceWeightOrder(const PlaneWeightOrder& planeOrder);
                bool operator()(const Model::BrushFace* lhs, const Model::BrushFace* rhs) const;
            };
                    
            Logger* m_logger;
            QuakeMapTokenizer m_tokenizer;
            Model::MapFormat m_format;
            typedef QuakeMapTokenizer::Token Token;
        public:
            QuakeMapParser(const char* begin, const char* end, Logger* logger = NULL);
            QuakeMapParser(const String& str, Logger* logger = NULL);
        private:
            String tokenName(const QuakeMapToken::Type typeMask) const;
            Model::Map* doParseMap(const BBox3& worldBounds);
            
            Model::MapFormat detectFormat();
            Model::Entity* parseEntity(const BBox3& worldBounds);
            Model::Brush* parseBrush(const BBox3& worldBounds);
            Model::BrushFace* parseFace(const BBox3& worldBounds);
            Vec3 parseVector();
            
            Model::Brush* createBrush(const BBox3& worldBounds, const Model::BrushFaceList faces, const size_t firstLine, const size_t lineCount) const;
        };
    }
}

#endif /* defined(__TrenchBroom__QuakeMapParser__) */
