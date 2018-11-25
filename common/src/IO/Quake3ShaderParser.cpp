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

#include "Quake3ShaderParser.h"

namespace TrenchBroom {
    namespace IO {
        Quake3ShaderTokenizer::Quake3ShaderTokenizer(const char* begin, const char* end) :
        Tokenizer(begin, end, "", '\\') {}

        Quake3ShaderTokenizer::Quake3ShaderTokenizer(const String &str) :
        Tokenizer(str, "", '\\') {}

        Tokenizer<unsigned int>::Token Quake3ShaderTokenizer::emitToken() {
            while (!eof()) {
                const auto startLine = line();
                const auto startColumn = column();
                const auto* c = curPos();
                switch (*c) {
                    case '{':
                        advance();
                        return Token(Quake3ShaderToken::OBrace, c, c + 1, offset(c), startLine, startColumn);
                    case '}':
                        advance();
                        return Token(Quake3ShaderToken::CBrace, c, c + 1, offset(c), startLine, startColumn);
                    case '\n':
                        discardWhile(Whitespace()); // handle empty lines and such
                        return Token(Quake3ShaderToken::Eol, c, c + 1, offset(c), startLine, startColumn);
                    case '\r':
                    case ' ':
                    case '\t':
                        break;
                    case '/':
                        if (lookAhead() == '/') {
                            discardUntil("\n\r");
                            discardWhile("\n\r");
                            break;
                        }
                        // fall through into the default case to parse a string that starts with '/'
                        switchFallthrough();
                    case '$': {
                        const auto* e = readUntil(Whitespace());
                        if (e == nullptr) {
                            throw ParserException(startLine, startColumn, "Unexpected character: " + String(c, 1));
                        }
                        return Token(Quake3ShaderToken::Variable, c, e, offset(c), startLine, startColumn);
                    }
                    default:
                        auto* e = readDecimal(Whitespace());
                        if (e != nullptr) {
                            return Token(Quake3ShaderToken::Number, c, e, offset(c), startLine, startColumn);
                        }

                        e = readUntil(Whitespace());
                        if (e == nullptr) {
                            throw ParserException(startLine, startColumn, "Unexpected character: " + String(c, 1));
                        }
                        return Token(Quake3ShaderToken::String, c, e, offset(c), startLine, startColumn);
                }
            }
            return Token(Quake3ShaderToken::Eof, nullptr, nullptr, length(), line(), column());
        }

        Quake3ShaderParser::Quake3ShaderParser(const char* begin, const char* end) :
        m_tokenizer(begin, end) {}

        Quake3ShaderParser::Quake3ShaderParser(const String& str) :
        m_tokenizer(str) {}

        String Quake3ShaderParser::parse() {
            if (m_tokenizer.peekToken().hasType(Quake3ShaderToken::Eof)) {
                return "";
            }
            return parseBlock();
        }

        String Quake3ShaderParser::parseBlock() {
            expect(Quake3ShaderToken::OBrace, m_tokenizer.nextToken());
            auto token = m_tokenizer.peekToken();
            expect(Quake3ShaderToken::CBrace | Quake3ShaderToken::OBrace | Quake3ShaderToken::String, token);

            while (!token.hasType(Quake3ShaderToken::CBrace)) {
                if (token.hasType(Quake3ShaderToken::OBrace)) {
                    const auto result = parseBlock();
                    if (!result.empty()) {
                        return result;
                    }
                } else {
                    const auto result = parseEntry();
                    if (!result.empty()) {
                        return result;
                    }
                }
            }
            expect(Quake3ShaderToken::CBrace, m_tokenizer.nextToken());
            return "";
        }

        String Quake3ShaderParser::parseEntry() {
            auto token = m_tokenizer.nextToken();
            expect(Quake3ShaderToken::String, token);
            const auto key = token.data();
            if (key == "qer_editorimage") {
                token = m_tokenizer.nextToken();
                expect(Quake3ShaderToken::String, token);
                return token.data();
            } else {
                while (!m_tokenizer.nextToken().hasType(Quake3ShaderToken::Eol));
            }

            return "";
        }

        Quake3ShaderParser::TokenNameMap Quake3ShaderParser::tokenNames() const {
            TokenNameMap result;
            result[Quake3ShaderToken::Number]   = "number";
            result[Quake3ShaderToken::String]   = "string";
            result[Quake3ShaderToken::Variable] = "variable";
            result[Quake3ShaderToken::OBrace]   = "'{'";
            result[Quake3ShaderToken::CBrace]   = "'}'";
            result[Quake3ShaderToken::Comment]  = "comment";
            result[Quake3ShaderToken::Eol]      = "end of line";
            result[Quake3ShaderToken::Eof]      = "end of file";
            return result;
        }
    }
}