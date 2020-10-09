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

#include "IO/Token.h"
#include "IO/Tokenizer.h"

#include <string>

#include "Catch2.h"
#include "GTestCompat.h"

namespace TrenchBroom {
    namespace IO {
        namespace SimpleToken {
            using Type = unsigned int;
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
            using Token = Tokenizer<SimpleToken::Type>::Token;
        private:
            Token emitToken() override {
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
                            if (e != nullptr)
                                return Token(SimpleToken::Integer, c, e, offset(c), startLine, startColumn);
                            e = readDecimal("{};= \n\r\t");
                            if (e != nullptr)
                                return Token(SimpleToken::Decimal, c, e, offset(c), startLine, startColumn);
                            e = readUntil("{};= \n\r\t");
                            assert(e != nullptr);
                            return Token(SimpleToken::String, c, e, offset(c), startLine, startColumn);
                        }
                    }
                }
                return Token(SimpleToken::Eof, nullptr, nullptr, length(), line(), column());
            }
        public:
            SimpleTokenizer(const std::string& str) :
            Tokenizer<SimpleToken::Type>(str, "", 0) {}
        };

        TEST_CASE("TokenizerTest.simpleLanguageEmptyString", "[TokenizerTest]") {
            const std::string testString("");
            SimpleTokenizer tokenizer(testString);
            ASSERT_EQ(SimpleToken::Eof, tokenizer.nextToken().type());
        }

        TEST_CASE("TokenizerTest.simpleLanguageBlankString", "[TokenizerTest]") {
            const std::string testString("\n  \t ");
            SimpleTokenizer tokenizer(testString);
            ASSERT_EQ(SimpleToken::Eof, tokenizer.nextToken().type());
        }

        TEST_CASE("TokenizerTest.simpleLanguageEmptyBlock", "[TokenizerTest]") {
            const std::string testString("{"
                                    "}");

            SimpleTokenizer tokenizer(testString);
            ASSERT_EQ(SimpleToken::OBrace, tokenizer.nextToken().type());
            ASSERT_EQ(SimpleToken::CBrace, tokenizer.nextToken().type());
            ASSERT_EQ(SimpleToken::Eof, tokenizer.nextToken().type());
        }

        TEST_CASE("TokenizerTest.simpleLanguagePushPeekPopToken", "[TokenizerTest]") {
            const std::string testString("{\n"
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

        TEST_CASE("TokenizerTest.simpleLanguageEmptyBlockWithLeadingAndTrailingWhitespace", "[TokenizerTest]") {
            const std::string testString(" \t{"
                                    " }  ");

            SimpleTokenizer tokenizer(testString);
            ASSERT_EQ(SimpleToken::OBrace, tokenizer.nextToken().type());
            ASSERT_EQ(SimpleToken::CBrace, tokenizer.nextToken().type());
            ASSERT_EQ(SimpleToken::Eof, tokenizer.nextToken().type());
        }

        TEST_CASE("TokenizerTest.simpleLanguageBlockWithStringAttribute", "[TokenizerTest]") {
            const std::string testString("{\n"
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

        TEST_CASE("TokenizerTest.simpleLanguageBlockWithIntegerAttribute", "[TokenizerTest]") {
            const std::string testString("{"
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

        TEST_CASE("TokenizerTest.simpleLanguageBlockWithNegativeIntegerAttribute", "[TokenizerTest]") {
            const std::string testString("{"
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

        TEST_CASE("TokenizerTest.simpleLanguageBlockWithDecimalAttribute", "[TokenizerTest]") {
            const std::string testString("{"
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

        TEST_CASE("TokenizerTest.simpleLanguageBlockWithDecimalAttributeStartingWithDot", "[TokenizerTest]") {
            const std::string testString("{"
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

        TEST_CASE("TokenizerTest.simpleLanguageBlockWithNegativeDecimalAttribute", "[TokenizerTest]") {
            const std::string testString("{"
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
