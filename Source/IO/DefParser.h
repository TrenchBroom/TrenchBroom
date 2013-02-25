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

#ifndef __TrenchBroom__DefParser__
#define __TrenchBroom__DefParser__

#include "IO/Tokenizer.h"
#include "IO/ParserException.h"
#include "Model/EntityDefinition.h"
#include "Model/EntityDefinitionTypes.h"
#include "Model/PropertyDefinition.h"
#include "Utility/Color.h"
#include "Utility/String.h"
#include "Utility/VecMath.h"

#include <istream>
#include <map>
#include <memory>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class EntityDefinition;
    }
    
    namespace IO {
        namespace TokenType {
            static const unsigned int Integer         = 1 <<  0; // integer number
            static const unsigned int Decimal         = 1 <<  1; // decimal number
            static const unsigned int String          = 1 <<  2; // string
            static const unsigned int OParenthesis    = 1 <<  3; // opening parenthesis: (
            static const unsigned int CParenthesis    = 1 <<  4; // closing parenthesis: )
            static const unsigned int OBrace          = 1 <<  5; // opening brace: {
            static const unsigned int CBrace          = 1 <<  6; // closing brace: }
            static const unsigned int Word            = 1 <<  7; // word
            static const unsigned int Question        = 1 <<  8; // question mark: ?
            static const unsigned int ODefinition     = 1 <<  9; // entity definition open
            static const unsigned int CDefinition     = 1 << 10; // entity definition close
            static const unsigned int Semicolon       = 1 << 11; // semicolon: ;
            static const unsigned int Newline         = 1 << 12; // new line
            static const unsigned int Comma           = 1 << 13; // comma: ,
            static const unsigned int Equality        = 1 << 14; // equality sign: =
            static const unsigned int Eof             = 1 << 15; // end of file
        }
        
        class DefTokenEmitter : public TokenEmitter<DefTokenEmitter> {
        private:
            StringStream m_buffer;
        protected:
            bool isDelimiter(char c) {
                return isWhitespace(c) || c == '(' || c == ')' || c == '{' || c == '}' || c == '?' || c == ';' || c == ',' || c == '=';
            }

            Token doEmit(Tokenizer& tokenizer, size_t line, size_t column);
        };
        
        class DefParser {
        protected:
            typedef std::map<String, Model::PropertyDefinition::List> BasePropertiesMap;
            
            StringTokenizer<DefTokenEmitter> m_tokenizer;
            BasePropertiesMap m_baseProperties;
            
            String typeNames(unsigned int types);
            
            inline void expect(unsigned int types, Token& token) {
                if ((token.type() & types) == 0)
                    throw ParserException(token.line(), token.column(), "Expected token type " + typeNames(types) + " but got " + typeNames(token.type()));
            }
            
            Token nextTokenIgnoringNewlines();
            Color parseColor();
            BBox parseBounds();
            Model::FlagsPropertyDefinition* parseFlags();
            bool parseProperty(Model::PropertyDefinition::List& properties, Model::ModelDefinition::List& modelDefinitions, StringList& baseClasses);
            void parseProperties(Model::PropertyDefinition::List& properties, Model::ModelDefinition::List& modelDefinitions, StringList& baseClasses);
            String parseDescription();
        public:
            DefParser(std::istream& stream) : m_tokenizer(stream) {}
            ~DefParser();
        
            Model::EntityDefinition* nextDefinition();
        };
    }
}

#endif /* defined(__TrenchBroom__DefParser__) */
