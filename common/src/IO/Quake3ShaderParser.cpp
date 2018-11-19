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
                size_t startLine = line();
                size_t startColumn = column();
                const char* c = curPos();
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
                    case '$':
                        // fall through into the default case to parse a variable name
                        switchFallthrough();
                    default:
                        e = readDecimal(Whitespace());
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
        }

        Quake3ShaderParser::Quake3ShaderParser(const char* begin, const char* end) :
        m_tokenizer(begin, end) {}

        Quake3ShaderParser::Quake3ShaderParser(const String& str) :
        m_tokenizer(str) {}
    }
}