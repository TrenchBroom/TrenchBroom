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

#include "ELParser.h"

namespace TrenchBroom {
    namespace IO {
        ELTokenizer::ELTokenizer(const char* begin, const char* end) :
        Tokenizer(begin, end) {}
        
        ELTokenizer::ELTokenizer(const String& str) :
        Tokenizer(str) {}
        
        ELTokenizer::Token ELTokenizer::emitToken() {
            while (!eof()) {
                size_t startLine = line();
                size_t startColumn = column();
                const char* c = curPos();
                switch (*c) {
                    case '/':
                        advance();
                        if (curChar() == '/')
                            discardUntil("\n\r");
                        return Token(ELToken::Over, c, c+1, offset(c), startLine, startColumn);
                    case '[':
                        advance();
                        return Token(ELToken::OBracket, c, c+1, offset(c), startLine, startColumn);
                    case ']':
                        advance();
                        return Token(ELToken::CBracket, c, c+1, offset(c), startLine, startColumn);
                    case '{':
                        advance();
                        return Token(ELToken::OBrace, c, c+1, offset(c), startLine, startColumn);
                    case '}':
                        advance();
                        return Token(ELToken::CBrace, c, c+1, offset(c), startLine, startColumn);
                    case '(':
                        advance();
                        return Token(ELToken::OParen, c, c+1, offset(c), startLine, startColumn);
                    case ')':
                        advance();
                        return Token(ELToken::CParen, c, c+1, offset(c), startLine, startColumn);
                    case '+':
                        advance();
                        return Token(ELToken::Plus, c, c+1, offset(c), startLine, startColumn);
                    case '-':
                        advance();
                        return Token(ELToken::Minus, c, c+1, offset(c), startLine, startColumn);
                    case '*':
                        advance();
                        return Token(ELToken::Times, c, c+1, offset(c), startLine, startColumn);
                    case ':':
                        advance();
                        return Token(ELToken::Colon, c, c+1, offset(c), startLine, startColumn);
                    case '"': {
                        advance();
                        c = curPos();
                        const char* e = readQuotedString();
                        return Token(ELToken::String, c, e, offset(c), startLine, startColumn);
                    }
                    case ' ':
                    case '\t':
                    case '\n':
                    case '\r':
                        discardWhile(Whitespace());
                        break;
                    default: {
                        const char* e = readInteger(Whitespace());
                        if (e != NULL)
                            return Token(ELToken::Integer, c, e, offset(c), startLine, startColumn);
                        
                        e = readDecimal(Whitespace());
                        if (e != NULL)
                            return Token(ELToken::Decimal, c, e, offset(c), startLine, startColumn);

                        e = readString(Whitespace() + "=");
                        if (e == NULL)
                            throw ParserException(startLine, startColumn, "Unexpected character: " + String(c, 1));
                        return Token(ELToken::Variable, c, e, offset(c), startLine, startColumn);
                    }
                }
            }
            return Token(ELToken::Eof, NULL, NULL, length(), line(), column());
        }
    }
}
