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

#include "Quake3ShaderParser.h"

#include "Macros.h"
#include "ParserStatus.h"
#include "mdl/Quake3Shader.h"

#include "kd/string_compare.h"
#include "kd/string_format.h"

#include <filesystem>
#include <string>

namespace tb::mdl
{
namespace
{

auto tokenNames()
{
  using namespace Quake3ShaderToken;

  return Quake3ShaderTokenizer::TokenNameMap{
    {Number, "number"},
    {String, "string"},
    {Variable, "variable"},
    {OBrace, "'{'"},
    {CBrace, "'}'"},
    {Comment, "comment"},
    {Eol, "end of line"},
    {Eof, "end of file"},
  };
}

} // namespace

Quake3ShaderTokenizer::Quake3ShaderTokenizer(const std::string_view str)
  : Tokenizer{tokenNames(), str, "", '\\'}
{
}

Tokenizer<unsigned int>::Token Quake3ShaderTokenizer::emitToken()
{
  while (!eof())
  {
    const auto startLine = line();
    const auto startColumn = column();
    const auto startLocation = location();
    const auto* c = curPos();
    switch (*c)
    {
    case '{':
      advance();
      return Token{
        Quake3ShaderToken::OBrace, c, c + 1, offset(c), startLine, startColumn};
    case '}':
      advance();
      return Token{
        Quake3ShaderToken::CBrace, c, c + 1, offset(c), startLine, startColumn};
    case '\r':
      if (lookAhead() == '\n')
      {
        advance();
      }
      // handle carriage return without consecutive linefeed
      // by falling through into the line feed case
      switchFallthrough();
    case '\n':
      discardWhile(Whitespace()); // handle empty lines and such
      return Token{Quake3ShaderToken::Eol, c, c + 1, offset(c), startLine, startColumn};
    case ' ':
    case '\t':
      advance();
      break;
    case '$':
      if (const auto* e = readUntil(Whitespace()))
      {
        return Token{
          Quake3ShaderToken::Variable, c, e, offset(c), startLine, startColumn};
      }
      throw ParserException{startLocation, "Unexpected character: " + std::string{c, 1}};
    case '/':
      if (lookAhead() == '/')
      {
        // parse single line comment starting with //
        advance(2);
        discardUntil("\n\r");
        // do not discard the terminating line break since it might be semantically
        // relevant e.g. for terminating a block entry
        break;
      }
      if (lookAhead() == '*')
      {
        // parse multiline comment delimited by /* and */
        advance(2);
        while (curChar() != '*' || lookAhead() != '/')
        {
          errorIfEof();
          advance();
        }
        advance(2);
        break;
      }
      // fall through into the default case to parse a string that starts with '/'
      switchFallthrough();
    default:
      if (const auto* e = readDecimal(Whitespace()))
      {
        return Token{Quake3ShaderToken::Number, c, e, offset(c), startLine, startColumn};
      }

      if (const auto* e = readUntil(Whitespace()))
      {
        return Token{Quake3ShaderToken::String, c, e, offset(c), startLine, startColumn};
      }

      throw ParserException{startLocation, "Unexpected character: " + std::string{c, 1}};
    }
  }
  return Token{Quake3ShaderToken::Eof, nullptr, nullptr, length(), line(), column()};
}

Quake3ShaderParser::Quake3ShaderParser(std::string_view str)
  : m_tokenizer{std::move(str)}
{
}

Result<std::vector<Quake3Shader>> Quake3ShaderParser::parse(ParserStatus& status)
{
  try
  {
    auto result = std::vector<Quake3Shader>{};
    while (!m_tokenizer.skipAndPeekToken(Quake3ShaderToken::Eol)
              .hasType(Quake3ShaderToken::Eof))
    {
      auto shader = Quake3Shader{};
      parseTexture(shader, status);
      parseBody(shader, status);
      result.push_back(shader);
    }
    return result;
  }
  catch (const ParserException& e)
  {
    return Error{e.what()};
  }
}

void Quake3ShaderParser::parseBody(Quake3Shader& shader, ParserStatus& status)
{
  m_tokenizer.skipAndNextToken(Quake3ShaderToken::Eol, Quake3ShaderToken::OBrace);
  auto token = m_tokenizer.skipAndPeekToken(
    Quake3ShaderToken::Eol,
    Quake3ShaderToken::CBrace | Quake3ShaderToken::OBrace | Quake3ShaderToken::String);

  while (!token.hasType(Quake3ShaderToken::CBrace))
  {
    if (token.hasType(Quake3ShaderToken::OBrace))
    {
      parseStage(shader, status);
    }
    else
    {
      parseBodyEntry(shader, status);
    }
    token = m_tokenizer.skipAndPeekToken(Quake3ShaderToken::Eol);
  }
  m_tokenizer.skipAndNextToken(Quake3ShaderToken::Eol, Quake3ShaderToken::CBrace);
}

void Quake3ShaderParser::parseStage(Quake3Shader& shader, ParserStatus& status)
{
  m_tokenizer.skipAndNextToken(Quake3ShaderToken::Eol, Quake3ShaderToken::OBrace);
  auto token = m_tokenizer.skipAndPeekToken(
    Quake3ShaderToken::Eol,
    Quake3ShaderToken::CBrace | Quake3ShaderToken::OBrace | Quake3ShaderToken::String);

  auto& stage = shader.addStage();
  while (!token.hasType(Quake3ShaderToken::CBrace))
  {
    parseStageEntry(stage, status);
    token = m_tokenizer.skipAndPeekToken(Quake3ShaderToken::Eol);
  }
  m_tokenizer.skipAndNextToken(Quake3ShaderToken::Eol, Quake3ShaderToken::CBrace);
}

void Quake3ShaderParser::parseTexture(Quake3Shader& shader, ParserStatus& /* status */)
{
  const auto token =
    m_tokenizer.skipAndNextToken(Quake3ShaderToken::Eol, Quake3ShaderToken::String);
  const auto pathStr = token.data();
  if (!pathStr.empty() && pathStr[0] == '/')
  {
    // 2633: Q3 accepts absolute shader paths, so we just strip the leading slash
    shader.shaderPath = std::filesystem::path{pathStr.substr(1)};
  }
  else
  {
    shader.shaderPath = std::filesystem::path{token.data()};
  }
}

void Quake3ShaderParser::parseBodyEntry(Quake3Shader& shader, ParserStatus& /* status */)
{
  auto token =
    m_tokenizer.skipAndNextToken(Quake3ShaderToken::Eol, Quake3ShaderToken::String);

  const auto key = token.data();
  if (kdl::ci::str_is_equal(key, "qer_editorimage"))
  {
    token = m_tokenizer.nextToken(Quake3ShaderToken::String);
    shader.editorImage = std::filesystem::path{token.data()};
  }
  else if (kdl::ci::str_is_equal(key, "q3map_lightimage"))
  {
    token = m_tokenizer.nextToken(Quake3ShaderToken::String);
    shader.lightImage = std::filesystem::path{token.data()};
  }
  else if (kdl::ci::str_is_equal(key, "surfaceparm"))
  {
    token = m_tokenizer.nextToken(Quake3ShaderToken::String);
    shader.surfaceParms.insert(token.data());
  }
  else if (kdl::ci::str_is_equal(key, "cull"))
  {
    token = m_tokenizer.nextToken(Quake3ShaderToken::String);
    const auto value = token.data();
    if (kdl::ci::str_is_equal(value, "front"))
    {
      shader.culling = Quake3Shader::Culling::Front;
    }
    else if (kdl::ci::str_is_equal(value, "back"))
    {
      shader.culling = Quake3Shader::Culling::Back;
    }
    else if (
      kdl::ci::str_is_equal(value, "none") || kdl::ci::str_is_equal(value, "disable"))
    {
      shader.culling = Quake3Shader::Culling::None;
    }
  }
  else
  {
    skipRemainderOfEntry();
  }
}

void Quake3ShaderParser::parseStageEntry(Quake3ShaderStage& stage, ParserStatus& status)
{
  auto token =
    m_tokenizer.skipAndNextToken(Quake3ShaderToken::Eol, Quake3ShaderToken::String);

  const auto key = token.data();
  if (kdl::ci::str_is_equal(key, "map"))
  {
    token =
      m_tokenizer.nextToken(Quake3ShaderToken::String | Quake3ShaderToken::Variable);
    stage.map = std::filesystem::path{token.data()};
  }
  else if (kdl::ci::str_is_equal(key, "blendFunc"))
  {
    token = m_tokenizer.nextToken(Quake3ShaderToken::String);
    const auto param1 = token.data();
    const auto param1Location = token.location();

    if (m_tokenizer.peekToken().hasType(Quake3ShaderToken::String))
    {
      token = m_tokenizer.nextToken();
      const auto param2 = token.data();
      const auto param2Location = token.location();

      stage.blendFunc.srcFactor = kdl::str_to_upper(param1);
      stage.blendFunc.destFactor = kdl::str_to_upper(param2);

      auto valid = true;
      if (!stage.blendFunc.validateSrcFactor())
      {
        valid = false;
        status.warn(param1Location, "Unknown blendFunc source factor '" + param1 + "'");
      }
      if (!stage.blendFunc.validateDestFactor())
      {
        valid = false;
        status.warn(
          param2Location, "Unknown blendFunc destination factor '" + param2 + "'");
      }
      if (!valid)
      {
        stage.blendFunc.reset();
      }
    }
    else
    {
      if (kdl::ci::str_is_equal(param1, "add"))
      {
        stage.blendFunc.srcFactor = Quake3ShaderStage::BlendFunc::One;
        stage.blendFunc.destFactor = Quake3ShaderStage::BlendFunc::One;
      }
      else if (kdl::ci::str_is_equal(param1, "filter"))
      {
        stage.blendFunc.srcFactor = Quake3ShaderStage::BlendFunc::DestColor;
        stage.blendFunc.destFactor = Quake3ShaderStage::BlendFunc::Zero;
      }
      else if (kdl::ci::str_is_equal(param1, "blend"))
      {
        stage.blendFunc.srcFactor = Quake3ShaderStage::BlendFunc::SrcAlpha;
        stage.blendFunc.destFactor = Quake3ShaderStage::BlendFunc::OneMinusSrcAlpha;
      }
      else
      {
        status.warn(param1Location, "Unknown blendFunc name '" + param1 + "'");
      }
    }
  }
  else
  {
    skipRemainderOfEntry();
  }
}

void Quake3ShaderParser::skipRemainderOfEntry()
{
  auto token = m_tokenizer.peekToken();
  while (!token.hasType(Quake3ShaderToken::Eol | Quake3ShaderToken::CBrace))
  {
    m_tokenizer.nextToken();
    token = m_tokenizer.peekToken();
  }
  if (token.hasType(Quake3ShaderToken::Eol))
  {
    m_tokenizer.skipToken();
  }
}

} // namespace tb::mdl
