/*
 Copyright (C) 2010 Kristian Duske

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

#include "io/Token.h"
#include "io/Tokenizer.h"

#include "vm/approx.h"

#include <string>

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::io
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

namespace
{
auto tokenNames()
{
  using namespace SimpleToken;

  return Tokenizer<Type>::TokenNameMap{
    {Integer, "integer"},
    {Decimal, "decimal"},
    {String, "string"},
    {OBrace, "'{'"},
    {CBrace, "'}'"},
    {Equals, "'='"},
    {Semicolon, "';'"},
    {Eof, "end of file"},
  };
}

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
        return {SimpleToken::OBrace, c, c + 1, offset(c), startLine, startColumn};
      case '}':
        advance();
        return {SimpleToken::CBrace, c, c + 1, offset(c), startLine, startColumn};
      case '=':
        advance();
        return {SimpleToken::Equals, c, c + 1, offset(c), startLine, startColumn};
      case ';':
        advance();
        return {SimpleToken::Semicolon, c, c + 1, offset(c), startLine, startColumn};
      default: { // integer, decimal, or string
        if (isWhitespace(*c))
        {
          advance();
          break;
        }
        if (const auto* e = readInteger("{};= \n\r\t"))
        {
          return {SimpleToken::Integer, c, e, offset(c), startLine, startColumn};
        }
        if (const auto* e = readDecimal("{};= \n\r\t"))
        {
          return {SimpleToken::Decimal, c, e, offset(c), startLine, startColumn};
        }
        const auto e = readUntil("{};= \n\r\t");
        assert(e != nullptr);
        return {SimpleToken::String, c, e, offset(c), startLine, startColumn};
      }
      }
    }
    return {SimpleToken::Eof, nullptr, nullptr, length(), line(), column()};
  }

public:
  explicit SimpleTokenizer(std::string_view str)
    : Tokenizer<SimpleToken::Type>{tokenNames(), std::move(str), "", 0}
  {
  }
};

} // namespace

TEST_CASE("TokenizerTest.simpleLanguageEmptyString")
{
  auto tokenizer = SimpleTokenizer{""};
  CHECK(tokenizer.nextToken().type() == SimpleToken::Eof);
}

TEST_CASE("TokenizerTest.simpleLanguageBlankString")
{
  auto tokenizer = SimpleTokenizer{"\n  \t "};
  CHECK(tokenizer.nextToken().type() == SimpleToken::Eof);
}

TEST_CASE("TokenizerTest.simpleLanguageEmptyBlock")
{
  auto tokenizer = SimpleTokenizer{R"({})"};
  CHECK(tokenizer.nextToken().type() == SimpleToken::OBrace);
  CHECK(tokenizer.nextToken().type() == SimpleToken::CBrace);
  CHECK(tokenizer.nextToken().type() == SimpleToken::Eof);
}

TEST_CASE("TokenizerTest.simpleLanguagePushPeekPopToken")
{
  auto tokenizer = SimpleTokenizer{R"({
})"};

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
  auto tokenizer = SimpleTokenizer{R"( 	{
 }  )"};

  CHECK(tokenizer.nextToken().type() == SimpleToken::OBrace);
  CHECK(tokenizer.nextToken().type() == SimpleToken::CBrace);
  CHECK(tokenizer.nextToken().type() == SimpleToken::Eof);
}

TEST_CASE("TokenizerTest.simpleLanguageBlockWithStringAttribute")
{
  auto tokenizer = SimpleTokenizer{R"({
    attribute =value;
}
)"};

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
  auto tokenizer = SimpleTokenizer{R"({
    attribute =  12328;
})"};

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
  auto tokenizer = SimpleTokenizer{R"({
    attribute =  -12328;
})"};

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
  auto tokenizer = SimpleTokenizer{R"({
    attribute =  12328.38283;
})"};

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
  auto tokenizer = SimpleTokenizer{R"({
    attribute =  .38283;
})"};
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
  auto tokenizer = SimpleTokenizer{R"({
    attribute =  -343.38283;
})"};

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

} // namespace tb::io
