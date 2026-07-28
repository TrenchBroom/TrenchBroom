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

#include "el/Parser.h"

#include "base/FileLocation.h"
#include "base/ParserException.h"
#include "base/Result.h"
#include "el/Exceptions.h"
#include "el/Expression.h"
#include "el/Value.h"

#include "kd/string_format.h"

#include <fmt/format.h>

#include <optional>
#include <string>

namespace tb::el
{
namespace
{
auto tokenNames()
{
  using namespace ElToken;

  return Tokenizer::TokenNameMap{
    {Name, "variable"},
    {String, "string"},
    {Number, "number"},
    {Boolean, "boolean"},
    {OBracket, "'['"},
    {CBracket, "']'"},
    {OBrace, "'{'"},
    {CBrace, "'}'"},
    {OParen, "'('"},
    {CParen, "')'"},
    {Addition, "'+'"},
    {Subtraction, "'-'"},
    {Multiplication, "'*'"},
    {Division, "'/'"},
    {Modulus, "'%'"},
    {Colon, "':'"},
    {Comma, "','"},
    {Range, "'..'"},
    {LogicalNegation, "'!'"},
    {LogicalAnd, "'&&'"},
    {LogicalOr, "'||'"},
    {Less, "'<'"},
    {LessOrEqual, "'<='"},
    {Equal, "'=='"},
    {NotEqual, "'!='"},
    {GreaterOrEqual, "'>='"},
    {Greater, "'>'"},
    {Case, "'->'"},
    {BitwiseNegation, "'~'"},
    {BitwiseAnd, "'&'"},
    {BitwiseOr, "'|'"},
    {BitwiseShiftLeft, "'<<'"},
    {BitwiseShiftRight, "'>>'"},
    {DoubleOBrace, "'{{'"},
    {DoubleCBrace, "'}}'"},
    {Null, "'null'"},
    {Eof, "end of file"},
  };
}
} // namespace

const std::string& Tokenizer::NumberDelim() const
{
  static const auto Delim = Whitespace() + "(){}[],:+-*/%<>=!&|^~";
  return Delim;
}

const std::string& Tokenizer::IntegerDelim() const
{
  static const auto Delim = NumberDelim() + ".";
  return Delim;
}

Tokenizer::Tokenizer(const std::string_view str, const size_t line, const size_t column)
  : tb::Tokenizer<ElToken::Type>{tokenNames(), str, "\"", '\\', line, column}
{
}

Tokenizer::Token Tokenizer::emitToken()
{
  while (!eof())
  {
    auto line = this->line();
    auto column = this->column();
    const auto* c = curPos();
    switch (*c)
    {
    case '[':
      advance();
      return Token{ElToken::OBracket, c, c + 1, offset(c), line, column};
    case ']':
      advance();
      return Token{ElToken::CBracket, c, c + 1, offset(c), line, column};
    case '{':
      advance();
      if (curChar() == '{')
      {
        advance();
        return Token{ElToken::DoubleOBrace, c, c + 2, offset(c), line, column};
      }
      return Token{ElToken::OBrace, c, c + 1, offset(c), line, column};
    case '}':
      advance();
      if (curChar() == '}')
      {
        advance();
        return Token{ElToken::DoubleCBrace, c, c + 2, offset(c), line, column};
      }
      return Token{ElToken::CBrace, c, c + 1, offset(c), line, column};
    case '(':
      advance();
      return Token{ElToken::OParen, c, c + 1, offset(c), line, column};
    case ')':
      advance();
      return Token{ElToken::CParen, c, c + 1, offset(c), line, column};
    case '+':
      advance();
      return Token{ElToken::Addition, c, c + 1, offset(c), line, column};
    case '-':
      advance();
      if (curChar() == '>')
      {
        advance();
        return Token{ElToken::Case, c, c + 2, offset(c), line, column};
      }
      return Token{ElToken::Subtraction, c, c + 1, offset(c), line, column};
    case '*':
      advance();
      return Token{ElToken::Multiplication, c, c + 1, offset(c), line, column};
    case '/':
      advance();
      if (curChar() == '/')
      {
        discardUntil("\n\r");
        break;
      }
      return Token{ElToken::Division, c, c + 1, offset(c), line, column};
    case '%':
      advance();
      return Token{ElToken::Modulus, c, c + 1, offset(c), line, column};
    case '~':
      advance();
      return Token{ElToken::BitwiseNegation, c, c + 1, offset(c), line, column};
    case '&':
      advance();
      if (curChar() == '&')
      {
        advance();
        return Token{ElToken::LogicalAnd, c, c + 2, offset(c), line, column};
      }
      return Token{ElToken::BitwiseAnd, c, c + 1, offset(c), line, column};
    case '|':
      advance();
      if (curChar() == '|')
      {
        advance();
        return Token{ElToken::LogicalOr, c, c + 2, offset(c), line, column};
      }
      return Token{ElToken::BitwiseOr, c, c + 1, offset(c), line, column};
    case '^':
      advance();
      return Token{ElToken::BitwiseXOr, c, c + 1, offset(c), line, column};
    case '!':
      advance();
      if (curChar() == '=')
      {
        advance();
        return Token{ElToken::NotEqual, c, c + 2, offset(c), line, column};
      }
      return Token{ElToken::LogicalNegation, c, c + 1, offset(c), line, column};
    case '<':
      advance();
      if (curChar() == '=')
      {
        advance();
        return Token{ElToken::LessOrEqual, c, c + 2, offset(c), line, column};
      }
      else if (curChar() == '<')
      {
        advance();
        return Token{ElToken::BitwiseShiftLeft, c, c + 2, offset(c), line, column};
      }
      return Token{ElToken::Less, c, c + 1, offset(c), line, column};
    case '>':
      advance();
      if (curChar() == '=')
      {
        advance();
        return Token{ElToken::GreaterOrEqual, c, c + 2, offset(c), line, column};
      }
      else if (curChar() == '>')
      {
        advance();
        return Token{ElToken::BitwiseShiftRight, c, c + 2, offset(c), line, column};
      }
      return Token{ElToken::Greater, c, c + 1, offset(c), line, column};
    case ':':
      advance();
      return Token{ElToken::Colon, c, c + 1, offset(c), line, column};
    case ',':
      advance();
      return Token{ElToken::Comma, c, c + 1, offset(c), line, column};
    case '\'':
    case '"': {
      const char delim = curChar();
      advance();
      c = curPos();
      const char* e = readQuotedString(delim);
      return Token{ElToken::String, c, e, offset(c), line, column};
    }
    case ' ':
    case '\t':
    case '\n':
    case '\r':
      discardWhile(Whitespace());
      break;
    default:
      switch (curChar())
      {
      case '.':
        if (lookAhead() == '.')
        {
          advance(2);
          return Token{ElToken::Range, c, c + 2, offset(c), line, column};
        }
        break;
      case '=':
        if (lookAhead() == '=')
        {
          advance(2);
          return Token{ElToken::Equal, c, c + 2, offset(c), line, column};
        }
        break;
      default:
        break;
      }

      if (const auto* e = readDecimal(NumberDelim()))
      {
        if (!eof() && curChar() == '.' && lookAhead() != '.')
        {
          throw ParserException{
            FileLocation{line, column}, fmt::format("Unexpected character: '{}'", *c)};
        }
        return Token{ElToken::Number, c, e, offset(c), line, column};
      }

      if (const auto* e = readInteger(IntegerDelim()))
      {
        return Token{ElToken::Number, c, e, offset(c), line, column};
      }

      if (const auto* e = discard("true"))
      {
        return Token{ElToken::Boolean, c, e, offset(c), line, column};
      }
      if (const auto* e = discard("false"))
      {
        return Token{ElToken::Boolean, c, e, offset(c), line, column};
      }

      if (const auto* e = discard("null"))
      {
        return Token{ElToken::Null, c, e, offset(c), line, column};
      }

      if (isLetter(*c) || *c == '_')
      {
        const char* e = nullptr;
        do
        {
          advance();
          e = curPos();
        } while (!eof() && (isLetter(*e) || isDigit(*e) || *e == '_'));

        return Token{ElToken::Name, c, e, offset(c), line, column};
      }

      throw ParserException{
        FileLocation{line, column}, fmt::format("Unexpected character: '{}'", *c)};
    }
  }
  return Token{ElToken::Eof, nullptr, nullptr, length(), line(), column()};
}

Parser::Parser(
  const ParseMode mode, std::string_view str, const size_t line, const size_t column)
  : m_mode{mode}
  , m_tokenizer{str, line, column}
{
}

TokenizerState Parser::tokenizerState() const
{
  return m_tokenizer.snapshot();
}

Result<ExpressionNode> Parser::parse()
{
  try
  {
    auto result = parseExpression();
    if (m_mode == ParseMode::Strict)
    {
      m_tokenizer.peekToken(ElToken::Eof); // avoid trailing garbage
    }
    return result;
  }
  catch (const ParserException& e)
  {
    return Error{e.what()};
  }
}

ExpressionNode Parser::parseExpression()
{
  if (m_tokenizer.peekToken().hasType(ElToken::OParen))
  {
    return parseGroupedTerm();
  }
  return parseTerm();
}

ExpressionNode Parser::parseGroupedTerm()
{
  auto token = m_tokenizer.nextToken(ElToken::OParen);
  auto expression = parseTerm();
  m_tokenizer.nextToken(ElToken::CParen);

  auto lhs = ExpressionNode{
    UnaryExpression{UnaryOperation::Group, std::move(expression)}, token.location()};
  if (m_tokenizer.peekToken().hasType(ElToken::CompoundTerm))
  {
    return parseCompoundTerm(lhs);
  }
  return lhs;
}

ExpressionNode Parser::parseTerm()
{
  m_tokenizer.peekToken(ElToken::SimpleTerm | ElToken::DoubleOBrace);

  auto lhs = parseSimpleTermOrSwitch();
  if (m_tokenizer.peekToken().hasType(ElToken::CompoundTerm))
  {
    return parseCompoundTerm(lhs);
  }
  return lhs;
}

ExpressionNode Parser::parseSimpleTermOrSwitch()
{
  const auto token = m_tokenizer.peekToken(ElToken::SimpleTerm | ElToken::DoubleOBrace);
  if (token.hasType(ElToken::SimpleTerm))
  {
    return parseSimpleTermOrSubscript();
  }
  return parseSwitch();
}

ExpressionNode Parser::parseSimpleTermOrSubscript()
{
  auto term = parseSimpleTerm();

  while (m_tokenizer.peekToken().hasType(ElToken::OBracket))
  {
    term = parseSubscript(std::move(term));
  }

  return term;
}

ExpressionNode Parser::parseSimpleTerm()
{
  const auto token = m_tokenizer.peekToken(ElToken::SimpleTerm);
  if (token.hasType(ElToken::UnaryOperator))
  {
    return parseUnaryOperator();
  }
  if (token.hasType(ElToken::OParen))
  {
    return parseGroupedTerm();
  }
  if (token.hasType(ElToken::Name))
  {
    return parseVariable();
  }
  return parseLiteral();
}

ExpressionNode Parser::parseSubscript(ExpressionNode lhs)
{
  const auto token = m_tokenizer.nextToken(ElToken::OBracket);
  const auto location = token.location();

  auto elements = std::vector<ExpressionNode>{};
  if (!m_tokenizer.peekToken().hasType(ElToken::CBracket))
  {
    do
    {
      elements.push_back(parseExpressionOrAnyRange());
    } while (
      m_tokenizer.nextToken(ElToken::Comma | ElToken::CBracket).hasType(ElToken::Comma));
  }
  else
  {
    m_tokenizer.nextToken();
  }

  auto rhs = elements.size() == 1u
               ? std::move(elements.front())
               : ExpressionNode{ArrayExpression{std::move(elements)}, location};
  return ExpressionNode{SubscriptExpression{std::move(lhs), std::move(rhs)}, location};
}

ExpressionNode Parser::parseVariable()
{
  const auto token = m_tokenizer.nextToken(ElToken::Name);
  return ExpressionNode{VariableExpression{token.data()}, token.location()};
}

ExpressionNode Parser::parseLiteral()
{
  const auto token =
    m_tokenizer.peekToken(ElToken::Literal | ElToken::OBracket | ElToken::OBrace);

  if (token.hasType(ElToken::String))
  {
    m_tokenizer.nextToken();
    // Escaping happens in Value::appendToStream
    auto value = kdl::str_unescape(token.data(), "\\\"");
    return ExpressionNode{LiteralExpression{Value{std::move(value)}}, token.location()};
  }
  if (token.hasType(ElToken::Number))
  {
    m_tokenizer.nextToken();
    return ExpressionNode{
      LiteralExpression{Value{token.toFloat<NumberType>()}}, token.location()};
  }
  if (token.hasType(ElToken::Boolean))
  {
    m_tokenizer.nextToken();
    return ExpressionNode{
      LiteralExpression{Value{token.data() == "true"}}, token.location()};
  }
  if (token.hasType(ElToken::Null))
  {
    m_tokenizer.nextToken();
    return ExpressionNode{LiteralExpression{Value::Null}, token.location()};
  }

  if (token.hasType(ElToken::OBracket))
  {
    return parseArray();
  }
  return parseMap();
}

ExpressionNode Parser::parseArray()
{
  const auto token = m_tokenizer.nextToken(ElToken::OBracket);
  const auto location = token.location();

  auto elements = std::vector<ExpressionNode>{};
  if (!m_tokenizer.peekToken().hasType(ElToken::CBracket))
  {
    do
    {
      elements.push_back(parseExpressionOrBoundedRange());
    } while (
      m_tokenizer.nextToken(ElToken::Comma | ElToken::CBracket).hasType(ElToken::Comma));
  }
  else
  {
    m_tokenizer.nextToken();
  }

  return ExpressionNode{ArrayExpression{std::move(elements)}, location};
}

ExpressionNode Parser::parseExpressionOrBoundedRange()
{
  auto expression = parseExpression();
  if (m_tokenizer.peekToken().hasType(ElToken::Range))
  {
    auto token = m_tokenizer.nextToken();
    expression = ExpressionNode{
      BinaryExpression{
        BinaryOperation::BoundedRange, std::move(expression), parseExpression()},
      token.location()};
  }

  return expression;
}

ExpressionNode Parser::parseExpressionOrAnyRange()
{
  auto expression = std::optional<ExpressionNode>{};
  if (m_tokenizer.peekToken().hasType(ElToken::Range))
  {
    auto token = m_tokenizer.nextToken();
    expression = ExpressionNode{
      UnaryExpression{UnaryOperation::RightBoundedRange, parseExpression()},
      token.location()};
  }
  else
  {
    expression = parseExpression();
    if (m_tokenizer.peekToken().hasType(ElToken::Range))
    {
      auto token = m_tokenizer.nextToken();
      if (m_tokenizer.peekToken().hasType(ElToken::SimpleTerm))
      {
        expression = ExpressionNode{
          BinaryExpression{
            BinaryOperation::BoundedRange, std::move(*expression), parseExpression()},
          token.location()};
      }
      else
      {
        expression = ExpressionNode{
          UnaryExpression{UnaryOperation::LeftBoundedRange, std::move(*expression)},
          token.location()};
      }
    }
  }

  return *expression;
}

ExpressionNode Parser::parseMap()
{
  auto elements = std::map<std::string, ExpressionNode>{};

  auto token = m_tokenizer.nextToken(ElToken::OBrace);
  const auto location = token.location();

  if (!m_tokenizer.peekToken().hasType(ElToken::CBrace))
  {
    do
    {
      token = m_tokenizer.nextToken(ElToken::String | ElToken::Name);
      auto key = token.data();

      m_tokenizer.nextToken(ElToken::Colon);
      elements.emplace(std::move(key), parseExpression());
    } while (
      m_tokenizer.nextToken(ElToken::Comma | ElToken::CBrace).hasType(ElToken::Comma));
  }
  else
  {
    m_tokenizer.nextToken();
  }

  return ExpressionNode{MapExpression{std::move(elements)}, location};
}

ExpressionNode Parser::parseUnaryOperator()
{
  static const auto TokenMap = std::unordered_map<ElToken::Type, UnaryOperation>{
    {ElToken::Addition, UnaryOperation::Plus},
    {ElToken::Subtraction, UnaryOperation::Minus},
    {ElToken::LogicalNegation, UnaryOperation::LogicalNegation},
    {ElToken::BitwiseNegation, UnaryOperation::BitwiseNegation},
  };

  const auto token = m_tokenizer.nextToken(ElToken::UnaryOperator);

  if (const auto it = TokenMap.find(token.type()); it != TokenMap.end())
  {
    const auto op = it->second;
    return ExpressionNode{
      UnaryExpression{op, parseSimpleTermOrSwitch()}, token.location()};
  }
  throw ParserException{
    token.location(),
    fmt::format("Unhandled unary operator: {}", m_tokenizer.tokenName(token.type()))};
}

ExpressionNode Parser::parseSwitch()
{
  auto token = m_tokenizer.nextToken(ElToken::DoubleOBrace);

  const auto location = token.location();
  auto subExpressions = std::vector<ExpressionNode>{};

  token = m_tokenizer.peekToken(ElToken::SimpleTerm | ElToken::DoubleCBrace);
  if (token.hasType(ElToken::SimpleTerm))
  {
    do
    {
      subExpressions.push_back(parseExpression());
    } while (m_tokenizer.nextToken(ElToken::Comma | ElToken::DoubleCBrace)
               .hasType(ElToken::Comma));
  }
  else if (token.hasType(ElToken::DoubleCBrace))
  {
    m_tokenizer.nextToken();
  }

  return ExpressionNode{SwitchExpression{std::move(subExpressions)}, location};
}

ExpressionNode Parser::parseCompoundTerm(ExpressionNode lhs)
{
  static const auto TokenMap = std::unordered_map<ElToken::Type, BinaryOperation>{
    {ElToken::Addition, BinaryOperation::Addition},
    {ElToken::Subtraction, BinaryOperation::Subtraction},
    {ElToken::Multiplication, BinaryOperation::Multiplication},
    {ElToken::Division, BinaryOperation::Division},
    {ElToken::Modulus, BinaryOperation::Modulus},
    {ElToken::LogicalAnd, BinaryOperation::LogicalAnd},
    {ElToken::LogicalOr, BinaryOperation::LogicalOr},
    {ElToken::BitwiseAnd, BinaryOperation::BitwiseAnd},
    {ElToken::BitwiseXOr, BinaryOperation::BitwiseXOr},
    {ElToken::BitwiseOr, BinaryOperation::BitwiseOr},
    {ElToken::BitwiseShiftLeft, BinaryOperation::BitwiseShiftLeft},
    {ElToken::BitwiseShiftRight, BinaryOperation::BitwiseShiftRight},
    {ElToken::Less, BinaryOperation::Less},
    {ElToken::LessOrEqual, BinaryOperation::LessOrEqual},
    {ElToken::Greater, BinaryOperation::Greater},
    {ElToken::GreaterOrEqual, BinaryOperation::GreaterOrEqual},
    {ElToken::Equal, BinaryOperation::Equal},
    {ElToken::NotEqual, BinaryOperation::NotEqual},
    {ElToken::Range, BinaryOperation::BoundedRange},
    {ElToken::Case, BinaryOperation::Case},
  };

  while (m_tokenizer.peekToken().hasType(ElToken::CompoundTerm))
  {
    const auto token = m_tokenizer.nextToken(ElToken::CompoundTerm);
    if (const auto it = TokenMap.find(token.type()); it != TokenMap.end())
    {
      const auto op = it->second;
      lhs = ExpressionNode{
        BinaryExpression{op, std::move(lhs), parseSimpleTermOrSwitch()},
        token.location()};
    }
    else
    {
      throw ParserException{
        token.location(),
        fmt::format(
          "Unhandled binary operator: {}", m_tokenizer.tokenName(token.type()))};
    }
  }

  return lhs;
}

} // namespace tb::el
