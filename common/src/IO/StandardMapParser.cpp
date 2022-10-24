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

#include "StandardMapParser.h"

#include "IO/ParserStatus.h"
#include "Model/BrushFace.h"
#include "Model/EntityProperties.h"

#include <kdl/invoke.h>
#include <kdl/vector_set.h>

#include <vecmath/plane.h>
#include <vecmath/vec.h>

#include <string>
#include <vector>

namespace TrenchBroom
{
namespace IO
{
const std::string& QuakeMapTokenizer::NumberDelim()
{
  static const std::string numberDelim(Whitespace() + ")");
  return numberDelim;
}

QuakeMapTokenizer::QuakeMapTokenizer(std::string_view str)
  : Tokenizer(std::move(str), "\"", '\\')
  , m_skipEol(true)
{
}

void QuakeMapTokenizer::setSkipEol(bool skipEol)
{
  m_skipEol = skipEol;
}

QuakeMapTokenizer::Token QuakeMapTokenizer::emitToken()
{
  while (!eof())
  {
    auto startLine = line();
    auto startColumn = column();
    const auto* c = curPos();
    switch (*c)
    {
    case '/':
      advance();
      if (curChar() == '/')
      {
        advance();
        if (curChar() == '/' && lookAhead(1) == ' ')
        {
          advance();
          return Token(
            QuakeMapToken::Comment, c, c + 3, offset(c), startLine, startColumn);
        }
        discardUntil("\n\r");
      }
      break;
    case ';':
      // Heretic2 allows semicolon to start a line comment.
      // QuArK writes comments in this format when saving a Heretic2 .map.
      advance();
      discardUntil("\n\r");
      break;
    case '{':
      advance();
      return Token(QuakeMapToken::OBrace, c, c + 1, offset(c), startLine, startColumn);
    case '}':
      advance();
      return Token(QuakeMapToken::CBrace, c, c + 1, offset(c), startLine, startColumn);
    case '(':
      advance();
      return Token(
        QuakeMapToken::OParenthesis, c, c + 1, offset(c), startLine, startColumn);
    case ')':
      advance();
      return Token(
        QuakeMapToken::CParenthesis, c, c + 1, offset(c), startLine, startColumn);
    case '[':
      advance();
      return Token(QuakeMapToken::OBracket, c, c + 1, offset(c), startLine, startColumn);
    case ']':
      advance();
      return Token(QuakeMapToken::CBracket, c, c + 1, offset(c), startLine, startColumn);
    case '"': { // quoted string
      advance();
      c = curPos();
      const auto* e = readQuotedString('"', "\n}");
      return Token(QuakeMapToken::String, c, e, offset(c), startLine, startColumn);
    }
    case '\r':
      if (lookAhead() == '\n')
      {
        advance();
      }
      // handle carriage return without consecutive linefeed
      // by falling through into the line feed case
      switchFallthrough();
    case '\n':
      if (!m_skipEol)
      {
        advance();
        return Token(QuakeMapToken::Eol, c, c + 1, offset(c), startLine, startColumn);
      }
      switchFallthrough();
    case ' ':
    case '\t':
      discardWhile(Whitespace());
      break;
    default: { // whitespace, integer, decimal or word
      const auto* e = readInteger(NumberDelim());
      if (e != nullptr)
      {
        return Token(QuakeMapToken::Integer, c, e, offset(c), startLine, startColumn);
      }

      e = readDecimal(NumberDelim());
      if (e != nullptr)
      {
        return Token(QuakeMapToken::Decimal, c, e, offset(c), startLine, startColumn);
      }

      e = readUntil(Whitespace());
      if (e == nullptr)
      {
        throw ParserException(
          startLine, startColumn, "Unexpected character: " + std::string(c, 1));
      }

      return Token(QuakeMapToken::String, c, e, offset(c), startLine, startColumn);
    }
    }
  }
  return Token(QuakeMapToken::Eof, nullptr, nullptr, length(), line(), column());
}

const std::string StandardMapParser::BrushPrimitiveId = "brushDef";
const std::string StandardMapParser::PatchId = "patchDef2";

StandardMapParser::StandardMapParser(
  std::string_view str,
  const Model::MapFormat sourceMapFormat,
  const Model::MapFormat targetMapFormat)
  : m_tokenizer(QuakeMapTokenizer(std::move(str)))
  , m_sourceMapFormat(sourceMapFormat)
  , m_targetMapFormat(targetMapFormat)
{
  assert(m_sourceMapFormat != Model::MapFormat::Unknown);
  assert(targetMapFormat != Model::MapFormat::Unknown);
}

StandardMapParser::~StandardMapParser() = default;

void StandardMapParser::parseEntities(ParserStatus& status)
{
  auto token = m_tokenizer.peekToken();
  while (token.type() != QuakeMapToken::Eof)
  {
    expect(QuakeMapToken::OBrace, token);
    parseEntity(status);
    token = m_tokenizer.peekToken();
  }
}

void StandardMapParser::parseBrushesOrPatches(ParserStatus& status)
{
  auto token = m_tokenizer.peekToken();
  while (token.type() != QuakeMapToken::Eof)
  {
    expect(QuakeMapToken::OBrace, token);
    parseBrushOrBrushPrimitiveOrPatch(status);
    token = m_tokenizer.peekToken();
  }
}

void StandardMapParser::parseBrushFaces(ParserStatus& status)
{
  auto token = m_tokenizer.peekToken();
  while (token.type() != QuakeMapToken::Eof)
  {
    expect(QuakeMapToken::OParenthesis, token);
    // TODO 2427: detect the face type when parsing Quake3 map faces!
    parseFace(status, false);
    token = m_tokenizer.peekToken();
  }
}

void StandardMapParser::reset()
{
  m_tokenizer.reset();
}

void StandardMapParser::parseEntity(ParserStatus& status)
{
  Token token = m_tokenizer.nextToken();
  if (token.type() == QuakeMapToken::Eof)
  {
    return;
  }

  expect(QuakeMapToken::OBrace, token);

  auto beginEntityCalled = false;

  auto properties = std::vector<Model::EntityProperty>();
  auto propertyKeys = EntityPropertyKeys();

  const auto startLine = token.line();

  token = m_tokenizer.peekToken();
  while (token.type() != QuakeMapToken::Eof)
  {
    switch (token.type())
    {
    case QuakeMapToken::Comment:
      m_tokenizer.nextToken();
      break;
    case QuakeMapToken::String:
      parseEntityProperty(properties, propertyKeys, status);
      break;
    case QuakeMapToken::OBrace:
      if (!beginEntityCalled)
      {
        onBeginEntity(startLine, std::move(properties), status);
        beginEntityCalled = true;
      }
      parseBrushOrBrushPrimitiveOrPatch(status);
      break;
    case QuakeMapToken::CBrace:
      m_tokenizer.nextToken();
      if (!beginEntityCalled)
      {
        onBeginEntity(startLine, properties, status);
      }
      onEndEntity(startLine, token.line() - startLine, status);
      return;
    default:
      expect(
        QuakeMapToken::Comment | QuakeMapToken::String | QuakeMapToken::OBrace
          | QuakeMapToken::CBrace,
        token);
    }

    token = m_tokenizer.peekToken();
  }
}

void StandardMapParser::parseEntityProperty(
  std::vector<Model::EntityProperty>& properties,
  EntityPropertyKeys& keys,
  ParserStatus& status)
{
  auto token = m_tokenizer.nextToken();
  assert(token.type() == QuakeMapToken::String);
  const auto name = token.data();

  const auto line = token.line();
  const auto column = token.column();

  expect(QuakeMapToken::String, token = m_tokenizer.nextToken());
  const auto value = token.data();

  if (keys.count(name) == 0)
  {
    properties.push_back(Model::EntityProperty(name, value));
    keys.insert(name);
  }
  else
  {
    status.warn(line, column, "Ignoring duplicate entity property '" + name + "'");
  }
}

void StandardMapParser::parseBrushOrBrushPrimitiveOrPatch(ParserStatus& status)
{
  // consume initial opening brace
  auto token = expect(
    QuakeMapToken::OBrace | QuakeMapToken::CBrace | QuakeMapToken::Eof,
    m_tokenizer.nextToken());

  if (token.hasType(QuakeMapToken::Eof | QuakeMapToken::CBrace))
  {
    return;
  }

  const auto startLine = token.line();

  token = m_tokenizer.peekToken();
  if (m_sourceMapFormat == Model::MapFormat::Quake3)
  {
    // We expect either a brush primitive, a patch or a regular brush.
    expect(QuakeMapToken::String | QuakeMapToken::OParenthesis, token);
    if (token.hasType(QuakeMapToken::String))
    {
      expect(std::vector<std::string>({BrushPrimitiveId, PatchId}), token);
      if (token.data() == BrushPrimitiveId)
      {
        parseBrushPrimitive(status, startLine);
      }
      else
      {
        parsePatch(status, startLine);
      }
    }
    else
    {
      parseBrush(status, startLine, false);
    }
  }
  else if (
    m_sourceMapFormat == Model::MapFormat::Quake3_Valve
    || m_sourceMapFormat == Model::MapFormat::Quake3_Legacy)
  {
    // We expect either a patch or a regular brush.
    expect(QuakeMapToken::String | QuakeMapToken::OParenthesis, token);
    if (token.hasType(QuakeMapToken::String))
    {
      expect(PatchId, token);
      parsePatch(status, startLine);
    }
    else
    {
      parseBrush(status, startLine, false);
    }
  }
  else
  {
    expect(QuakeMapToken::OParenthesis, token);
    parseBrush(status, startLine, false);
  }

  // consume final closing brace
  expect(QuakeMapToken::CBrace, m_tokenizer.nextToken());
}

void StandardMapParser::parseBrushPrimitive(ParserStatus& status, const size_t startLine)
{
  auto token = expect(QuakeMapToken::String, m_tokenizer.nextToken());
  expect(BrushPrimitiveId, token);
  expect(QuakeMapToken::OBrace, m_tokenizer.nextToken());
  parseBrush(status, startLine, true);
  expect(QuakeMapToken::CBrace, m_tokenizer.nextToken());
}

void StandardMapParser::parseBrush(
  ParserStatus& status, const size_t startLine, const bool primitive)
{
  auto beginBrushCalled = false;

  auto token = m_tokenizer.peekToken();
  while (!token.hasType(QuakeMapToken::Eof))
  {
    switch (token.type())
    {
    case QuakeMapToken::Comment:
      m_tokenizer.nextToken();
      break;
    case QuakeMapToken::OParenthesis:
      // TODO 2427: handle brush primitives
      if (!beginBrushCalled && !primitive)
      {
        onBeginBrush(startLine, status);
        beginBrushCalled = true;
      }
      parseFace(status, primitive);
      break;
    case QuakeMapToken::CBrace:
      // TODO 2427: handle brush primitives
      if (!primitive)
      {
        if (!beginBrushCalled)
        {
          onBeginBrush(startLine, status);
        }
        onEndBrush(startLine, token.line() - startLine, status);
      }
      else
      {
        status.warn(startLine, "Skipping brush primitive: currently not supported");
      }
      return;
    default: {
      expect(QuakeMapToken::OParenthesis | QuakeMapToken::CParenthesis, token);
    }
    }

    token = m_tokenizer.peekToken();
  }
}

void StandardMapParser::parseFace(ParserStatus& status, const bool primitive)
{
  switch (m_sourceMapFormat)
  {
  case Model::MapFormat::Standard:
    parseQuakeFace(status);
    break;
  case Model::MapFormat::Quake2:
  case Model::MapFormat::Quake3_Legacy:
    parseQuake2Face(status);
    break;
  case Model::MapFormat::Quake2_Valve:
  case Model::MapFormat::Quake3_Valve:
    parseQuake2ValveFace(status);
    break;
  case Model::MapFormat::Hexen2:
    parseHexen2Face(status);
    break;
  case Model::MapFormat::Daikatana:
    parseDaikatanaFace(status);
    break;
  case Model::MapFormat::Valve:
    parseValveFace(status);
    break;
  case Model::MapFormat::Quake3:
    if (primitive)
    {
      parsePrimitiveFace(status);
    }
    else
    {
      parseQuake2Face(status);
    }
    break;
  case Model::MapFormat::Unknown:
    // cannot happen
    break;
    switchDefault();
  }
}

void StandardMapParser::parseQuakeFace(ParserStatus& status)
{
  const auto line = m_tokenizer.line();

  const auto [p1, p2, p3] = parseFacePoints(status);
  const auto textureName = parseTextureName(status);

  auto attribs = Model::BrushFaceAttributes(textureName);
  attribs.setXOffset(parseFloat());
  attribs.setYOffset(parseFloat());
  attribs.setRotation(parseFloat());
  attribs.setXScale(parseFloat());
  attribs.setYScale(parseFloat());

  onStandardBrushFace(line, m_targetMapFormat, p1, p2, p3, attribs, status);
}

void StandardMapParser::parseQuake2Face(ParserStatus& status)
{
  const auto line = m_tokenizer.line();

  const auto [p1, p2, p3] = parseFacePoints(status);
  const auto textureName = parseTextureName(status);

  auto attribs = Model::BrushFaceAttributes(textureName);
  attribs.setXOffset(parseFloat());
  attribs.setYOffset(parseFloat());
  attribs.setRotation(parseFloat());
  attribs.setXScale(parseFloat());
  attribs.setYScale(parseFloat());

  // Quake 2 extra info is optional
  if (!check(
        QuakeMapToken::OParenthesis | QuakeMapToken::CBrace | QuakeMapToken::Eof,
        m_tokenizer.peekToken()))
  {
    attribs.setSurfaceContents(parseInteger());
    attribs.setSurfaceFlags(parseInteger());
    attribs.setSurfaceValue(parseFloat());
  }

  onStandardBrushFace(line, m_targetMapFormat, p1, p2, p3, attribs, status);
}

void StandardMapParser::parseQuake2ValveFace(ParserStatus& status)
{
  const auto line = m_tokenizer.line();

  const auto [p1, p2, p3] = parseFacePoints(status);
  const auto textureName = parseTextureName(status);

  const auto [texX, xOffset, texY, yOffset] = parseValveTextureAxes(status);

  auto attribs = Model::BrushFaceAttributes(textureName);
  attribs.setXOffset(xOffset);
  attribs.setYOffset(yOffset);
  attribs.setRotation(parseFloat());
  attribs.setXScale(parseFloat());
  attribs.setYScale(parseFloat());

  // Quake 2 extra info is optional
  if (!check(
        QuakeMapToken::OParenthesis | QuakeMapToken::CBrace | QuakeMapToken::Eof,
        m_tokenizer.peekToken()))
  {
    attribs.setSurfaceContents(parseInteger());
    attribs.setSurfaceFlags(parseInteger());
    attribs.setSurfaceValue(parseFloat());
  }

  onValveBrushFace(line, m_targetMapFormat, p1, p2, p3, attribs, texX, texY, status);
}

void StandardMapParser::parseHexen2Face(ParserStatus& status)
{
  const auto line = m_tokenizer.line();

  const auto [p1, p2, p3] = parseFacePoints(status);
  const auto textureName = parseTextureName(status);

  auto attribs = Model::BrushFaceAttributes(textureName);
  attribs.setXOffset(parseFloat());
  attribs.setYOffset(parseFloat());
  attribs.setRotation(parseFloat());
  attribs.setXScale(parseFloat());
  attribs.setYScale(parseFloat());

  // Hexen 2 extra info is optional
  if (!check(
        QuakeMapToken::OParenthesis | QuakeMapToken::CBrace | QuakeMapToken::Eof,
        m_tokenizer.peekToken()))
  {
    m_tokenizer.nextToken(); // noone seems to know what the extra value does in Hexen 2
  }

  onStandardBrushFace(line, m_targetMapFormat, p1, p2, p3, attribs, status);
}

void StandardMapParser::parseDaikatanaFace(ParserStatus& status)
{
  const auto line = m_tokenizer.line();

  const auto [p1, p2, p3] = parseFacePoints(status);
  const auto textureName = parseTextureName(status);

  auto attribs = Model::BrushFaceAttributes(textureName);
  attribs.setXOffset(parseFloat());
  attribs.setYOffset(parseFloat());
  attribs.setRotation(parseFloat());
  attribs.setXScale(parseFloat());
  attribs.setYScale(parseFloat());

  // Daikatana extra info is optional
  if (check(QuakeMapToken::Integer, m_tokenizer.peekToken()))
  {
    attribs.setSurfaceContents(parseInteger());
    attribs.setSurfaceFlags(parseInteger());
    attribs.setSurfaceValue(parseFloat());

    // Daikatana color triple is optional
    if (check(QuakeMapToken::Integer, m_tokenizer.peekToken()))
    {
      // red, green, blue
      attribs.setColor(Color(parseInteger(), parseInteger(), parseInteger()));
    }
  }

  onStandardBrushFace(line, m_targetMapFormat, p1, p2, p3, attribs, status);
}

void StandardMapParser::parseValveFace(ParserStatus& status)
{
  const auto line = m_tokenizer.line();

  const auto [p1, p2, p3] = parseFacePoints(status);
  const auto textureName = parseTextureName(status);

  const auto [texX, xOffset, texY, yOffset] = parseValveTextureAxes(status);

  auto attribs = Model::BrushFaceAttributes(textureName);
  attribs.setXOffset(xOffset);
  attribs.setYOffset(yOffset);
  attribs.setRotation(parseFloat());
  attribs.setXScale(parseFloat());
  attribs.setYScale(parseFloat());

  onValveBrushFace(line, m_targetMapFormat, p1, p2, p3, attribs, texX, texY, status);
}

void StandardMapParser::parsePrimitiveFace(ParserStatus& status)
{
  /* const auto line = */ m_tokenizer.line();

  /* const auto [p1, p2, p3] = */ parseFacePoints(status);

  expect(QuakeMapToken::OParenthesis, m_tokenizer.nextToken());

  /* const auto [texX, texY] = */ parsePrimitiveTextureAxes(status);
  expect(QuakeMapToken::CParenthesis, m_tokenizer.nextToken());

  const auto textureName = parseTextureName(status);

  // TODO 2427: what to set for offset, rotation, scale?!
  auto attribs = Model::BrushFaceAttributes(textureName);

  // Quake 2 extra info is optional
  if (!check(
        QuakeMapToken::OParenthesis | QuakeMapToken::CBrace | QuakeMapToken::Eof,
        m_tokenizer.peekToken()))
  {
    attribs.setSurfaceContents(parseInteger());
    attribs.setSurfaceFlags(parseInteger());
    attribs.setSurfaceValue(parseFloat());
  }

  // TODO 2427: create a brush face
  // brushFace(line, p1, p2, p3, attribs, texX, texY, status);
}

void StandardMapParser::parsePatch(ParserStatus& status, const size_t startLine)
{
  auto token = expect(QuakeMapToken::String, m_tokenizer.nextToken());
  expect(PatchId, token);
  expect(QuakeMapToken::OBrace, m_tokenizer.nextToken());

  auto textureName = parseTextureName(status);
  expect(QuakeMapToken::OParenthesis, m_tokenizer.nextToken());

  /*
  Quake 3 parses the patches a bit differently. In the GtkRadiant source, the first number
  is the column count and the second is the row count, and the points are transposed
  during parsing. Later, when the points are interpreted, radiant puts the origin (the
  first control point) in the bottom left, but we put it in the top left. For the grid
  computed from the this makes no difference as long as the normals are computed
  correctly.

  I chose to interpret the data this way because it seems more intuitive and easier to
  reason about.
  */

  token = expect(QuakeMapToken::Integer, m_tokenizer.nextToken());
  int rowCountInt = token.toInteger<int>();
  if (rowCountInt < 3 || rowCountInt % 2 != 1)
  {
    status.warn(token.line(), token.column(), "Invalid patch height, assuming 3");
    rowCountInt = 3;
  }

  token = expect(QuakeMapToken::Integer, m_tokenizer.nextToken());
  int columnCountInt = token.toInteger<int>();
  if (columnCountInt < 3 || columnCountInt % 2 != 1)
  {
    status.warn(token.line(), token.column(), "Invalid patch width, assuming 3");
    columnCountInt = 3;
  }

  size_t rowCount = static_cast<size_t>(rowCountInt);
  size_t columnCount = static_cast<size_t>(columnCountInt);

  expect(QuakeMapToken::Integer, m_tokenizer.nextToken());
  expect(QuakeMapToken::Integer, m_tokenizer.nextToken());
  expect(QuakeMapToken::Integer, m_tokenizer.nextToken());
  expect(QuakeMapToken::CParenthesis, m_tokenizer.nextToken());

  auto controlPoints = std::vector<vm::vec<FloatType, 5>>{};
  controlPoints.reserve(columnCount * rowCount);

  expect(QuakeMapToken::OParenthesis, m_tokenizer.nextToken());
  for (size_t i = 0; i < size_t(rowCount); ++i)
  {
    expect(QuakeMapToken::OParenthesis, m_tokenizer.nextToken());
    for (size_t j = 0; j < size_t(columnCount); ++j)
    {
      const auto controlPoint =
        parseFloatVector<5>(QuakeMapToken::OParenthesis, QuakeMapToken::CParenthesis);
      controlPoints.push_back(controlPoint);
    }
    expect(QuakeMapToken::CParenthesis, m_tokenizer.nextToken());
  }
  expect(QuakeMapToken::CParenthesis, m_tokenizer.nextToken());

  token = expect(QuakeMapToken::CBrace, m_tokenizer.nextToken());
  const size_t lineCount = token.line() - startLine;

  onPatch(
    startLine,
    lineCount,
    m_targetMapFormat,
    rowCount,
    columnCount,
    std::move(controlPoints),
    std::move(textureName),
    status);
}

std::tuple<vm::vec3, vm::vec3, vm::vec3> StandardMapParser::parseFacePoints(
  ParserStatus& /* status */)
{
  const auto p1 =
    correct(parseFloatVector(QuakeMapToken::OParenthesis, QuakeMapToken::CParenthesis));
  const auto p2 =
    correct(parseFloatVector(QuakeMapToken::OParenthesis, QuakeMapToken::CParenthesis));
  const auto p3 =
    correct(parseFloatVector(QuakeMapToken::OParenthesis, QuakeMapToken::CParenthesis));

  return std::make_tuple(p1, p2, p3);
}

std::string StandardMapParser::parseTextureName(ParserStatus& /* status */)
{
  const auto [textureName, wasQuoted] =
    m_tokenizer.readAnyString(QuakeMapTokenizer::Whitespace());
  return wasQuoted ? kdl::str_unescape(textureName, "\"\\") : std::string(textureName);
}

std::tuple<vm::vec3, float, vm::vec3, float> StandardMapParser::parseValveTextureAxes(
  ParserStatus& /* status */)
{
  const auto firstAxis =
    parseFloatVector<4>(QuakeMapToken::OBracket, QuakeMapToken::CBracket);
  const auto texS = firstAxis.xyz();
  const auto xOffset = static_cast<float>(firstAxis.w());

  const auto secondAxis =
    parseFloatVector<4>(QuakeMapToken::OBracket, QuakeMapToken::CBracket);
  const auto texT = secondAxis.xyz();
  const auto yOffset = static_cast<float>(secondAxis.w());

  return std::make_tuple(texS, xOffset, texT, yOffset);
}

std::tuple<vm::vec3, vm::vec3> StandardMapParser::parsePrimitiveTextureAxes(
  ParserStatus& /* status */)
{
  const auto texX =
    correct(parseFloatVector(QuakeMapToken::OParenthesis, QuakeMapToken::CParenthesis));
  const auto texY =
    correct(parseFloatVector(QuakeMapToken::OParenthesis, QuakeMapToken::CParenthesis));
  return std::make_tuple(texX, texY);
}

float StandardMapParser::parseFloat()
{
  return expect(QuakeMapToken::Number, m_tokenizer.nextToken()).toFloat<float>();
}

int StandardMapParser::parseInteger()
{
  return expect(QuakeMapToken::Integer, m_tokenizer.nextToken()).toInteger<int>();
}

StandardMapParser::TokenNameMap StandardMapParser::tokenNames() const
{
  using namespace QuakeMapToken;

  TokenNameMap names;
  names[Integer] = "integer";
  names[Decimal] = "decimal";
  names[String] = "string";
  names[OParenthesis] = "'('";
  names[CParenthesis] = "')'";
  names[OBrace] = "'{'";
  names[CBrace] = "'}'";
  names[OBracket] = "'['";
  names[CBracket] = "']'";
  names[Comment] = "comment";
  names[Eof] = "end of file";
  return names;
}
} // namespace IO
} // namespace TrenchBroom
