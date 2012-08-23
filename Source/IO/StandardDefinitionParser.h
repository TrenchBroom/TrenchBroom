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
#ifndef __TrenchBroom__StandardDefinitionParser__
#define __TrenchBroom__StandardDefinitionParser__

#include "IO/AbstractTokenizer.h"
#include "IO/ParserException.h"
#include "Model/EntityDefinition.h"
#include "Model/PropertyDefinition.h"
#include "Utility/Pool.h"
#include "Utility/String.h"
#include "Utility/VecMath.h"

#include <istream>
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
            static const unsigned int Comma           = 1 << 13;  // comma: ,
        }
        
        namespace TokenizerState {
            static const unsigned int Outside    = 0; // currently outside of a definition
            static const unsigned int Inside     = 1; // currently parsing a definition
            static const unsigned int Comment    = 2; // currently reading a comment
            static const unsigned int Integer    = 3; // currently reading an integer number
            static const unsigned int Decimal    = 4; // currently reading a decimal number
            static const unsigned int Word       = 5; // currently reading a single word
            static const unsigned int String     = 6; // currently reading a quoted string
            static const unsigned int Eof        = 7; // reached end of file
        };
        
        class StandardDefinitionTokenizer : public AbstractTokenizer {
        public:
            class Token : public AbstractToken<unsigned int, Token> {
            public:
                Token(unsigned int type, const String& data, size_t position, size_t line, size_t column) : AbstractToken(type, data, position, line, column) {}
            };
            
            typedef std::auto_ptr<Token> TokenPtr;
        protected:
            unsigned int m_state;
            
            inline TokenPtr MakeToken(unsigned int type, const String& data) {
                return TokenPtr(new Token(type, data, m_stream.tellg(), m_line, m_column));
            }
        public:
            StandardDefinitionTokenizer(const String& name, std::istream& stream) : AbstractTokenizer(name, stream), m_state(TokenizerState::Outside) {}
            
            TokenPtr NextToken();
            TokenPtr PeekToken();
            String Remainder();
        };
        
        class StandardProperty {
        public:
            enum class PropertyType {
                Base,
                Choice,
                Default,
                Model
            };
        protected:
            PropertyType m_type;
        public:
            typedef std::vector<StandardProperty> List;
            
            StandardProperty(PropertyType type) : m_type(type) {}
            
            inline PropertyType Type() const {
                return m_type;
            }
        };
        
        class StandardBaseProperty : public StandardProperty {
        protected:
            String m_basename;
        public:
            StandardBaseProperty(StandardProperty::PropertyType type, const String& basename) : StandardProperty(type), m_basename(basename) {}
            
            inline const String& Basename() const {
                return m_basename;
            }
        };
        
        class StandardChoiceArgument {
        protected:
            int m_key;
            String m_value;
        public:
            typedef std::vector<StandardChoiceArgument> List;
            
            StandardChoiceArgument(int key, const String& value) : m_key(key), m_value(value) {}
            
            inline int Key() const {
                return m_key;
            }
            
            inline const String& Value() const {
                return m_value;
            }
        };

        class StandardChoiceProperty : public StandardProperty {
        protected:
            String m_propertyName;
            StandardChoiceArgument::List m_arguments;
        public:
            StandardChoiceProperty(StandardProperty::PropertyType type, const String& propertyName, const StandardChoiceArgument::List& arguments) :
            StandardProperty(type),
            m_propertyName(propertyName),
            m_arguments(arguments) {}
            
            inline const String& PropertyName() const {
                return m_propertyName;
            }
            
            inline const StandardChoiceArgument::List& Arguments() const {
                return m_arguments;
            }
        };
        
        class StandardDefaultProperty : public StandardProperty {
        protected:
            String m_propertyName;
            String m_propertyValue;
        public:
            StandardDefaultProperty(StandardProperty::PropertyType type, const String& propertyName, const String& propertyValue) :
            StandardProperty(type),
            m_propertyName(propertyName),
            m_propertyValue(propertyValue) {}
            
            inline const String& PropertyName() const {
                return m_propertyName;
            }
            
            inline const String& PropertyValue() const {
                return m_propertyValue;
            }
        };
        
        class StandardModelProperty : public StandardProperty {
        protected:
            String m_modelName;
            String m_flagName;
            unsigned int m_skinIndex;
        public:
            StandardModelProperty(StandardProperty::PropertyType type, const String& modelName, const String& flagName, unsigned int skinIndex) :
            StandardProperty(type),
            m_modelName(modelName),
            m_flagName(flagName),
            m_skinIndex(skinIndex) {}
            
            inline const String& ModelName() const {
                return m_modelName;
            }
            
            inline const String& FlagName() const {
                return m_flagName;
            }
            
            inline unsigned int SkinIndex() const {
                return m_skinIndex;
            }
        };
        
        class StandardDefinitionParser {
        protected:
            
            StandardDefinitionTokenizer m_tokenizer;
            
            String TypeNames(unsigned int types);
            
            inline void Expect(unsigned int types, StandardDefinitionTokenizer::Token* token) {
                if (token == NULL)
                    throw ParserException(m_tokenizer.Path(), m_tokenizer.Line(), m_tokenizer.Column(), "Expected token type " + TypeNames(types) + " but got NULL");
                else if ((token->Type() & types) == 0)
                    throw ParserException(m_tokenizer.Path(), m_tokenizer.Line(), m_tokenizer.Column(), "Expected token type " + TypeNames(types) + " but got " + TypeNames(token->Type()));
            }
            
            StandardDefinitionTokenizer::TokenPtr NextTokenIgnoringNewlines();
            Vec4f ParseColor();
            BBox ParseBounds();
            Model::Spawnflag::List ParseFlags();
            bool ParseProperty(StandardProperty::List& properties);
            StandardProperty::List ParseProperties();
            String ParseDescription();
        public:
            StandardDefinitionParser(const String& name, std::istream& stream) : m_tokenizer(name, stream) {}
        
            Model::EntityDefinition* NextDefinition();
        };
    }
}

#endif /* defined(__TrenchBroom__StandardDefinitionParser__) */
