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

#include "IO/Tokenizer.h"
#include "IO/Token.h"

#include <string>

#include <vecmath/approx.h>

#include "Catch2.h"

namespace TrenchBroom
{
namespace IO
{
namespace SimpleToken
{
using Type = unsigned int;
static const Type Integer = 1 << 0;   // integer number
static const Type Decimal = 1 << 1;   // decimal number
static const Type String = 1 << 2;    // string
static const Type OBrace = 1 << 3;    // opening brace: {
static const Type CBrace = 1 << 4;    // closing brace: }
static const Type Equals = 1 << 5;    // equals sign: =
static const Type Semicolon = 1 << 6; // semicolon: ;
static const Type Eof = 1 << 7;       // end of file
} // namespace SimpleToken

class SimpleTokenizer : public Tokenizer<SimpleToken::Type>
{
public:
  using Token = Tokenizer<SimpleToken::Type>::Token;

private:
  Token emitToken() override
  {
    while (!eof())
    {
      size_t startLine = line();
      size_t startColumn = column();
      const char* c = curPos();
      switch (*c)
      {
      case '{':
        advance();
        return Token(SimpleToken::OBrace, c, c + 1, offset(c), startLine, startColumn);
      case '}':
        advance();
        return Token(SimpleToken::CBrace, c, c + 1, offset(c), startLine, startColumn);
      case '=':
        advance();
        return Token(SimpleToken::Equals, c, c + 1, offset(c), startLine, startColumn);
      case ';':
        advance();
        return Token(SimpleToken::Semicolon, c, c + 1, offset(c), startLine, startColumn);
      default: { // integer, decimal, or string
        if (isWhitespace(*c))
        {
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
  SimpleTokenizer(std::string_view str)
    : Tokenizer<SimpleToken::Type>(std::move(str), "", 0)
  {
  }
};

TEST_CASE("TokenizerTest.simpleLanguageEmptyString")
{
  const std::string testString("");
  SimpleTokenizer tokenizer(testString);
  CHECK(tokenizer.nextToken().type() == SimpleToken::Eof);
}

TEST_CASE("TokenizerTest.simpleLanguageBlankString")
{
  const std::string testString("\n  \t ");
  SimpleTokenizer tokenizer(testString);
  CHECK(tokenizer.nextToken().type() == SimpleToken::Eof);
}

TEST_CASE("TokenizerTest.simpleLanguageEmptyBlock")
{
  const std::string testString(
    "{"
    "}");

  SimpleTokenizer tokenizer(testString);
  CHECK(tokenizer.nextToken().type() == SimpleToken::OBrace);
  CHECK(tokenizer.nextToken().type() == SimpleToken::CBrace);
  CHECK(tokenizer.nextToken().type() == SimpleToken::Eof);
}

TEST_CASE("TokenizerTest.simpleLanguagePushPeekPopToken")
{
  const std::string testString(
    "{\n"
    "}");

  SimpleTokenizer tokenizer(testString);
  SimpleTokenizer::Token token;
  CHECK((token = tokenizer.peekToken()).type() == SimpleToken::OBrace);
  CHECK(token.line() == 1u);
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::OBrace);
  CHECK(token.line() == 1u);
  CHECK(tokenizer.nextToken().type() == SimpleToken::CBrace);
  CHECK(tokenizer.nextToken().type() == SimpleToken::Eof);
}

TEST_CASE("TokenizerTest.simpleLanguageEmptyBlockWithLeadingAndTrailingWhitespace")
{
  const std::string testString(
    " \t{"
    " }  ");

  SimpleTokenizer tokenizer(testString);
  CHECK(tokenizer.nextToken().type() == SimpleToken::OBrace);
  CHECK(tokenizer.nextToken().type() == SimpleToken::CBrace);
  CHECK(tokenizer.nextToken().type() == SimpleToken::Eof);
}

TEST_CASE("TokenizerTest.simpleLanguageBlockWithStringAttribute")
{
  const std::string testString(
    "{\n"
    "    attribute =value;\n"
    "}\n");

  SimpleTokenizer tokenizer(testString);
  SimpleTokenizer::Token token;
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::OBrace);
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::String);
  CHECK(token.data() == "attribute");
  CHECK(token.line() == 2u);
  CHECK(token.column() == 5u);
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::Equals);
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::String);
  CHECK(token.data() == "value");
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::Semicolon);
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::CBrace);
  CHECK(tokenizer.nextToken().type() == SimpleToken::Eof);
}

TEST_CASE("TokenizerTest.simpleLanguageBlockWithIntegerAttribute")
{
  const std::string testString(
    "{"
    "    attribute =  12328;"
    "}");

  SimpleTokenizer tokenizer(testString);
  SimpleTokenizer::Token token;
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::OBrace);
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::String);
  CHECK(token.data() == "attribute");
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::Equals);
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::Integer);
  CHECK(token.toInteger<int>() == 12328);
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::Semicolon);
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::CBrace);
  CHECK(tokenizer.nextToken().type() == SimpleToken::Eof);
}

TEST_CASE("TokenizerTest.simpleLanguageBlockWithNegativeIntegerAttribute")
{
  const std::string testString(
    "{"
    "    attribute =  -12328;"
    "}");

  SimpleTokenizer tokenizer(testString);
  SimpleTokenizer::Token token;
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::OBrace);
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::String);
  CHECK(token.data() == "attribute");
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::Equals);
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::Integer);
  CHECK(token.toInteger<int>() == -12328);
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::Semicolon);
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::CBrace);
  CHECK(tokenizer.nextToken().type() == SimpleToken::Eof);
}

TEST_CASE("TokenizerTest.simpleLanguageBlockWithDecimalAttribute")
{
  const std::string testString(
    "{"
    "    attribute =  12328.38283;"
    "}");

  SimpleTokenizer tokenizer(testString);
  SimpleTokenizer::Token token;
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::OBrace);
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::String);
  CHECK(token.data() == "attribute");
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::Equals);
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::Decimal);
  CHECK(token.toFloat<double>() == vm::approx(12328.38283));
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::Semicolon);
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::CBrace);
  CHECK(tokenizer.nextToken().type() == SimpleToken::Eof);
}

TEST_CASE("TokenizerTest.simpleLanguageBlockWithDecimalAttributeStartingWithDot")
{
  const std::string testString(
    "{"
    "    attribute =  .38283;"
    "}");

  SimpleTokenizer tokenizer(testString);
  SimpleTokenizer::Token token;
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::OBrace);
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::String);
  CHECK(token.data() == "attribute");
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::Equals);
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::Decimal);
  CHECK(token.toFloat<double>() == vm::approx(0.38283));
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::Semicolon);
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::CBrace);
  CHECK(tokenizer.nextToken().type() == SimpleToken::Eof);
}

TEST_CASE("TokenizerTest.simpleLanguageBlockWithNegativeDecimalAttribute")
{
  const std::string testString(
    "{"
    "    attribute =  -343.38283;"
    "}");

  SimpleTokenizer tokenizer(testString);
  SimpleTokenizer::Token token;
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::OBrace);
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::String);
  CHECK(token.data() == "attribute");
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::Equals);
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::Decimal);
  CHECK(token.toFloat<double>() == vm::approx(-343.38283));
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::Semicolon);
  CHECK((token = tokenizer.nextToken()).type() == SimpleToken::CBrace);
  CHECK(tokenizer.nextToken().type() == SimpleToken::Eof);
}
} // namespace IO
} // namespace TrenchBroom
