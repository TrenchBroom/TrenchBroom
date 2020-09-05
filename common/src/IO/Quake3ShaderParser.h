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

#ifndef TRENCHBROOM_Q3SHADERPARSER_H
#define TRENCHBROOM_Q3SHADERPARSER_H

#include "IO/Parser.h"
#include "IO/Tokenizer.h"

#include <string>

namespace TrenchBroom {
    namespace Assets {
        class Quake3Shader;
        class Quake3ShaderStage;
    }

    namespace IO {
        class ParserStatus;

        namespace Quake3ShaderToken {
            using Type = unsigned int;
            static const Type Number        = 1 << 1; // decimal number
            static const Type String        = 1 << 2; // string
            static const Type Variable      = 1 << 3; // variable starting with $
            static const Type OBrace        = 1 << 4; // opening brace: {
            static const Type CBrace        = 1 << 5; // closing brace: }
            static const Type Comment       = 1 << 6; // line comment starting with //
            static const Type Eol           = 1 << 7; // end of line
            static const Type Eof           = 1 << 8; // end of file
        }

        class Quake3ShaderTokenizer : public Tokenizer<Quake3ShaderToken::Type> {
        public:
            Quake3ShaderTokenizer(const char* begin, const char* end);
            explicit Quake3ShaderTokenizer(const std::string& str);
        private:
            Token emitToken() override;
        };

        class Quake3ShaderParser : public Parser<Quake3ShaderToken::Type> {
        private:
            Quake3ShaderTokenizer m_tokenizer;
        public:
            Quake3ShaderParser(const char* begin, const char* end);
            explicit Quake3ShaderParser(const std::string& str);

            /**
             * Parses a Quake 3 shader and returns the value of the qer_editorimage entry.
             *
             * @return the value of the qer_editorimage entry or an empty string if no such value was found
             *
             * @throws ParserException if the shader is not well-formed
             */
            std::vector<Assets::Quake3Shader> parse(ParserStatus& status);
        private:
            void parseTexture(Assets::Quake3Shader& shader, ParserStatus& status);
            void parseBody(Assets::Quake3Shader& shader, ParserStatus& status);
            void parseStage(Assets::Quake3Shader& shader, ParserStatus& status);
            void parseBodyEntry(Assets::Quake3Shader& shader, ParserStatus& status);
            void parseStageEntry(Assets::Quake3ShaderStage& stage, ParserStatus& status);
            void skipRemainderOfEntry();
        private:
            TokenNameMap tokenNames() const override;
        };
    }
}

#endif //TRENCHBROOM_Q3SHADERPARSER_H
