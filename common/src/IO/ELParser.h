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

#pragma once

#include "EL/EL_Forward.h"
#include "IO/Parser.h"
#include "IO/Tokenizer.h"

#ifdef _MSC_VER
#include <cstdint>
#elif defined __GNUC__
#include <stdint.h>
#endif

#include <iosfwd>
#include <string>

namespace TrenchBroom::IO
{
namespace ELToken
{
using Type = uint64_t;
constexpr auto Name = Type{1} << 1;
constexpr auto String = Type{1} << 2;
constexpr auto Number = Type{1} << 3;
constexpr auto Boolean = Type{1} << 4;
constexpr auto OBracket = Type{1} << 5;
constexpr auto CBracket = Type{1} << 6;
constexpr auto OBrace = Type{1} << 7;
constexpr auto CBrace = Type{1} << 8;
constexpr auto OParen = Type{1} << 9;
constexpr auto CParen = Type{1} << 10;
constexpr auto Addition = Type{1} << 11;
constexpr auto Subtraction = Type{1} << 12;
constexpr auto Multiplication = Type{1} << 13;
constexpr auto Division = Type{1} << 14;
constexpr auto Colon = Type{1} << 16;
constexpr auto Modulus = Type{1} << 15;
constexpr auto Comma = Type{1} << 17;
constexpr auto Range = Type{1} << 18;
constexpr auto LogicalNegation = Type{1} << 19;
constexpr auto LogicalAnd = Type{1} << 20;
constexpr auto LogicalOr = Type{1} << 21;
constexpr auto Less = Type{1} << 22;
constexpr auto LessOrEqual = Type{1} << 23;
constexpr auto Equal = Type{1} << 24;
constexpr auto NotEqual = Type{1} << 25;
constexpr auto GreaterOrEqual = Type{1} << 26;
constexpr auto Greater = Type{1} << 27;
constexpr auto Case = Type{1} << 28;
constexpr auto BitwiseNegation = Type{1} << 29;
constexpr auto BitwiseAnd = Type{1} << 30;
constexpr auto BitwiseXOr = Type{1} << 31;
constexpr auto BitwiseOr = Type{1} << 32;
constexpr auto BitwiseShiftLeft = Type{1} << 33;
constexpr auto BitwiseShiftRight = Type{1} << 34;
constexpr auto DoubleOBrace = Type{1} << 35;
constexpr auto DoubleCBrace = Type{1} << 36;
constexpr auto Null = Type{1} << 37;
constexpr auto Eof = Type{1} << 38;
constexpr auto Literal = String | Number | Boolean | Null;
constexpr auto UnaryOperator = Addition | Subtraction | LogicalNegation | BitwiseNegation;
constexpr auto SimpleTerm = Name | Literal | OParen | OBracket | OBrace | UnaryOperator;
constexpr auto CompoundTerm =
  Addition | Subtraction | Multiplication | Division | Modulus | LogicalAnd | LogicalOr
  | Less | LessOrEqual | Equal | NotEqual | GreaterOrEqual | Greater | Case | BitwiseAnd
  | BitwiseXOr | BitwiseOr | BitwiseShiftLeft | BitwiseShiftRight;
} // namespace ELToken

class ELTokenizer : public Tokenizer<ELToken::Type>
{
private:
  const std::string& NumberDelim() const;
  const std::string& IntegerDelim() const;

public:
  ELTokenizer(std::string_view str, size_t line, size_t column);

public:
  void appendUntil(const std::string& pattern, std::stringstream& str);

private:
  Token emitToken() override;
};

class ELParser : public Parser<ELToken::Type>
{
public:
  enum class Mode
  {
    Strict,
    Lenient
  };

protected:
  ELParser::Mode m_mode;
  ELTokenizer m_tokenizer;
  using Token = ELTokenizer::Token;

public:
  ELParser(ELParser::Mode mode, std::string_view str, size_t line = 1, size_t column = 1);
  TokenizerState tokenizerState() const;

  static EL::Expression parseStrict(const std::string& str);
  static EL::Expression parseLenient(const std::string& str);

  EL::Expression parse();

private:
  EL::Expression parseExpression();
  EL::Expression parseGroupedTerm();
  EL::Expression parseTerm();
  EL::Expression parseSimpleTermOrSwitch();
  EL::Expression parseSimpleTermOrSubscript();
  EL::Expression parseSimpleTerm();
  EL::Expression parseSubscript(EL::Expression lhs);
  EL::Expression parseVariable();
  EL::Expression parseLiteral();
  EL::Expression parseArray();
  EL::Expression parseExpressionOrRange();
  EL::Expression parseExpressionOrAnyRange();
  EL::Expression parseMap();
  EL::Expression parseUnaryOperator();
  EL::Expression parseSwitch();
  EL::Expression parseCompoundTerm(EL::Expression lhs);

private:
  TokenNameMap tokenNames() const override;
};

} // namespace TrenchBroom::IO
