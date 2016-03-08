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

#include "ConfigParser.h"
#include "Macros.h"

namespace TrenchBroom {
    namespace IO {
        ConfigTokenizer::ConfigTokenizer(const char* begin, const char* end) :
        Tokenizer(begin, end) {}
        
        ConfigTokenizer::ConfigTokenizer(const String& str) :
        Tokenizer(str) {}

        ConfigTokenizer::Token ConfigTokenizer::emitToken() {
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
                        return Token(ConfigToken::OBrace, c, c+1, offset(c), startLine, startColumn);
                    case '}':
                        advance();
                        return Token(ConfigToken::CBrace, c, c+1, offset(c), startLine, startColumn);
                    case ',':
                        advance();
                        return Token(ConfigToken::Comma, c, c+1, offset(c), startLine, startColumn);
                    case '=':
                        advance();
                        return Token(ConfigToken::Equals, c, c+1, offset(c), startLine, startColumn);
                    case '"': {
                        advance();
                        c = curPos();
                        const char* e = readQuotedString();
                        return Token(ConfigToken::String, c, e, offset(c), startLine, startColumn);
                    }
                    case ' ':
                    case '\t':
                    case '\n':
                    case '\r':
                        discardWhile(Whitespace);
                        break;
                    default: {
                        const char* e = readString(Whitespace + "=");
                        if (e == NULL)
                            throw ParserException(startLine, startColumn, "Unexpected character: " + String(c, 1));
                        return Token(ConfigToken::Identifier, c, e, offset(c), startLine, startColumn);
                    }
                }
            }
            return Token(ConfigToken::Eof, NULL, NULL, length(), line(), column());
        }

        ConfigParser::ConfigParser(const char* begin, const char* end) :
        m_tokenizer(begin, end) {}
        
        ConfigParser::ConfigParser(const String& str) :
        m_tokenizer(str) {}

        ConfigEntry::Ptr ConfigParser::parse() {
            const Token token = m_tokenizer.nextToken();
            expect(ConfigToken::OBrace | ConfigToken::String | ConfigToken::Eof, token);
            if (token.type() == ConfigToken::Eof)
                return ConfigEntry::Ptr();

            m_tokenizer.pushToken(token);
            return parseEntry();
        }
        
        ConfigEntry::Ptr ConfigParser::parseEntry() {
            const Token token = m_tokenizer.nextToken();
            if (token.type() == ConfigToken::Eof)
                return ConfigEntry::Ptr();
            
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
        
        ConfigEntry::Type ConfigParser::detectEntryType() {
            const Token firstToken = m_tokenizer.nextToken();
            expect(ConfigToken::String | ConfigToken::OBrace, firstToken);
            if (firstToken.type() == ConfigToken::String) {
                m_tokenizer.pushToken(firstToken);
                return ConfigEntry::Type_Value;
            }
            
            const Token secondToken = m_tokenizer.nextToken();
            expect(ConfigToken::Identifier | ConfigToken::String | ConfigToken::OBrace | ConfigToken::CBrace, secondToken);

            const ConfigEntry::Type entryType = secondToken.type() == ConfigToken::Identifier ? ConfigEntry::Type_Table : ConfigEntry::Type_List;
            
            m_tokenizer.pushToken(secondToken);
            m_tokenizer.pushToken(firstToken);
            return entryType;
        }

        ConfigEntry::Ptr ConfigParser::parseValue() {
            const Token token = m_tokenizer.nextToken();
            expect(ConfigToken::String, token);
            return ConfigValue::Ptr(new ConfigValue(token.data()));
        }

        ConfigEntry::Ptr ConfigParser::parseList() {
            ConfigList::Ptr list(new ConfigList());
            
            Token token;
            expect(ConfigToken::OBrace, token = m_tokenizer.nextToken());
            token = m_tokenizer.nextToken();
            if (token.type() == ConfigToken::CBrace)
                return list;
            
            m_tokenizer.pushToken(token);
            do {
                list->addEntry(parseEntry());
                expect(ConfigToken::Comma | ConfigToken::CBrace, token = m_tokenizer.nextToken());
            } while (token.type() != ConfigToken::CBrace);
            
            return list;
        }
        
        ConfigEntry::Ptr ConfigParser::parseTable() {
            ConfigTable::Ptr table(new ConfigTable());
            
            Token token;
            expect(ConfigToken::OBrace, token = m_tokenizer.nextToken());
            token = m_tokenizer.nextToken();
            if (token.type() == ConfigToken::CBrace)
                return table;
            
            m_tokenizer.pushToken(token);
            do {
                expect(ConfigToken::Identifier, token = m_tokenizer.nextToken());
                const String key = token.data();
                expect(ConfigToken::Equals, token = m_tokenizer.nextToken());
                table->addEntry(key, parseEntry());
                expect(ConfigToken::Comma | ConfigToken::CBrace, token = m_tokenizer.nextToken());
            } while (token.type() != ConfigToken::CBrace);
            
            return table;
        }

        ConfigParser::TokenNameMap ConfigParser::tokenNames() const {
            using namespace ConfigToken;
            
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
