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

#include "ELParser.h"

#include "EL/Expressions.h"
#include "EL/Value.h"

#include "kdl/string_format.h"

#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>

namespace TrenchBroom::IO
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
      return Token(ELToken::Less, c, c + 1, offset(c), line, column);
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
    default: {
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

      const char* e;
      if ((e = readDecimal(NumberDelim())) != nullptr)
      {
        if (!eof() && curChar() == '.' && lookAhead() != '.')
        {
          throw ParserException{
            line, column, "Unexpected character: " + std::string{c, 1}};
        }
        return Token{ELToken::Number, c, e, offset(c), line, column};
      }

      if ((e = readInteger(IntegerDelim())) != nullptr)
      {
        return Token{ELToken::Number, c, e, offset(c), line, column};
      }

      if ((e = discard("true")) != nullptr)
      {
        return Token{ELToken::Boolean, c, e, offset(c), line, column};
      }
      if ((e = discard("false")) != nullptr)
      {
        return Token{ELToken::Boolean, c, e, offset(c), line, column};
      }

      if ((e = discard("null")) != nullptr)
      {
        return Token{ELToken::Null, c, e, offset(c), line, column};
      }

      if (isLetter(*c) || *c == '_')
      {
        do
        {
          advance();
          e = curPos();
        } while (!eof() && (isLetter(*e) || isDigit(*e) || *e == '_'));

        return Token{ELToken::Name, c, e, offset(c), line, column};
      }

      throw ParserException{line, column, "Unexpected character: " + std::string{c, 1}};
    }
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

EL::Expression ELParser::parseStrict(const std::string& str)
{
  return ELParser{Mode::Strict, str}.parse();
}

EL::Expression ELParser::parseLenient(const std::string& str)
{
  return ELParser(Mode::Lenient, str).parse();
}

EL::Expression ELParser::parse()
{
  auto result = parseExpression();
  if (m_mode == Mode::Strict)
  {
    expect(ELToken::Eof, m_tokenizer.peekToken()); // avoid trailing garbage
  }
  return result;
}

EL::Expression ELParser::parseExpression()
{
  if (m_tokenizer.peekToken().hasType(ELToken::OParen))
  {
    return parseGroupedTerm();
  }
  return parseTerm();
}

EL::Expression ELParser::parseGroupedTerm()
{
  auto token = m_tokenizer.nextToken();
  expect(ELToken::OParen, token);
  auto expression = parseTerm();
  expect(ELToken::CParen, m_tokenizer.nextToken());

  auto lhs = EL::Expression{
    EL::UnaryExpression{EL::UnaryOperator::Group, std::move(expression)},
    token.line(),
    token.column()};
  if (m_tokenizer.peekToken().hasType(ELToken::CompoundTerm))
  {
    return parseCompoundTerm(lhs);
  }
  return lhs;
}

EL::Expression ELParser::parseTerm()
{
  expect(ELToken::SimpleTerm | ELToken::DoubleOBrace, m_tokenizer.peekToken());

  auto lhs = parseSimpleTermOrSwitch();
  if (m_tokenizer.peekToken().hasType(ELToken::CompoundTerm))
  {
    return parseCompoundTerm(lhs);
  }
  return lhs;
}

EL::Expression ELParser::parseSimpleTermOrSwitch()
{
  auto token = m_tokenizer.peekToken();
  expect(ELToken::SimpleTerm | ELToken::DoubleOBrace, token);

  if (token.hasType(ELToken::SimpleTerm))
  {
    return parseSimpleTermOrSubscript();
  }
  return parseSwitch();
}

EL::Expression ELParser::parseSimpleTermOrSubscript()
{
  auto term = parseSimpleTerm();

  while (m_tokenizer.peekToken().hasType(ELToken::OBracket))
  {
    term = parseSubscript(std::move(term));
  }

  return term;
}

EL::Expression ELParser::parseSimpleTerm()
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

EL::Expression ELParser::parseSubscript(EL::Expression lhs)
{
  auto token = m_tokenizer.nextToken();
  const auto line = token.line();
  const auto column = token.column();

  expect(ELToken::OBracket, token);
  auto elements = std::vector<EL::Expression>{};
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
               : EL::Expression{EL::ArrayExpression{std::move(elements)}, line, column};
  return {EL::SubscriptExpression{std::move(lhs), std::move(rhs)}, line, column};
}

EL::Expression ELParser::parseVariable()
{
  auto token = m_tokenizer.nextToken();
  expect(ELToken::Name, token);
  return {EL::VariableExpression{token.data()}, token.line(), token.column()};
}

EL::Expression ELParser::parseLiteral()
{
  auto token = m_tokenizer.peekToken();
  expect(ELToken::Literal | ELToken::OBracket | ELToken::OBrace, token);

  if (token.hasType(ELToken::String))
  {
    m_tokenizer.nextToken();
    // Escaping happens in EL::Value::appendToStream
    auto value = kdl::str_unescape(token.data(), "\\\"");
    return {
      EL::LiteralExpression{EL::Value{std::move(value)}}, token.line(), token.column()};
  }
  if (token.hasType(ELToken::Number))
  {
    m_tokenizer.nextToken();
    return {
      EL::LiteralExpression{EL::Value{token.toFloat<EL::NumberType>()}},
      token.line(),
      token.column()};
  }
  if (token.hasType(ELToken::Boolean))
  {
    m_tokenizer.nextToken();
    return {
      EL::LiteralExpression{EL::Value{token.data() == "true"}},
      token.line(),
      token.column()};
  }
  if (token.hasType(ELToken::Null))
  {
    m_tokenizer.nextToken();
    return {EL::LiteralExpression{EL::Value::Null}, token.line(), token.column()};
  }

  if (token.hasType(ELToken::OBracket))
  {
    return parseArray();
  }
  return parseMap();
}

EL::Expression ELParser::parseArray()
{
  auto token = m_tokenizer.nextToken();
  const auto line = token.line();
  const auto column = token.column();

  expect(ELToken::OBracket, token);
  auto elements = std::vector<EL::Expression>{};
  if (!m_tokenizer.peekToken().hasType(ELToken::CBracket))
  {
    do
    {
      elements.push_back(parseExpressionOrRange());
    } while (expect(ELToken::Comma | ELToken::CBracket, m_tokenizer.nextToken())
               .hasType(ELToken::Comma));
  }
  else
  {
    m_tokenizer.nextToken();
  }

  return {EL::ArrayExpression{std::move(elements)}, line, column};
}

EL::Expression ELParser::parseExpressionOrRange()
{
  auto expression = parseExpression();
  if (m_tokenizer.peekToken().hasType(ELToken::Range))
  {
    auto token = m_tokenizer.nextToken();
    expression = EL::Expression{
      EL::BinaryExpression{
        EL::BinaryOperator::Range, std::move(expression), parseExpression()},
      token.line(),
      token.column()};
  }

  return expression;
}

EL::Expression ELParser::parseExpressionOrAnyRange()
{
  auto expression = std::optional<EL::Expression>{};
  if (m_tokenizer.peekToken().hasType(ELToken::Range))
  {
    auto token = m_tokenizer.nextToken();
    expression = EL::BinaryExpression::createAutoRangeWithRightOperand(
      parseExpression(), token.line(), token.column());
  }
  else
  {
    expression = parseExpression();
    if (m_tokenizer.peekToken().hasType(ELToken::Range))
    {
      auto token = m_tokenizer.nextToken();
      if (m_tokenizer.peekToken().hasType(ELToken::SimpleTerm))
      {
        expression = EL::Expression{
          EL::BinaryExpression{
            EL::BinaryOperator::Range, std::move(*expression), parseExpression()},
          token.line(),
          token.column()};
      }
      else
      {
        expression = EL::BinaryExpression::createAutoRangeWithLeftOperand(
          std::move(*expression), token.line(), token.column());
      }
    }
  }

  return *expression;
}

EL::Expression ELParser::parseMap()
{
  auto elements = std::map<std::string, EL::Expression>{};

  auto token = m_tokenizer.nextToken();
  const auto line = token.line();
  const auto column = token.column();

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

  return {EL::MapExpression{std::move(elements)}, line, column};
}

EL::Expression ELParser::parseUnaryOperator()
{
  static const auto TokenMap = std::unordered_map<ELToken::Type, EL::UnaryOperator>{
    {ELToken::Addition, EL::UnaryOperator::Plus},
    {ELToken::Subtraction, EL::UnaryOperator::Minus},
    {ELToken::LogicalNegation, EL::UnaryOperator::LogicalNegation},
    {ELToken::BitwiseNegation, EL::UnaryOperator::BitwiseNegation},
  };

  auto token = m_tokenizer.nextToken();
  expect(ELToken::UnaryOperator, token);

  if (const auto it = TokenMap.find(token.type()); it != TokenMap.end())
  {
    const auto op = it->second;
    return {
      EL::UnaryExpression{op, parseSimpleTermOrSwitch()}, token.line(), token.column()};
  }
  throw ParserException{
    token.line(), token.column(), "Unhandled unary operator: " + tokenName(token.type())};
}

EL::Expression ELParser::parseSwitch()
{
  auto token = m_tokenizer.nextToken();
  expect(ELToken::DoubleOBrace, token);

  const auto line = token.line();
  const auto column = token.column();
  auto subExpressions = std::vector<EL::Expression>{};

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

  return {EL::SwitchExpression{std::move(subExpressions)}, line, column};
}

EL::Expression ELParser::parseCompoundTerm(EL::Expression lhs)
{
  static const auto TokenMap = std::unordered_map<ELToken::Type, EL::BinaryOperator>{
    {ELToken::Addition, EL::BinaryOperator::Addition},
    {ELToken::Subtraction, EL::BinaryOperator::Subtraction},
    {ELToken::Multiplication, EL::BinaryOperator::Multiplication},
    {ELToken::Division, EL::BinaryOperator::Division},
    {ELToken::Modulus, EL::BinaryOperator::Modulus},
    {ELToken::LogicalAnd, EL::BinaryOperator::LogicalAnd},
    {ELToken::LogicalOr, EL::BinaryOperator::LogicalOr},
    {ELToken::BitwiseAnd, EL::BinaryOperator::BitwiseAnd},
    {ELToken::BitwiseXOr, EL::BinaryOperator::BitwiseXOr},
    {ELToken::BitwiseOr, EL::BinaryOperator::BitwiseOr},
    {ELToken::BitwiseShiftLeft, EL::BinaryOperator::BitwiseShiftLeft},
    {ELToken::BitwiseShiftRight, EL::BinaryOperator::BitwiseShiftRight},
    {ELToken::Less, EL::BinaryOperator::Less},
    {ELToken::LessOrEqual, EL::BinaryOperator::LessOrEqual},
    {ELToken::Greater, EL::BinaryOperator::Greater},
    {ELToken::GreaterOrEqual, EL::BinaryOperator::GreaterOrEqual},
    {ELToken::Equal, EL::BinaryOperator::Equal},
    {ELToken::NotEqual, EL::BinaryOperator::NotEqual},
    {ELToken::Range, EL::BinaryOperator::Range},
    {ELToken::Case, EL::BinaryOperator::Case},
  };

  while (m_tokenizer.peekToken().hasType(ELToken::CompoundTerm))
  {
    auto token = m_tokenizer.nextToken();
    expect(ELToken::CompoundTerm, token);


    if (const auto it = TokenMap.find(token.type()); it != TokenMap.end())
    {
      const auto op = it->second;
      lhs = EL::Expression{
        EL::BinaryExpression{op, std::move(lhs), parseSimpleTermOrSwitch()},
        token.line(),
        token.column()};
    }
    else
    {
      throw ParserException{
        token.line(),
        token.column(),
        "Unhandled binary operator: " + tokenName(token.type())};
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

} // namespace TrenchBroom::IO
