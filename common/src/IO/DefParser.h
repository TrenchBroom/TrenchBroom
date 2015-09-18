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

#ifndef TrenchBroom_DefParser
#define TrenchBroom_DefParser

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
        namespace DefToken {
            typedef unsigned int Type;
            static const Type Integer         = 1 <<  0; // integer number
            static const Type Decimal         = 1 <<  1; // decimal number
            static const Type QuotedString    = 1 <<  2; // string
            static const Type OParenthesis    = 1 <<  3; // opening parenthesis: (
            static const Type CParenthesis    = 1 <<  4; // closing parenthesis: )
            static const Type OBrace          = 1 <<  5; // opening brace: {
            static const Type CBrace          = 1 <<  6; // closing brace: }
            static const Type Word            = 1 <<  7; // word
            static const Type Question        = 1 <<  8; // question mark: ?
            static const Type ODefinition     = 1 <<  9; // entity definition open
            static const Type CDefinition     = 1 << 10; // entity definition close
            static const Type Semicolon       = 1 << 11; // semicolon: ;
            static const Type Newline         = 1 << 12; // new line
            static const Type Comma           = 1 << 13; // comma: ,
            static const Type Equality        = 1 << 14; // equality sign: =
            static const Type Eof             = 1 << 15; // end of file
        }

        class DefTokenizer : public Tokenizer<DefToken::Type> {
        public:
            DefTokenizer(const char* begin, const char* end);
            DefTokenizer(const String& str);
        private:
            static const String WordDelims;
            Token emitToken();
        };
        
        class DefParser : public EntityDefinitionParser, public Parser<DefToken::Type> {
        private:
            typedef DefTokenizer::Token Token;
            
            Color m_defaultEntityColor;
            DefTokenizer m_tokenizer;
            EntityDefinitionClassInfoMap m_baseClasses;
        public:
            DefParser(const char* begin, const char* end, const Color& defaultEntityColor);
            DefParser(const String& str, const Color& defaultEntityColor);
        private:
            TokenNameMap tokenNames() const;
            Assets::EntityDefinitionList doParseDefinitions(ParserStatus& status);
            
            Assets::EntityDefinition* parseDefinition(ParserStatus& status);
            Assets::AttributeDefinitionPtr parseSpawnflags(ParserStatus& status);
            void parserAttributes(ParserStatus& status, Assets::AttributeDefinitionMap& attributes, Assets::ModelDefinitionList& modelDefinitions, StringList& superClasses);
            bool parseAttribute(ParserStatus& status, Assets::AttributeDefinitionMap& attributes, Assets::ModelDefinitionList& modelDefinitions, StringList& superClasses);
            
            void parseDefaultAttribute(ParserStatus& status);
            void parseBaseAttribute(ParserStatus& status, StringList& superClasses);
            void parseChoiceAttribute(ParserStatus& status, Assets::AttributeDefinitionMap& attributes);
            void parseModelDefinitions(ParserStatus& status, Assets::ModelDefinitionList& modelDefinitions);
            void parseStaticModelDefinition(ParserStatus& status, Assets::ModelDefinitionList& modelDefinitions);
            void parseDynamicModelDefinition(ParserStatus& status, Assets::ModelDefinitionList& modelDefinitions);
            String parseDescription();

            Vec3 parseVector(ParserStatus& status);
            BBox3 parseBounds(ParserStatus& status);
            Color parseColor(ParserStatus& status);
            
            Token nextTokenIgnoringNewlines();
        };
    }
}

#endif /* defined(TrenchBroom_DefParser) */
