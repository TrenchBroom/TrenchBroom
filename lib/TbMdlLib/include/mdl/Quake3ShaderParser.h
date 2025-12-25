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

#include "Parser.h"
#include "Result.h"
#include "Tokenizer.h"

#include <string_view>

namespace tb
{
class ParserStatus;

namespace mdl
{
class Quake3Shader;
class Quake3ShaderStage;

namespace Quake3ShaderToken
{
using Type = unsigned int;
static const Type Number = 1 << 1;   // decimal number
static const Type String = 1 << 2;   // string
static const Type Variable = 1 << 3; // variable starting with $
static const Type OBrace = 1 << 4;   // opening brace: {
static const Type CBrace = 1 << 5;   // closing brace: }
static const Type Comment = 1 << 6;  // line comment starting with //
static const Type Eol = 1 << 7;      // end of line
static const Type Eof = 1 << 8;      // end of file
} // namespace Quake3ShaderToken

class Quake3ShaderTokenizer : public Tokenizer<Quake3ShaderToken::Type>
{
public:
  explicit Quake3ShaderTokenizer(std::string_view str);

private:
  Token emitToken() override;
};

class Quake3ShaderParser : public Parser<Quake3ShaderToken::Type>
{
private:
  Quake3ShaderTokenizer m_tokenizer;

public:
  explicit Quake3ShaderParser(std::string_view str);

  Result<std::vector<Quake3Shader>> parse(ParserStatus& status);

private:
  void parseTexture(Quake3Shader& shader, ParserStatus& status);
  void parseBody(Quake3Shader& shader, ParserStatus& status);
  void parseStage(Quake3Shader& shader, ParserStatus& status);
  void parseBodyEntry(Quake3Shader& shader, ParserStatus& status);
  void parseStageEntry(Quake3ShaderStage& stage, ParserStatus& status);
  void skipRemainderOfEntry();
};

} // namespace mdl
} // namespace tb
