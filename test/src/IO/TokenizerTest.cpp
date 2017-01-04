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

#include "IO/Token.h"
#include "IO/Tokenizer.h"

#include <gtest/gtest.h>

namespace TrenchBroom {
    namespace IO {
        namespace SimpleToken {
            typedef unsigned int Type;
            static const Type Integer       = 1 <<  0; // integer number
            static const Type Decimal       = 1 <<  1; // decimal number
            static const Type String        = 1 <<  2; // string
            static const Type OBrace        = 1 <<  3; // opening brace: {
            static const Type CBrace        = 1 <<  4; // closing brace: }
            static const Type Equals        = 1 <<  5; // equals sign: =
            static const Type Semicolon     = 1 <<  6; // semicolon: ;
            static const Type Eof           = 1 <<  7; // end of file
        }
        
        
        class SimpleTokenizer : public Tokenizer<SimpleToken::Type> {
        public:
            typedef Tokenizer<SimpleToken::Type>::Token Token;
        private:
            Token emitToken() {
                while (!eof()) {
                    size_t startLine = line();
                    size_t startColumn = column();
                    const char* c = curPos();
                    switch (*c) {
                        case '{':
                            advance();
                            return Token(SimpleToken::OBrace, c, c+1, offset(c), startLine, startColumn);
                        case '}':
                            advance();
                            return Token(SimpleToken::CBrace, c, c+1, offset(c), startLine, startColumn);
                        case '=':
                            advance();
                            return Token(SimpleToken::Equals, c, c+1, offset(c), startLine, startColumn);
                        case ';':
                            advance();
                            return Token(SimpleToken::Semicolon, c, c+1, offset(c), startLine, startColumn);
                        default: { // integer, decimal, or string
                            if (isWhitespace(*c)) {
                                advance();
                                break;
                            }
                            const char* e = readInteger("{};= \n\r\t");
                            if (e != NULL)
                                return Token(SimpleToken::Integer, c, e, offset(c), startLine, startColumn);
                            e = readDecimal("{};= \n\r\t");
                            if (e != NULL)
                                return Token(SimpleToken::Decimal, c, e, offset(c), startLine, startColumn);
                            e = readUntil("{};= \n\r\t");
                            assert(e != NULL);
                            return Token(SimpleToken::String, c, e, offset(c), startLine, startColumn);
                        }
                    }
                }
                return Token(SimpleToken::Eof, NULL, NULL, length(), line(), column());
            }
        public:
            SimpleTokenizer(const String& str) :
            Tokenizer<SimpleToken::Type>(str, "", 0) {}
        };
        
        TEST(TokenizerTest, simpleLanguageEmptyString) {
            const String testString("");
            SimpleTokenizer tokenizer(testString);
            ASSERT_EQ(SimpleToken::Eof, tokenizer.nextToken().type());
        }
        
        TEST(TokenizerTest, simpleLanguageBlankString) {
            const String testString("\n  \t ");
            SimpleTokenizer tokenizer(testString);
            ASSERT_EQ(SimpleToken::Eof, tokenizer.nextToken().type());
        }
        
        TEST(TokenizerTest, simpleLanguageEmptyBlock) {
            const String testString("{"
                                    "}");
            
            SimpleTokenizer tokenizer(testString);
            ASSERT_EQ(SimpleToken::OBrace, tokenizer.nextToken().type());
            ASSERT_EQ(SimpleToken::CBrace, tokenizer.nextToken().type());
            ASSERT_EQ(SimpleToken::Eof, tokenizer.nextToken().type());
        }
        
        TEST(TokenizerTest, simpleLanguagePushPeekPopToken) {
            const String testString("{\n"
                                    "}");
            
            SimpleTokenizer tokenizer(testString);
            SimpleTokenizer::Token token;
            ASSERT_EQ(SimpleToken::OBrace, (token = tokenizer.peekToken()).type());
            ASSERT_EQ(1u, token.line());
            ASSERT_EQ(SimpleToken::OBrace, (token = tokenizer.nextToken()).type());
            ASSERT_EQ(1u, token.line());
            ASSERT_EQ(SimpleToken::CBrace, tokenizer.nextToken().type());
            ASSERT_EQ(SimpleToken::Eof, tokenizer.nextToken().type());
        }

        TEST(TokenizerTest, simpleLanguageEmptyBlockWithLeadingAndTrailingWhitespace) {
            const String testString(" \t{"
                                    " }  ");
            
            SimpleTokenizer tokenizer(testString);
            ASSERT_EQ(SimpleToken::OBrace, tokenizer.nextToken().type());
            ASSERT_EQ(SimpleToken::CBrace, tokenizer.nextToken().type());
            ASSERT_EQ(SimpleToken::Eof, tokenizer.nextToken().type());
        }
        
        TEST(TokenizerTest, simpleLanguageBlockWithStringAttribute) {
            const String testString("{\n"
                                    "    attribute =value;\n"
                                    "}\n");
            
            SimpleTokenizer tokenizer(testString);
            SimpleTokenizer::Token token;
            ASSERT_EQ(SimpleToken::OBrace, (token = tokenizer.nextToken()).type());
            ASSERT_EQ(SimpleToken::String, (token = tokenizer.nextToken()).type());
            ASSERT_STREQ("attribute", token.data().c_str());
            ASSERT_EQ(2u, token.line());
            ASSERT_EQ(5u, token.column());
            ASSERT_EQ(SimpleToken::Equals, (token = tokenizer.nextToken()).type());
            ASSERT_EQ(SimpleToken::String, (token = tokenizer.nextToken()).type());
            ASSERT_STREQ("value", token.data().c_str());
            ASSERT_EQ(SimpleToken::Semicolon, (token = tokenizer.nextToken()).type());
            ASSERT_EQ(SimpleToken::CBrace, (token = tokenizer.nextToken()).type());
            ASSERT_EQ(SimpleToken::Eof, tokenizer.nextToken().type());
        }

        TEST(TokenizerTest, simpleLanguageBlockWithIntegerAttribute) {
            const String testString("{"
                                    "    attribute =  12328;"
                                    "}");
            
            SimpleTokenizer tokenizer(testString);
            SimpleTokenizer::Token token;
            ASSERT_EQ(SimpleToken::OBrace, (token = tokenizer.nextToken()).type());
            ASSERT_EQ(SimpleToken::String, (token = tokenizer.nextToken()).type());
            ASSERT_STREQ("attribute", token.data().c_str());
            ASSERT_EQ(SimpleToken::Equals, (token = tokenizer.nextToken()).type());
            ASSERT_EQ(SimpleToken::Integer, (token = tokenizer.nextToken()).type());
            ASSERT_EQ(12328, token.toInteger<int>());
            ASSERT_EQ(SimpleToken::Semicolon, (token = tokenizer.nextToken()).type());
            ASSERT_EQ(SimpleToken::CBrace, (token = tokenizer.nextToken()).type());
            ASSERT_EQ(SimpleToken::Eof, tokenizer.nextToken().type());
        }
        
        TEST(TokenizerTest, simpleLanguageBlockWithNegativeIntegerAttribute) {
            const String testString("{"
                                    "    attribute =  -12328;"
                                    "}");
            
            SimpleTokenizer tokenizer(testString);
            SimpleTokenizer::Token token;
            ASSERT_EQ(SimpleToken::OBrace, (token = tokenizer.nextToken()).type());
            ASSERT_EQ(SimpleToken::String, (token = tokenizer.nextToken()).type());
            ASSERT_STREQ("attribute", token.data().c_str());
            ASSERT_EQ(SimpleToken::Equals, (token = tokenizer.nextToken()).type());
            ASSERT_EQ(SimpleToken::Integer, (token = tokenizer.nextToken()).type());
            ASSERT_EQ(-12328, token.toInteger<int>());
            ASSERT_EQ(SimpleToken::Semicolon, (token = tokenizer.nextToken()).type());
            ASSERT_EQ(SimpleToken::CBrace, (token = tokenizer.nextToken()).type());
            ASSERT_EQ(SimpleToken::Eof, tokenizer.nextToken().type());
        }

        TEST(TokenizerTest, simpleLanguageBlockWithDecimalAttribute) {
            const String testString("{"
                                    "    attribute =  12328.38283;"
                                    "}");
            
            SimpleTokenizer tokenizer(testString);
            SimpleTokenizer::Token token;
            ASSERT_EQ(SimpleToken::OBrace, (token = tokenizer.nextToken()).type());
            ASSERT_EQ(SimpleToken::String, (token = tokenizer.nextToken()).type());
            ASSERT_STREQ("attribute", token.data().c_str());
            ASSERT_EQ(SimpleToken::Equals, (token = tokenizer.nextToken()).type());
            ASSERT_EQ(SimpleToken::Decimal, (token = tokenizer.nextToken()).type());
            ASSERT_DOUBLE_EQ(12328.38283, token.toFloat<double>());
            ASSERT_EQ(SimpleToken::Semicolon, (token = tokenizer.nextToken()).type());
            ASSERT_EQ(SimpleToken::CBrace, (token = tokenizer.nextToken()).type());
            ASSERT_EQ(SimpleToken::Eof, tokenizer.nextToken().type());
        }
        
        TEST(TokenizerTest, simpleLanguageBlockWithDecimalAttributeStartingWithDot) {
            const String testString("{"
                                    "    attribute =  .38283;"
                                    "}");
            
            SimpleTokenizer tokenizer(testString);
            SimpleTokenizer::Token token;
            ASSERT_EQ(SimpleToken::OBrace, (token = tokenizer.nextToken()).type());
            ASSERT_EQ(SimpleToken::String, (token = tokenizer.nextToken()).type());
            ASSERT_STREQ("attribute", token.data().c_str());
            ASSERT_EQ(SimpleToken::Equals, (token = tokenizer.nextToken()).type());
            ASSERT_EQ(SimpleToken::Decimal, (token = tokenizer.nextToken()).type());
            ASSERT_DOUBLE_EQ(0.38283, token.toFloat<double>());
            ASSERT_EQ(SimpleToken::Semicolon, (token = tokenizer.nextToken()).type());
            ASSERT_EQ(SimpleToken::CBrace, (token = tokenizer.nextToken()).type());
            ASSERT_EQ(SimpleToken::Eof, tokenizer.nextToken().type());
        }
        
        TEST(TokenizerTest, simpleLanguageBlockWithNegativeDecimalAttribute) {
            const String testString("{"
                                    "    attribute =  -343.38283;"
                                    "}");
            
            SimpleTokenizer tokenizer(testString);
            SimpleTokenizer::Token token;
            ASSERT_EQ(SimpleToken::OBrace, (token = tokenizer.nextToken()).type());
            ASSERT_EQ(SimpleToken::String, (token = tokenizer.nextToken()).type());
            ASSERT_STREQ("attribute", token.data().c_str());
            ASSERT_EQ(SimpleToken::Equals, (token = tokenizer.nextToken()).type());
            ASSERT_EQ(SimpleToken::Decimal, (token = tokenizer.nextToken()).type());
            ASSERT_DOUBLE_EQ(-343.38283, token.toFloat<double>());
            ASSERT_EQ(SimpleToken::Semicolon, (token = tokenizer.nextToken()).type());
            ASSERT_EQ(SimpleToken::CBrace, (token = tokenizer.nextToken()).type());
            ASSERT_EQ(SimpleToken::Eof, tokenizer.nextToken().type());
        }
    }
}
