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

#include "base/ParserException.h"
#include "base/Token.h"
#include "base/Tokenizer.h"

#include "kd/contracts.h"

#include "vm/approx.h"

#include <string>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

namespace tb
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
      const auto startLine = line();
      const auto startColumn = column();
      const auto* c = curPos();
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
        contract_assert(e != nullptr);

        return {SimpleToken::String, c, e, offset(c), startLine, startColumn};
      }
      }
    }
    return {SimpleToken::Eof, nullptr, nullptr, length(), line(), column()};
  }

public:
  explicit SimpleTokenizer(
    std::string_view str,
    const std::string_view escapableChars = "",
    const char escapeChar = 0)
    : Tokenizer<SimpleToken::Type>{tokenNames(), str, escapableChars, escapeChar}
  {
  }

  // expose the protected tokenizer API for testing
  using Tokenizer::advance;
  using Tokenizer::curChar;
  using Tokenizer::escaped;
  using Tokenizer::lookAhead;
  using Tokenizer::resetEscaped;
  using Tokenizer::restore;
  using Tokenizer::snapshot;
  using Tokenizer::unescape;
};

} // namespace

TEST_CASE("TokenizerBase")
{
  SECTION("advance")
  {
    SECTION("counts columns")
    {
      auto tokenizer = SimpleTokenizer{"abc"};
      REQUIRE(tokenizer.location() == FileLocation{1, 1});

      tokenizer.advance();
      CHECK(tokenizer.location() == FileLocation{1, 2});

      tokenizer.advance();
      CHECK(tokenizer.location() == FileLocation{1, 3});
    }

    SECTION("counts lines separated by a line feed")
    {
      auto tokenizer = SimpleTokenizer{"a\nb"};

      tokenizer.advance();
      tokenizer.advance();
      CHECK(tokenizer.location() == FileLocation{2, 1});
    }

    SECTION("counts lines separated by a carriage return and a line feed")
    {
      auto tokenizer = SimpleTokenizer{"a\r\nb"};

      tokenizer.advance();
      REQUIRE(tokenizer.location() == FileLocation{1, 2});

      // the carriage return only advances the column
      tokenizer.advance();
      CHECK(tokenizer.location() == FileLocation{1, 3});

      tokenizer.advance();
      CHECK(tokenizer.location() == FileLocation{2, 1});
    }

    SECTION("counts lines separated by a lone carriage return")
    {
      auto tokenizer = SimpleTokenizer{"a\rb"};

      tokenizer.advance();
      tokenizer.advance();
      CHECK(tokenizer.location() == FileLocation{2, 1});
    }

    SECTION("advances by the given number of characters")
    {
      auto tokenizer = SimpleTokenizer{"abcde"};

      tokenizer.advance(3u);
      CHECK(tokenizer.location() == FileLocation{1, 4});
      CHECK(tokenizer.curChar() == 'd');
    }

    SECTION("throws at the end of the input")
    {
      auto tokenizer = SimpleTokenizer{"a"};

      tokenizer.advance();
      REQUIRE(tokenizer.eof());

      CHECK_THROWS_MATCHES(
        tokenizer.advance(),
        ParserException,
        Catch::Matchers::Message("Unexpected end of file"));
    }
  }

  SECTION("lookAhead")
  {
    auto tokenizer = SimpleTokenizer{"abc"};

    CHECK(tokenizer.lookAhead(0u) == 'a');
    CHECK(tokenizer.lookAhead() == 'b');
    CHECK(tokenizer.lookAhead(2u) == 'c');

    // returns 0 past the end of the input
    CHECK(tokenizer.lookAhead(3u) == 0);
    CHECK(tokenizer.lookAhead(4u) == 0);
  }

  SECTION("escaped")
  {
    auto tokenizer = SimpleTokenizer{R"(a\;\ab)", ";", '\\'};

    // 'a' is not escaped
    CHECK(!tokenizer.escaped());

    // the escape character itself is not escaped
    tokenizer.advance();
    CHECK(!tokenizer.escaped());

    // ';' is escapable and preceded by the escape character
    tokenizer.advance();
    CHECK(tokenizer.escaped());

    SECTION("resetEscaped")
    {
      tokenizer.resetEscaped();
      CHECK(!tokenizer.escaped());
    }

    SECTION("a character that is not escapable is never escaped")
    {
      tokenizer.advance();
      tokenizer.advance();
      CHECK(!tokenizer.escaped());
    }
  }

  SECTION("unescape")
  {
    const auto tokenizer = SimpleTokenizer{"", ";", '\\'};

    CHECK(tokenizer.unescape("") == "");
    CHECK(tokenizer.unescape("asdf") == "asdf");
    CHECK(tokenizer.unescape(R"(a\;b)") == "a;b");

    // only escapable characters are unescaped
    CHECK(tokenizer.unescape(R"(a\xb)") == R"(a\xb)");
  }

  SECTION("snapshot / restore")
  {
    auto tokenizer = SimpleTokenizer{"abc"};
    tokenizer.advance();

    const auto snapshot = tokenizer.snapshot();
    REQUIRE(tokenizer.location() == FileLocation{1, 2});

    tokenizer.advance();
    REQUIRE(tokenizer.location() == FileLocation{1, 3});

    tokenizer.restore(snapshot);
    CHECK(tokenizer.location() == FileLocation{1, 2});
    CHECK(tokenizer.curChar() == 'b');
  }

  SECTION("adoptState")
  {
    auto tokenizer = SimpleTokenizer{"abc"};
    tokenizer.advance();
    const auto snapshot = tokenizer.snapshot();

    tokenizer.advance();
    REQUIRE(tokenizer.location() == FileLocation{1, 3});

    tokenizer.adoptState(snapshot);
    CHECK(tokenizer.location() == FileLocation{1, 2});
    CHECK(tokenizer.curChar() == 'b');
  }

  SECTION("reset")
  {
    auto tokenizer = SimpleTokenizer{"abc"};
    tokenizer.advance();
    tokenizer.advance();
    REQUIRE(tokenizer.location() == FileLocation{1, 3});

    tokenizer.reset();
    CHECK(tokenizer.location() == FileLocation{1, 1});
    CHECK(tokenizer.curChar() == 'a');
  }

  SECTION("replaceState")
  {
    const auto replacement = std::string{"xy"};

    auto tokenizer = SimpleTokenizer{"abc"};
    tokenizer.advance();

    tokenizer.replaceState(replacement);

    CHECK(tokenizer.location() == FileLocation{1, 1});
    CHECK(tokenizer.nextToken().data() == "xy");
  }

  SECTION("snapshotStateAndSource / restoreStateAndSource")
  {
    const auto replacement = std::string{"xyz"};

    auto tokenizer = SimpleTokenizer{"abc"};
    tokenizer.advance();

    const auto snapshot = tokenizer.snapshotStateAndSource();

    tokenizer.replaceState(replacement);
    REQUIRE(tokenizer.location() == FileLocation{1, 1});
    REQUIRE(tokenizer.curChar() == 'x');

    tokenizer.restoreStateAndSource(snapshot);
    CHECK(tokenizer.location() == FileLocation{1, 2});
    CHECK(tokenizer.curChar() == 'b');
  }
}

TEST_CASE("Tokenizer")
{
  SECTION("tokenName")
  {
    const auto tokenizer = SimpleTokenizer{""};

    SECTION("returns a placeholder if no token type matches")
    {
      CHECK(tokenizer.tokenName(0) == "unknown token type");
      CHECK(tokenizer.tokenName(1 << 20) == "unknown token type");
    }

    SECTION("returns the name of the matching token type")
    {
      CHECK(tokenizer.tokenName(SimpleToken::Integer) == "integer");
      CHECK(tokenizer.tokenName(SimpleToken::OBrace) == "'{'");
      CHECK(tokenizer.tokenName(SimpleToken::Eof) == "end of file");
    }

    SECTION("joins the names of all matching token types in token type order")
    {
      CHECK(
        tokenizer.tokenName(SimpleToken::Integer | SimpleToken::Decimal)
        == "integer or decimal");
      CHECK(
        tokenizer.tokenName(SimpleToken::Decimal | SimpleToken::Integer)
        == "integer or decimal");
      CHECK(
        tokenizer.tokenName(
          SimpleToken::Integer | SimpleToken::Decimal | SimpleToken::String)
        == "integer, decimal, or string");
      CHECK(
        tokenizer.tokenName(SimpleToken::CBrace | SimpleToken::OBrace) == "'{' or '}'");
    }
  }

  SECTION("expectString")
  {
    auto tokenizer = SimpleTokenizer{"123"};
    const auto token = tokenizer.peekToken();

    CHECK(
      tokenizer.expectString(token, "a number")
      == "Expected a number, but got integer (raw data: '123')");
  }

  SECTION("expect")
  {
    auto tokenizer = SimpleTokenizer{"123"};

    SECTION("returns the token if it has one of the expected types")
    {
      CHECK(
        tokenizer.nextToken(SimpleToken::Integer | SimpleToken::Decimal).type()
        == SimpleToken::Integer);
    }

    SECTION("throws if the token does not have one of the expected types")
    {
      CHECK_THROWS_MATCHES(
        tokenizer.nextToken(SimpleToken::OBrace),
        ParserException,
        Catch::Matchers::Message(
          "At line 1, column 1: Expected '{', but got integer (raw data: '123')"));
    }
  }

  SECTION("Simple language")
  {
    SECTION("empty string")
    {
      auto tokenizer = SimpleTokenizer{""};
      CHECK(tokenizer.nextToken().type() == SimpleToken::Eof);
    }

    SECTION("blank string")
    {
      auto tokenizer = SimpleTokenizer{"\n  \t "};
      CHECK(tokenizer.nextToken().type() == SimpleToken::Eof);
    }

    SECTION("empty block")
    {
      auto tokenizer = SimpleTokenizer{R"({})"};
      CHECK(tokenizer.nextToken().type() == SimpleToken::OBrace);
      CHECK(tokenizer.nextToken().type() == SimpleToken::CBrace);
      CHECK(tokenizer.nextToken().type() == SimpleToken::Eof);
    }

    SECTION("push peek pop token")
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

    SECTION("empty block with leading and trailing whitespace")
    {
      auto tokenizer = SimpleTokenizer{R"( 	{
 }  )"};

      CHECK(tokenizer.nextToken().type() == SimpleToken::OBrace);
      CHECK(tokenizer.nextToken().type() == SimpleToken::CBrace);
      CHECK(tokenizer.nextToken().type() == SimpleToken::Eof);
    }

    SECTION("block with string attribute")
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

    SECTION("block with integer attribute")
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

    SECTION("block with negative integer attribute")
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

    SECTION("block with decimal attribute")
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

    SECTION("block with decimal attribute starting with dot")
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

    SECTION("block with negative decimal attribute")
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

    SECTION("numbers")
    {
      // the numbers are not followed by a delimiter, so these also exercise reading a
      // number that extends to the end of the input
      // clang-format off
      const auto
      [str,       expectedType        ] = GENERATE(table<std::string_view, SimpleToken::Type>({
      {"123",     SimpleToken::Integer},
      {"+123",    SimpleToken::Integer},
      {"-123",    SimpleToken::Integer},
      {"1.5",     SimpleToken::Decimal},
      {"+1.5",    SimpleToken::Decimal},
      {"-1.5",    SimpleToken::Decimal},
      {".5",      SimpleToken::Decimal},
      {"1e10",    SimpleToken::Decimal},
      {"1E10",    SimpleToken::Decimal},
      {"1.5e3",   SimpleToken::Decimal},
      {"1.5e+3",  SimpleToken::Decimal},
      {"1.5e-3",  SimpleToken::Decimal},
      {"1.5E-3",  SimpleToken::Decimal},
      {"-1.5e-3", SimpleToken::Decimal},
      {".5e2",    SimpleToken::Decimal},
      }));
      // clang-format on

      CAPTURE(str);

      auto tokenizer = SimpleTokenizer{str};

      auto token = tokenizer.nextToken();
      CHECK(token.type() == expectedType);
      CHECK(token.data() == str);
      CHECK(tokenizer.nextToken().type() == SimpleToken::Eof);
    }

    SECTION("number values")
    {
      // clang-format off
      const auto
      [str,       expectedValue] = GENERATE(table<std::string_view, double>({
      {"123",     123.0        },
      {"+123",    123.0        },
      {"-123",    -123.0       },
      {"1.5",     1.5          },
      {"+1.5",    1.5          },
      {"-1.5",    -1.5         },
      {".5",      0.5          },
      {"+.5",     0.5          },
      {"1e10",    1e10         },
      {"1E10",    1e10         },
      {"1.5e3",   1500.0       },
      {"1.5e+3",  1500.0       },
      {"1.5e-3",  0.0015       },
      {"1.5E-3",  0.0015       },
      {"-1.5e-3", -0.0015      },
      {".5e2",    50.0         },
      }));
      // clang-format on

      CAPTURE(str);

      auto tokenizer = SimpleTokenizer{str};

      CHECK(tokenizer.nextToken().toFloat<double>() == vm::approx(expectedValue));
    }

    SECTION("numbers followed by delimiter")
    {
      auto tokenizer = SimpleTokenizer{"1.5e-3;"};

      SimpleTokenizer::Token token;
      CHECK((token = tokenizer.nextToken()).type() == SimpleToken::Decimal);
      CHECK(token.data() == "1.5e-3");
      CHECK(token.toFloat<double>() == vm::approx(0.0015));
      CHECK(tokenizer.nextToken().type() == SimpleToken::Semicolon);
      CHECK(tokenizer.nextToken().type() == SimpleToken::Eof);
    }

    SECTION("malformed numbers")
    {
      // a number that is not followed by a delimiter is not a number at all, and the
      // tokenizer must roll back and read it as a string
      // clang-format off
      const auto
      [str,      expectedType        ] = GENERATE(table<std::string_view, SimpleToken::Type>({
      {"123a",   SimpleToken::String },
      {"1.5a",   SimpleToken::String },
      {"1.5e3a", SimpleToken::String },
      {"1ea",    SimpleToken::String },
      {"1e+a",   SimpleToken::String },
      {"+a",     SimpleToken::String },
      {"-a",     SimpleToken::String },
      {".a",     SimpleToken::String },
      }));
      // clang-format on

      CAPTURE(str);

      auto tokenizer = SimpleTokenizer{str};

      auto token = tokenizer.nextToken();
      CHECK(token.type() == expectedType);
      CHECK(token.data() == str);
      CHECK(tokenizer.nextToken().type() == SimpleToken::Eof);
    }
  }
}

} // namespace tb
