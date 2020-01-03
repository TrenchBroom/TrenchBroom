/*
 Copyright (C) 2010-2017 Kristian Duske

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

#ifndef TrenchBroom_StandardMapParser
#define TrenchBroom_StandardMapParser

#include "FloatType.h"
#include "IO/MapParser.h"
#include "IO/Parser.h"
#include "IO/Tokenizer.h"
#include "Model/MapFormat.h"

#include <kdl/vector_set_forward.h>

#include <vecmath/forward.h>

#include <string>
#include <tuple>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        class ParserStatus;

        namespace QuakeMapToken {
            using Type = unsigned int;
            static const Type Integer       = 1 <<  0; // integer number
            static const Type Decimal       = 1 <<  1; // decimal number
            static const Type String        = 1 <<  2; // string
            static const Type OParenthesis  = 1 <<  3; // opening parenthesis: (
            static const Type CParenthesis  = 1 <<  4; // closing parenthesis: )
            static const Type OBrace        = 1 <<  5; // opening brace: {
            static const Type CBrace        = 1 <<  6; // closing brace: }
            static const Type OBracket      = 1 <<  7; // opening bracket: [
            static const Type CBracket      = 1 <<  8; // closing bracket: ]
            static const Type Comment       = 1 <<  9; // line comment starting with ///
            static const Type Eof           = 1 << 10; // end of file
            static const Type Eol           = 1 << 11; // end of line
            static const Type Number        = Integer | Decimal;
        }

        class QuakeMapTokenizer : public Tokenizer<QuakeMapToken::Type> {
        private:
            static const std::string& NumberDelim();
            bool m_skipEol;
        public:
            QuakeMapTokenizer(const char* begin, const char* end);
            explicit QuakeMapTokenizer(const std::string& str);

            void setSkipEol(bool skipEol);
        private:
            Token emitToken() override;
        };

        class StandardMapParser : public MapParser, public Parser<QuakeMapToken::Type> {
        private:
            using Token = QuakeMapTokenizer::Token;
            using AttributeNames = kdl::vector_set<std::string>;

            static const std::string BrushPrimitiveId;
            static const std::string PatchId;

            QuakeMapTokenizer m_tokenizer;
            Model::MapFormat m_format;
        public:
            StandardMapParser(const char* begin, const char* end);
            explicit StandardMapParser(const std::string& str);

            ~StandardMapParser() override;
        protected:
            Model::MapFormat detectFormat();

            void parseEntities(Model::MapFormat format, ParserStatus& status);
            void parseBrushes(Model::MapFormat format, ParserStatus& status);
            void parseBrushFaces(Model::MapFormat format, ParserStatus& status);

            void reset();
        private:
            void setFormat(Model::MapFormat format);

            void parseEntity(ParserStatus& status);
            void parseEntityAttribute(std::vector<Model::EntityAttribute>& attributes, AttributeNames& names, ParserStatus& status);

            void parseBrushOrBrushPrimitiveOrPatch(ParserStatus& status);
            void parseBrushPrimitive(ParserStatus& status, size_t startLine);
            void parseBrush(ParserStatus& status, size_t startLine, bool primitive);

            void parseFace(ParserStatus& status, bool primitive);
            void parseQuakeFace(ParserStatus& status);
            void parseQuake2Face(ParserStatus& status);
            void parseHexen2Face(ParserStatus& status);
            void parseDaikatanaFace(ParserStatus& status);
            void parseValveFace(ParserStatus& status);
            void parsePrimitiveFace(ParserStatus& status);
            bool checkFacePoints(ParserStatus& status, const vm::vec3& p1, const vm::vec3& p2, const vm::vec3& p3, size_t line) const;

            void parsePatch(ParserStatus& status, size_t startLine);

            std::tuple<vm::vec3, vm::vec3, vm::vec3> parseFacePoints(ParserStatus& status);
            std::string parseTextureName(ParserStatus& status);
            std::tuple<vm::vec3, float, vm::vec3, float> parseValveTextureAxes(ParserStatus& status);
            std::tuple<vm::vec3, vm::vec3> parsePrimitiveTextureAxes(ParserStatus& status);

            template <size_t S=3, typename T=FloatType>
            vm::vec<T,S> parseFloatVector(const QuakeMapToken::Type o, const QuakeMapToken::Type c) {
                expect(o, m_tokenizer.nextToken());
                vm::vec<T,S> vec;
                for (size_t i = 0; i < S; i++) {
                    vec[i] = expect(QuakeMapToken::Number, m_tokenizer.nextToken()).toFloat<T>();
                }
                expect(c, m_tokenizer.nextToken());
                return vec;
            }

            float parseFloat();
            int parseInteger();

            void parseExtraAttributes(ExtraAttributes& extraAttributes, ParserStatus& status);
        private: // implement Parser interface
            TokenNameMap tokenNames() const override;
        };
    }
}

#endif /* defined(TrenchBroom_StandardMapParser) */
