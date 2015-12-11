/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Color.h"
#include "StringUtils.h"
#include "Assets/AssetTypes.h"
#include "IO/EntityDefinitionClassInfo.h"
#include "IO/EntityDefinitionParser.h"
#include "IO/Parser.h"
#include "IO/Token.h"
#include "IO/Tokenizer.h"

namespace TrenchBroom {
    namespace IO {
        namespace FgdToken {
            typedef unsigned int Type;
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
            static const Type Eof               = 1 << 11; // end of file
        }
        
        class FgdTokenizer : public Tokenizer<FgdToken::Type> {
        public:
            FgdTokenizer(const char* begin, const char* end);
            FgdTokenizer(const String& str);
        private:
            static const String WordDelims;
            Token emitToken();
        };

        class FgdParser : public EntityDefinitionParser, public Parser<FgdToken::Type> {
        private:
            typedef FgdTokenizer::Token Token;
            
            template <typename T>
            struct DefaultValue {
                bool present;
                T value;
                
                DefaultValue() : present(false) {}
                DefaultValue(const T& i_value) : present(true), value(i_value) {}
            };
            
            Color m_defaultEntityColor;
            FgdTokenizer m_tokenizer;
            EntityDefinitionClassInfoMap m_baseClasses;
        public:
            FgdParser(const char* begin, const char* end, const Color& defaultEntityColor);
            FgdParser(const String& str, const Color& defaultEntityColor);
        private:
            TokenNameMap tokenNames() const;
            Assets::EntityDefinitionList doParseDefinitions(ParserStatus& status);
            
            Assets::EntityDefinition* parseDefinition(ParserStatus& status);
            Assets::EntityDefinition* parseSolidClass(ParserStatus& status);
            Assets::EntityDefinition* parsePointClass(ParserStatus& status);
            EntityDefinitionClassInfo parseBaseClass(ParserStatus& status);
            EntityDefinitionClassInfo parseClass(ParserStatus& status);
            void skipMainClass(ParserStatus& status);
            
            StringList parseSuperClasses(ParserStatus& status);
            Assets::ModelDefinitionList parseModels(ParserStatus& status);
            Assets::ModelDefinitionPtr parseStaticModel(ParserStatus& status);
            Assets::ModelDefinitionPtr parseDynamicModel(ParserStatus& status);
            void skipClassAttribute(ParserStatus& status);
            
            Assets::AttributeDefinitionMap parseProperties(ParserStatus& status);
            Assets::AttributeDefinitionPtr parseTargetSourceAttribute(ParserStatus& status, const String& name);
            Assets::AttributeDefinitionPtr parseTargetDestinationAttribute(ParserStatus& status, const String& name);
            Assets::AttributeDefinitionPtr parseStringAttribute(ParserStatus& status, const String& name);
            Assets::AttributeDefinitionPtr parseIntegerAttribute(ParserStatus& status, const String& name);
            Assets::AttributeDefinitionPtr parseFloatAttribute(ParserStatus& status, const String& name);
            Assets::AttributeDefinitionPtr parseChoicesAttribute(ParserStatus& status, const String& name);
            Assets::AttributeDefinitionPtr parseFlagsAttribute(ParserStatus& status, const String& name);
            Assets::AttributeDefinitionPtr parseUnknownAttribute(ParserStatus& status, const String& name);
            
            String parseAttributeDescription(ParserStatus& status);
            DefaultValue<String> parseDefaultStringValue(ParserStatus& status);
            DefaultValue<int> parseDefaultIntegerValue(ParserStatus& status);
            DefaultValue<float> parseDefaultFloatValue(ParserStatus& status);
            
            Vec3 parseVector(ParserStatus& status);
            BBox3 parseSize(ParserStatus& status);
            Color parseColor(ParserStatus& status);
        };
    }
}

#endif /* defined(TrenchBroom_FgdParser) */
