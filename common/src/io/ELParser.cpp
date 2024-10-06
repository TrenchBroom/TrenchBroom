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

#include "ELParser.h"

#include "FileLocation.h"
#include "el/Expression.h"
#include "el/Value.h"

#include "kdl/string_format.h"

#include <fmt/format.h>

#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>

namespace tb::io
{

const std::string& ELTokenizer::NumberDelim() const
{
  static const auto Delim = Whitespace() + "(){}[],:+-*/%";
  return Delim;
}

const std::string& ELTokenizer::IntegerDelim() const
{
  static const auto Delim = NumberDelim() + ".";
  return Delim;
}

ELTokenizer::ELTokenizer(std::string_view str, const size_t line, const size_t column)
  : Tokenizer{std::move(str), "\"", '\\', line, column}
{
}

void ELTokenizer::appendUntil(const std::string& pattern, std::stringstream& str)
{
  const auto* begin = curPos();
  const auto* end = discardUntilPattern(pattern);
  str << std::string{begin, end};
  if (!eof())
  {
    discard("${");
  }
}

ELTokenizer::Token ELTokenizer::emitToken()
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
      return Token{ELToken::OBracket, c, c + 1, offset(c), line, column};
    case ']':
      advance();
      return Token{ELToken::CBracket, c, c + 1, offset(c), line, column};
    case '{':
      advance();
      if (curChar() == '{')
      {
        advance();
        return Token{ELToken::DoubleOBrace, c, c + 2, offset(c), line, column};
      }
      return Token{ELToken::OBrace, c, c + 1, offset(c), line, column};
    case '}':
      advance();
      if (curChar() == '}')
      {
        advance();
        return Token{ELToken::DoubleCBrace, c, c + 2, offset(c), line, column};
      }
      return Token{ELToken::CBrace, c, c + 1, offset(c), line, column};
    case '(':
      advance();
      return Token{ELToken::OParen, c, c + 1, offset(c), line, column};
    case ')':
      advance();
      return Token{ELToken::CParen, c, c + 1, offset(c), line, column};
    case '+':
      advance();
      return Token{ELToken::Addition, c, c + 1, offset(c), line, column};
    case '-':
      advance();
      if (curChar() == '>')
      {
        advance();
        return Token{ELToken::Case, c, c + 2, offset(c), line, column};
      }
      return Token{ELToken::Subtraction, c, c + 1, offset(c), line, column};
    case '*':
      advance();
      return Token{ELToken::Multiplication, c, c + 1, offset(c), line, column};
    case '/':
      advance();
      if (curChar() == '/')
      {
        discardUntil("\n\r");
        break;
      }
      return Token{ELToken::Division, c, c + 1, offset(c), line, column};
    case '%':
      advance();
      return Token{ELToken::Modulus, c, c + 1, offset(c), line, column};
    case '~':
      advance();
      return Token{ELToken::BitwiseNegation, c, c + 1, offset(c), line, column};
    case '&':
      advance();
      if (curChar() == '&')
      {
        advance();
        return Token{ELToken::LogicalAnd, c, c + 2, offset(c), line, column};
      }
      return Token{ELToken::BitwiseAnd, c, c + 1, offset(c), line, column};
    case '|':
      advance();
      if (curChar() == '|')
      {
        advance();
        return Token{ELToken::LogicalOr, c, c + 2, offset(c), line, column};
      }
      return Token{ELToken::BitwiseOr, c, c + 1, offset(c), line, column};
    case '^':
      advance();
      return Token{ELToken::BitwiseXOr, c, c + 1, offset(c), line, column};
    case '!':
      advance();
      if (curChar() == '=')
      {
        advance();
        return Token{ELToken::NotEqual, c, c + 2, offset(c), line, column};
      }
      return Token{ELToken::LogicalNegation, c, c + 1, offset(c), line, column};
    case '<':
      advance();
      if (curChar() == '=')
      {
        advance();
        return Token{ELToken::LessOrEqual, c, c + 2, offset(c), line, column};
      }
      else if (curChar() == '<')
      {
        advance();
        return Token{ELToken::BitwiseShiftLeft, c, c + 2, offset(c), line, column};
      }
      return Token{ELToken::Less, c, c + 1, offset(c), line, column};
    case '>':
      advance();
      if (curChar() == '=')
      {
        advance();
        return Token{ELToken::GreaterOrEqual, c, c + 2, offset(c), line, column};
      }
      else if (curChar() == '>')
      {
        advance();
        return Token{ELToken::BitwiseShiftRight, c, c + 2, offset(c), line, column};
      }
      return Token{ELToken::Greater, c, c + 1, offset(c), line, column};
    case ':':
      advance();
      return Token{ELToken::Colon, c, c + 1, offset(c), line, column};
    case ',':
      advance();
      return Token{ELToken::Comma, c, c + 1, offset(c), line, column};
    case '\'':
    case '"': {
      const char delim = curChar();
      advance();
      c = curPos();
      const char* e = readQuotedString(delim);
      return Token{ELToken::String, c, e, offset(c), line, column};
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
          return Token{ELToken::Range, c, c + 2, offset(c), line, column};
        }
        break;
      case '=':
        if (curChar() == '=')
        {
          advance(2);
          return Token{ELToken::Equal, c, c + 2, offset(c), line, column};
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
        return Token{ELToken::Number, c, e, offset(c), line, column};
      }

      if (const auto* e = readInteger(IntegerDelim()))
      {
        return Token{ELToken::Number, c, e, offset(c), line, column};
      }

      if (const auto* e = discard("true"))
      {
        return Token{ELToken::Boolean, c, e, offset(c), line, column};
      }
      if (const auto* e = discard("false"))
      {
        return Token{ELToken::Boolean, c, e, offset(c), line, column};
      }

      if (const auto* e = discard("null"))
      {
        return Token{ELToken::Null, c, e, offset(c), line, column};
      }

      if (isLetter(*c) || *c == '_')
      {
        const char* e = nullptr;
        do
        {
          advance();
          e = curPos();
        } while (!eof() && (isLetter(*e) || isDigit(*e) || *e == '_'));

        return Token{ELToken::Name, c, e, offset(c), line, column};
      }

      throw ParserException{
        FileLocation{line, column}, fmt::format("Unexpected character: '{}'", *c)};
    }
  }
  return Token{ELToken::Eof, nullptr, nullptr, length(), line(), column()};
}

ELParser::ELParser(
  const ELParser::Mode mode, std::string_view str, const size_t line, const size_t column)
  : m_mode{mode}
  , m_tokenizer{str, line, column}
{
}

TokenizerState ELParser::tokenizerState() const
{
  return m_tokenizer.snapshot();
}

el::ExpressionNode ELParser::parseStrict(const std::string& str)
{
  return ELParser{Mode::Strict, str}.parse();
}

el::ExpressionNode ELParser::parseLenient(const std::string& str)
{
  return ELParser(Mode::Lenient, str).parse();
}

el::ExpressionNode ELParser::parse()
{
  auto result = parseExpression();
  if (m_mode == Mode::Strict)
  {
    expect(ELToken::Eof, m_tokenizer.peekToken()); // avoid trailing garbage
  }
  return result;
}

el::ExpressionNode ELParser::parseExpression()
{
  if (m_tokenizer.peekToken().hasType(ELToken::OParen))
  {
    return parseGroupedTerm();
  }
  return parseTerm();
}

el::ExpressionNode ELParser::parseGroupedTerm()
{
  auto token = m_tokenizer.nextToken();
  expect(ELToken::OParen, token);
  auto expression = parseTerm();
  expect(ELToken::CParen, m_tokenizer.nextToken());

  auto lhs = el::ExpressionNode{
    el::UnaryExpression{el::UnaryOperation::Group, std::move(expression)},
    token.location()};
  if (m_tokenizer.peekToken().hasType(ELToken::CompoundTerm))
  {
    return parseCompoundTerm(lhs);
  }
  return lhs;
}

el::ExpressionNode ELParser::parseTerm()
{
  expect(ELToken::SimpleTerm | ELToken::DoubleOBrace, m_tokenizer.peekToken());

  auto lhs = parseSimpleTermOrSwitch();
  if (m_tokenizer.peekToken().hasType(ELToken::CompoundTerm))
  {
    return parseCompoundTerm(lhs);
  }
  return lhs;
}

el::ExpressionNode ELParser::parseSimpleTermOrSwitch()
{
  auto token = m_tokenizer.peekToken();
  expect(ELToken::SimpleTerm | ELToken::DoubleOBrace, token);

  if (token.hasType(ELToken::SimpleTerm))
  {
    return parseSimpleTermOrSubscript();
  }
  return parseSwitch();
}

el::ExpressionNode ELParser::parseSimpleTermOrSubscript()
{
  auto term = parseSimpleTerm();

  while (m_tokenizer.peekToken().hasType(ELToken::OBracket))
  {
    term = parseSubscript(std::move(term));
  }

  return term;
}

el::ExpressionNode ELParser::parseSimpleTerm()
{
  auto token = m_tokenizer.peekToken();
  expect(ELToken::SimpleTerm, token);

  if (token.hasType(ELToken::UnaryOperator))
  {
    return parseUnaryOperator();
  }
  if (token.hasType(ELToken::OParen))
  {
    return parseGroupedTerm();
  }
  if (token.hasType(ELToken::Name))
  {
    return parseVariable();
  }
  return parseLiteral();
}

el::ExpressionNode ELParser::parseSubscript(el::ExpressionNode lhs)
{
  auto token = m_tokenizer.nextToken();
  const auto location = token.location();

  expect(ELToken::OBracket, token);
  auto elements = std::vector<el::ExpressionNode>{};
  if (!m_tokenizer.peekToken().hasType(ELToken::CBracket))
  {
    do
    {
      elements.push_back(parseExpressionOrAnyRange());
    } while (expect(ELToken::Comma | ELToken::CBracket, m_tokenizer.nextToken())
               .hasType(ELToken::Comma));
  }
  else
  {
    m_tokenizer.nextToken();
  }

  auto rhs = elements.size() == 1u
               ? std::move(elements.front())
               : el::ExpressionNode{el::ArrayExpression{std::move(elements)}, location};
  return el::ExpressionNode{
    el::SubscriptExpression{std::move(lhs), std::move(rhs)}, location};
}

el::ExpressionNode ELParser::parseVariable()
{
  auto token = m_tokenizer.nextToken();
  expect(ELToken::Name, token);
  return el::ExpressionNode{el::VariableExpression{token.data()}, token.location()};
}

el::ExpressionNode ELParser::parseLiteral()
{
  auto token = m_tokenizer.peekToken();
  expect(ELToken::Literal | ELToken::OBracket | ELToken::OBrace, token);

  if (token.hasType(ELToken::String))
  {
    m_tokenizer.nextToken();
    // Escaping happens in el::Value::appendToStream
    auto value = kdl::str_unescape(token.data(), "\\\"");
    return el::ExpressionNode{
      el::LiteralExpression{el::Value{std::move(value)}}, token.location()};
  }
  if (token.hasType(ELToken::Number))
  {
    m_tokenizer.nextToken();
    return el::ExpressionNode{
      el::LiteralExpression{el::Value{token.toFloat<el::NumberType>()}},
      token.location()};
  }
  if (token.hasType(ELToken::Boolean))
  {
    m_tokenizer.nextToken();
    return el::ExpressionNode{
      el::LiteralExpression{el::Value{token.data() == "true"}}, token.location()};
  }
  if (token.hasType(ELToken::Null))
  {
    m_tokenizer.nextToken();
    return el::ExpressionNode{el::LiteralExpression{el::Value::Null}, token.location()};
  }

  if (token.hasType(ELToken::OBracket))
  {
    return parseArray();
  }
  return parseMap();
}

el::ExpressionNode ELParser::parseArray()
{
  auto token = m_tokenizer.nextToken();
  const auto location = token.location();

  expect(ELToken::OBracket, token);
  auto elements = std::vector<el::ExpressionNode>{};
  if (!m_tokenizer.peekToken().hasType(ELToken::CBracket))
  {
    do
    {
      elements.push_back(parseExpressionOrBoundedRange());
    } while (expect(ELToken::Comma | ELToken::CBracket, m_tokenizer.nextToken())
               .hasType(ELToken::Comma));
  }
  else
  {
    m_tokenizer.nextToken();
  }

  return el::ExpressionNode{el::ArrayExpression{std::move(elements)}, location};
}

el::ExpressionNode ELParser::parseExpressionOrBoundedRange()
{
  auto expression = parseExpression();
  if (m_tokenizer.peekToken().hasType(ELToken::Range))
  {
    auto token = m_tokenizer.nextToken();
    expression = el::ExpressionNode{
      el::BinaryExpression{
        el::BinaryOperation::BoundedRange, std::move(expression), parseExpression()},
      token.location()};
  }

  return expression;
}

el::ExpressionNode ELParser::parseExpressionOrAnyRange()
{
  auto expression = std::optional<el::ExpressionNode>{};
  if (m_tokenizer.peekToken().hasType(ELToken::Range))
  {
    auto token = m_tokenizer.nextToken();
    expression = el::ExpressionNode{
      el::UnaryExpression{el::UnaryOperation::RightBoundedRange, parseExpression()},
      token.location()};
  }
  else
  {
    expression = parseExpression();
    if (m_tokenizer.peekToken().hasType(ELToken::Range))
    {
      auto token = m_tokenizer.nextToken();
      if (m_tokenizer.peekToken().hasType(ELToken::SimpleTerm))
      {
        expression = el::ExpressionNode{
          el::BinaryExpression{
            el::BinaryOperation::BoundedRange, std::move(*expression), parseExpression()},
          token.location()};
      }
      else
      {
        expression = el::ExpressionNode{
          el::UnaryExpression{
            el::UnaryOperation::LeftBoundedRange, std::move(*expression)},
          token.location()};
      }
    }
  }

  return *expression;
}

el::ExpressionNode ELParser::parseMap()
{
  auto elements = std::map<std::string, el::ExpressionNode>{};

  auto token = m_tokenizer.nextToken();
  const auto location = token.location();

  expect(ELToken::OBrace, token);
  if (!m_tokenizer.peekToken().hasType(ELToken::CBrace))
  {
    do
    {
      token = m_tokenizer.nextToken();
      expect(ELToken::String | ELToken::Name, token);
      auto key = token.data();

      expect(ELToken::Colon, m_tokenizer.nextToken());
      elements.emplace(std::move(key), parseExpression());
    } while (expect(ELToken::Comma | ELToken::CBrace, m_tokenizer.nextToken())
               .hasType(ELToken::Comma));
  }
  else
  {
    m_tokenizer.nextToken();
  }

  return el::ExpressionNode{el::MapExpression{std::move(elements)}, location};
}

el::ExpressionNode ELParser::parseUnaryOperator()
{
  static const auto TokenMap = std::unordered_map<ELToken::Type, el::UnaryOperation>{
    {ELToken::Addition, el::UnaryOperation::Plus},
    {ELToken::Subtraction, el::UnaryOperation::Minus},
    {ELToken::LogicalNegation, el::UnaryOperation::LogicalNegation},
    {ELToken::BitwiseNegation, el::UnaryOperation::BitwiseNegation},
  };

  auto token = m_tokenizer.nextToken();
  expect(ELToken::UnaryOperator, token);

  if (const auto it = TokenMap.find(token.type()); it != TokenMap.end())
  {
    const auto op = it->second;
    return el::ExpressionNode{
      el::UnaryExpression{op, parseSimpleTermOrSwitch()}, token.location()};
  }
  throw ParserException{
    token.location(),
    fmt::format("Unhandled unary operator: {}", tokenName(token.type()))};
}

el::ExpressionNode ELParser::parseSwitch()
{
  auto token = m_tokenizer.nextToken();
  expect(ELToken::DoubleOBrace, token);

  const auto location = token.location();
  auto subExpressions = std::vector<el::ExpressionNode>{};

  token = m_tokenizer.peekToken();
  expect(ELToken::SimpleTerm | ELToken::DoubleCBrace, token);

  if (token.hasType(ELToken::SimpleTerm))
  {
    do
    {
      subExpressions.push_back(parseExpression());
    } while (expect(ELToken::Comma | ELToken::DoubleCBrace, m_tokenizer.nextToken())
               .hasType(ELToken::Comma));
  }
  else if (token.hasType(ELToken::DoubleCBrace))
  {
    m_tokenizer.nextToken();
  }

  return el::ExpressionNode{el::SwitchExpression{std::move(subExpressions)}, location};
}

el::ExpressionNode ELParser::parseCompoundTerm(el::ExpressionNode lhs)
{
  static const auto TokenMap = std::unordered_map<ELToken::Type, el::BinaryOperation>{
    {ELToken::Addition, el::BinaryOperation::Addition},
    {ELToken::Subtraction, el::BinaryOperation::Subtraction},
    {ELToken::Multiplication, el::BinaryOperation::Multiplication},
    {ELToken::Division, el::BinaryOperation::Division},
    {ELToken::Modulus, el::BinaryOperation::Modulus},
    {ELToken::LogicalAnd, el::BinaryOperation::LogicalAnd},
    {ELToken::LogicalOr, el::BinaryOperation::LogicalOr},
    {ELToken::BitwiseAnd, el::BinaryOperation::BitwiseAnd},
    {ELToken::BitwiseXOr, el::BinaryOperation::BitwiseXOr},
    {ELToken::BitwiseOr, el::BinaryOperation::BitwiseOr},
    {ELToken::BitwiseShiftLeft, el::BinaryOperation::BitwiseShiftLeft},
    {ELToken::BitwiseShiftRight, el::BinaryOperation::BitwiseShiftRight},
    {ELToken::Less, el::BinaryOperation::Less},
    {ELToken::LessOrEqual, el::BinaryOperation::LessOrEqual},
    {ELToken::Greater, el::BinaryOperation::Greater},
    {ELToken::GreaterOrEqual, el::BinaryOperation::GreaterOrEqual},
    {ELToken::Equal, el::BinaryOperation::Equal},
    {ELToken::NotEqual, el::BinaryOperation::NotEqual},
    {ELToken::Range, el::BinaryOperation::BoundedRange},
    {ELToken::Case, el::BinaryOperation::Case},
  };

  while (m_tokenizer.peekToken().hasType(ELToken::CompoundTerm))
  {
    auto token = m_tokenizer.nextToken();
    expect(ELToken::CompoundTerm, token);


    if (const auto it = TokenMap.find(token.type()); it != TokenMap.end())
    {
      const auto op = it->second;
      lhs = el::ExpressionNode{
        el::BinaryExpression{op, std::move(lhs), parseSimpleTermOrSwitch()},
        token.location()};
    }
    else
    {
      throw ParserException{
        token.location(),
        fmt::format("Unhandled binary operator: {}", tokenName(token.type()))};
    }
  }

  return lhs;
}

ELParser::TokenNameMap ELParser::tokenNames() const
{
  return {
    {ELToken::Name, "variable"},
    {ELToken::String, "string"},
    {ELToken::Number, "number"},
    {ELToken::Boolean, "boolean"},
    {ELToken::OBracket, "'['"},
    {ELToken::CBracket, "']'"},
    {ELToken::OBrace, "'{'"},
    {ELToken::CBrace, "'}'"},
    {ELToken::OParen, "'('"},
    {ELToken::CParen, "')'"},
    {ELToken::Addition, "'+'"},
    {ELToken::Subtraction, "'-'"},
    {ELToken::Multiplication, "'*'"},
    {ELToken::Division, "'/'"},
    {ELToken::Modulus, "'%'"},
    {ELToken::Colon, "':'"},
    {ELToken::Comma, "','"},
    {ELToken::Range, "'..'"},
    {ELToken::LogicalNegation, "'!'"},
    {ELToken::LogicalAnd, "'&&'"},
    {ELToken::LogicalOr, "'||'"},
    {ELToken::Less, "'<'"},
    {ELToken::LessOrEqual, "'<='"},
    {ELToken::Equal, "'=='"},
    {ELToken::NotEqual, "'!='"},
    {ELToken::GreaterOrEqual, "'>='"},
    {ELToken::Greater, "'>'"},
    {ELToken::Case, "'->'"},
    {ELToken::BitwiseNegation, "'~'"},
    {ELToken::BitwiseAnd, "'&'"},
    {ELToken::BitwiseOr, "'|'"},
    {ELToken::BitwiseShiftLeft, "'<<'"},
    {ELToken::BitwiseShiftRight, "'>>'"},
    {ELToken::DoubleOBrace, "'{{'"},
    {ELToken::DoubleCBrace, "'}}'"},
    {ELToken::Null, "'null'"},
    {ELToken::Eof, "end of file"},
  };
}

} // namespace tb::io
