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

#ifndef TrenchBroom_FgdParser
#define TrenchBroom_FgdParser

#include "FloatType.h"
#include "Color.h"
#include "IO/EntityDefinitionClassInfo.h"
#include "IO/EntityDefinitionParser.h"
#include "IO/Parser.h"
#include "IO/Tokenizer.h"

#include <memory>
#include <nonstd/optional.hpp>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class ModelDefinition;
    }

    namespace IO {
        class FileSystem;
        class ParserStatus;
        class Path;

        namespace FgdToken {
            using Type = unsigned int;
            static const Type Integer           = 1 <<  0; // integer number
            static const Type Decimal           = 1 <<  1; // decimal number
            static const Type Word              = 1 <<  2; // letter or digits, no whitespace
            static const Type String            = 1 <<  3; // "anything but quotes"
            static const Type OParenthesis      = 1 <<  4; // opening parenthesis: (
            static const Type CParenthesis      = 1 <<  5; // closing parenthesis: )
            static const Type OBracket          = 1 <<  6; // opening bracket: [
            static const Type CBracket          = 1 <<  7; // closing bracket: ]
            static const Type Equality          = 1 <<  8; // equality sign: =
            static const Type Colon             = 1 <<  9; // colon: :
            static const Type Comma             = 1 << 10; // comma: ,
            static const Type Plus              = 1 << 11; // plus: + (not used in string continuations)
            static const Type Eof               = 1 << 12; // end of file
        }

        class FgdTokenizer : public Tokenizer<FgdToken::Type> {
        public:
            FgdTokenizer(const char* begin, const char* end);
            explicit FgdTokenizer(const std::string& str);
        private:
            static const std::string WordDelims;
            Token emitToken() override;
        };

        class FgdParser : public EntityDefinitionParser, public Parser<FgdToken::Type> {
        private:
            using Token = FgdTokenizer::Token;

            Color m_defaultEntityColor;

            std::vector<Path> m_paths;
            std::shared_ptr<FileSystem> m_fs;

            FgdTokenizer m_tokenizer;
            std::map<std::string, EntityDefinitionClassInfo> m_baseClasses;
        public:
            FgdParser(const char* begin, const char* end, const Color& defaultEntityColor, const Path& path);
            FgdParser(const std::string& str, const Color& defaultEntityColor, const Path& path);
            FgdParser(const std::string& str, const Color& defaultEntityColor);
        private:
            class PushIncludePath;
            void pushIncludePath(const Path& path);
            void popIncludePath();

            bool isRecursiveInclude(const Path& path) const;
        private:
            TokenNameMap tokenNames() const override;
            EntityDefinitionList doParseDefinitions(ParserStatus& status) override;

            void parseDefinitionOrInclude(ParserStatus& status, EntityDefinitionList& definitions);

            Assets::EntityDefinition* parseDefinition(ParserStatus& status);
            Assets::EntityDefinition* parseSolidClass(ParserStatus& status);
            Assets::EntityDefinition* parsePointClass(ParserStatus& status);
            EntityDefinitionClassInfo parseBaseClass(ParserStatus& status);
            EntityDefinitionClassInfo parseClass(ParserStatus& status);
            void skipMainClass(ParserStatus& status);

            std::vector<std::string> parseSuperClasses(ParserStatus& status);
            Assets::ModelDefinition parseModel(ParserStatus& status);
            std::string parseNamedValue(ParserStatus& status, const std::string& name);
            void skipClassAttribute(ParserStatus& status);

            AttributeDefinitionMap parseProperties(ParserStatus& status);
            AttributeDefinitionPtr parseTargetSourceAttribute(ParserStatus& status, const std::string& name);
            AttributeDefinitionPtr parseTargetDestinationAttribute(ParserStatus& status, const std::string& name);
            AttributeDefinitionPtr parseStringAttribute(ParserStatus& status, const std::string& name);
            AttributeDefinitionPtr parseIntegerAttribute(ParserStatus& status, const std::string& name);
            AttributeDefinitionPtr parseFloatAttribute(ParserStatus& status, const std::string& name);
            AttributeDefinitionPtr parseChoicesAttribute(ParserStatus& status, const std::string& name);
            AttributeDefinitionPtr parseFlagsAttribute(ParserStatus& status, const std::string& name);
            AttributeDefinitionPtr parseUnknownAttribute(ParserStatus& status, const std::string& name);

            bool parseReadOnlyFlag(ParserStatus& status);
            std::string parseAttributeDescription(ParserStatus& status);
            nonstd::optional<std::string> parseDefaultStringValue(ParserStatus& status);
            nonstd::optional<int> parseDefaultIntegerValue(ParserStatus& status);
            nonstd::optional<float> parseDefaultFloatValue(ParserStatus& status);
            nonstd::optional<std::string> parseDefaultChoiceValue(ParserStatus& status);

            vm::vec3 parseVector(ParserStatus& status);
            vm::bbox3 parseSize(ParserStatus& status);
            Color parseColor(ParserStatus& status);
            std::string parseString(ParserStatus& status);

            EntityDefinitionList parseInclude(ParserStatus& status);
            EntityDefinitionList handleInclude(ParserStatus& status, const Path& path);
        };
    }
}

#endif /* defined(TrenchBroom_FgdParser) */
