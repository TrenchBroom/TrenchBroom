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

#pragma once

#include "el/EL_Forward.h"
#include "io/Parser.h"
#include "io/Tokenizer.h"

#include <string_view>

namespace tb::io
{
class ParserStatus;

namespace MdlToken
{
using Type = size_t;
static const Type Integer = 1 << 0;
static const Type Equality = 1 << 1;
static const Type Word = 1 << 2;
static const Type String = 1 << 3;
static const Type Comma = 1 << 4;
static const Type CParenthesis = 1 << 5;
static const Type Eof = 1 << 6;
} // namespace MdlToken

class LegacyModelDefinitionTokenizer : public Tokenizer<MdlToken::Type>
{
public:
  LegacyModelDefinitionTokenizer(std::string_view str, size_t line, size_t column);

private:
  static const std::string WordDelims;
  Token emitToken() override;
};

class LegacyModelDefinitionParser : public Parser<MdlToken::Type>
{
private:
  using Token = LegacyModelDefinitionTokenizer::Token;
  LegacyModelDefinitionTokenizer m_tokenizer;

public:
  LegacyModelDefinitionParser(std::string_view str, size_t line, size_t column);
  TokenizerState tokenizerState() const;

public:
  el::ExpressionNode parse(ParserStatus& status);

private:
  el::ExpressionNode parseModelDefinition(ParserStatus& status);
  el::ExpressionNode parseStaticModelDefinition();
  el::ExpressionNode parseDynamicModelDefinition(ParserStatus& status);
  el::ExpressionNode parseNamedValue(const std::string& name);
};

} // namespace tb::io
