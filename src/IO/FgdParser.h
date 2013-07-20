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

#ifndef __TrenchBroom__FgdParser__
#define __TrenchBroom__FgdParser__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Color.h"
#include "StringUtils.h"
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
            
            Color m_defaultEntityColor;
            FgdTokenizer m_tokenizer;
            EntityDefinitionClassInfoMap m_baseClasses;
        public:
            FgdParser(const char* begin, const char* end, const Color& defaultEntityColor);
            FgdParser(const String& str, const Color& defaultEntityColor);
        private:
            String tokenName(const FgdToken::Type typeMask) const;
            Model::EntityDefinitionList doParseDefinitions();
            
            Model::EntityDefinition* parseDefinition();
            Model::EntityDefinition* parseSolidClass();
            Model::EntityDefinition* parsePointClass();
            EntityDefinitionClassInfo parseBaseClass();
            EntityDefinitionClassInfo parseClass();
            StringList parseSuperClasses();
            Model::ModelDefinitionList parseModels();
            Model::ModelDefinitionPtr parseStaticModel();
            Model::ModelDefinitionPtr parseDynamicModel();
            Model::PropertyDefinitionMap parseProperties();
            Model::PropertyDefinitionPtr parseTargetSourceProperty(const String& name);
            Model::PropertyDefinitionPtr parseTargetDestinationProperty(const String& name);
            Model::PropertyDefinitionPtr parseStringProperty(const String& name);
            Model::PropertyDefinitionPtr parseIntegerProperty(const String& name);
            Model::PropertyDefinitionPtr parseFloatProperty(const String& name);
            Model::PropertyDefinitionPtr parseChoicesProperty(const String& name);
            Model::PropertyDefinitionPtr parseFlagsProperty(const String& name);
            
            Vec3 parseVector();
            BBox3 parseSize();
            Color parseColor();
        };
    }
}

#endif /* defined(__TrenchBroom__FgdParser__) */
