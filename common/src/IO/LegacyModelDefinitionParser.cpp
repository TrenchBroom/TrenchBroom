/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "LegacyModelDefinitionParser.h"

namespace TrenchBroom {
    namespace IO {
        LegacyModelDefinitionTokenizer::LegacyModelDefinitionTokenizer(const char* begin, const char* end) :
        Tokenizer(begin, end) {}
        
        LegacyModelDefinitionTokenizer::LegacyModelDefinitionTokenizer(const String& str) :
        Tokenizer(str) {}
        
        const String LegacyModelDefinitionTokenizer::WordDelims = " \t\n\r()[]{};,=";
        
        LegacyModelDefinitionTokenizer::Token LegacyModelDefinitionTokenizer::emitToken() {
            
        }

        LegacyModelDefinitionParser::LegacyModelDefinitionParser(const char* begin, const char* end) :
        m_tokenizer(begin, end) {}
        
        LegacyModelDefinitionParser::LegacyModelDefinitionParser(const String& str) :
        m_tokenizer(str) {}
        
        Assets::ModelDefinition LegacyModelDefinitionParser::parse(ParserStatus& status) {
            return parseModelDefinition(status);
        }

        Assets::ModelDefinition LegacyModelDefinitionParser::parseModelDefinition(ParserStatus& status) {
            
        }
        
        Assets::ModelDefinition LegacyModelDefinitionParser::parseStaticModelDefinition(ParserStatus& status) {
            
        }
        
        Assets::ModelDefinition LegacyModelDefinitionParser::parseDynamicModelDefinition(ParserStatus& status) {
            
        }

        LegacyModelDefinitionParser::TokenNameMap LegacyModelDefinitionParser::tokenNames() const {
            using namespace MdlToken;
            
            TokenNameMap names;
            names[Integer]      = "integer";
            names[String]       = "quoted string";
            names[Word]         = "word";
            names[Comma]        = "','";
            names[Equality]     = "'='";
            names[Eof]          = "end of file";
            return names;
        }
    }
}
