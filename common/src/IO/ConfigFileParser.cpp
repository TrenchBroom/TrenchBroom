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

#include "ConfigFileParser.h"
#include "Macros.h"

namespace TrenchBroom {
    namespace IO {
        ConfigFileTokenizer::ConfigFileTokenizer(const char* begin, const char* end) :
        Tokenizer(begin, end) {}
        
        ConfigFileTokenizer::ConfigFileTokenizer(const String& str) :
        Tokenizer(str) {}
        
        ConfigFileTokenizer::Token ConfigFileTokenizer::emitToken() {
            while (!eof()) {
                size_t startLine = line();
                size_t startColumn = column();
                const char* c = curPos();
                switch (*c) {
                    case '/':
                        advance();
                        if (curChar() == '/')
                            discardUntil("\n\r");
                        break;
                    case '{':
                        advance();
                        return Token(ConfigFileToken::OBrace, c, c+1, offset(c), startLine, startColumn);
                    case '}':
                        advance();
                        return Token(ConfigFileToken::CBrace, c, c+1, offset(c), startLine, startColumn);
                    case ',':
                        advance();
                        return Token(ConfigFileToken::Comma, c, c+1, offset(c), startLine, startColumn);
                    case '=':
                        advance();
                        return Token(ConfigFileToken::Equals, c, c+1, offset(c), startLine, startColumn);
                    case '"': {
                        advance();
                        c = curPos();
                        const char* e = readQuotedString();
                        return Token(ConfigFileToken::String, c, e, offset(c), startLine, startColumn);
                    }
                    case ' ':
                    case '\t':
                    case '\n':
                    case '\r':
                        discardWhile(Whitespace());
                        break;
                    default: {
                        const char* e = readString(Whitespace() + "=");
                        if (e == NULL)
                            throw ParserException(startLine, startColumn, "Unexpected character: " + String(c, 1));
                        return Token(ConfigFileToken::Identifier, c, e, offset(c), startLine, startColumn);
                    }
                }
            }
            return Token(ConfigFileToken::Eof, NULL, NULL, length(), line(), column());
        }
        
        ConfigFileParser::ConfigFileParser(const char* begin, const char* end) :
        m_tokenizer(begin, end) {}
        
        ConfigFileParser::ConfigFileParser(const String& str) :
        m_tokenizer(str) {}
        
        ConfigEntry* ConfigFileParser::parse() {
            const Token token = m_tokenizer.nextToken();
            expect(ConfigFileToken::OBrace | ConfigFileToken::String | ConfigFileToken::Eof, token);
            if (token.type() == ConfigFileToken::Eof)
                return NULL;
            
            m_tokenizer.pushToken(token);
            return parseEntry();
        }
        
        ConfigEntry* ConfigFileParser::parseEntry() {
            const Token token = m_tokenizer.nextToken();
            if (token.type() == ConfigFileToken::Eof)
                return NULL;
            
            m_tokenizer.pushToken(token);
            const ConfigEntry::Type entryType = detectEntryType();
            
            switch (entryType) {
                case ConfigEntry::Type_Value:
                    return parseValue();
                case ConfigEntry::Type_List:
                    return parseList();
                case ConfigEntry::Type_Table:
                    return parseTable();
                    switchDefault()
            }
        }
        
        ConfigEntry::Type ConfigFileParser::detectEntryType() {
            const Token firstToken = m_tokenizer.nextToken();
            expect(ConfigFileToken::String | ConfigFileToken::OBrace, firstToken);
            if (firstToken.type() == ConfigFileToken::String) {
                m_tokenizer.pushToken(firstToken);
                return ConfigEntry::Type_Value;
            }
            
            const Token secondToken = m_tokenizer.nextToken();
            expect(ConfigFileToken::Identifier | ConfigFileToken::String | ConfigFileToken::OBrace | ConfigFileToken::CBrace, secondToken);
            
            const ConfigEntry::Type entryType = secondToken.type() == ConfigFileToken::Identifier ? ConfigEntry::Type_Table : ConfigEntry::Type_List;
            
            m_tokenizer.pushToken(secondToken);
            m_tokenizer.pushToken(firstToken);
            return entryType;
        }
        
        ConfigEntry* ConfigFileParser::parseValue() {
            const Token token = m_tokenizer.nextToken();
            expect(ConfigFileToken::String, token);
            ConfigEntry* value = new ConfigValue(token.data(), token.line(), token.column());
            return value;
        }
        
        ConfigEntry* ConfigFileParser::parseList() {
            Token token;
            expect(ConfigFileToken::OBrace, token = m_tokenizer.nextToken());
            ConfigList* list = new ConfigList(token.line(), token.column());
            
            try {
                token = m_tokenizer.nextToken();
                if (token.type() == ConfigFileToken::CBrace)
                    return list;
                
                m_tokenizer.pushToken(token);
                do {
                    list->addEntry(parseEntry());
                    expect(ConfigFileToken::Comma | ConfigFileToken::CBrace, token = m_tokenizer.nextToken());
                } while (token.type() != ConfigFileToken::CBrace);

                return list;
            } catch (...) {
                delete list;
                throw;
            }
        }
        
        ConfigEntry* ConfigFileParser::parseTable() {
            Token token;
            expect(ConfigFileToken::OBrace, token = m_tokenizer.nextToken());
            ConfigTable* table = new ConfigTable(token.line(), token.column());
            
            try {
                token = m_tokenizer.nextToken();
                if (token.type() == ConfigFileToken::CBrace)
                    return table;
                
                m_tokenizer.pushToken(token);
                do {
                    expect(ConfigFileToken::Identifier, token = m_tokenizer.nextToken());
                    const String key = token.data();
                    expect(ConfigFileToken::Equals, token = m_tokenizer.nextToken());
                    table->addEntry(key, parseEntry());
                    expect(ConfigFileToken::Comma | ConfigFileToken::CBrace, token = m_tokenizer.nextToken());
                } while (token.type() != ConfigFileToken::CBrace);

                return table;
            } catch (...) {
                delete table;
                throw;
            }
        }
        
        ConfigFileParser::TokenNameMap ConfigFileParser::tokenNames() const {
            using namespace ConfigFileToken;
            
            TokenNameMap names;
            names[Identifier]   = "identifier";
            names[String]       = "string";
            names[OBrace]       = "'{'";
            names[CBrace]       = "'}'";
            names[Comma]        = "','";
            names[Equals]       = "'='";
            names[Comment]      = "comment";
            names[Eof]          = "end of file";
            return names;
        }
    }
}
