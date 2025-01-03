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

#include "StandardMapParser.h"

#include "FileLocation.h"
#include "io/ParserStatus.h"
#include "mdl/BrushFace.h"
#include "mdl/EntityProperties.h"

#include "vm/vec.h"

#include <string>
#include <vector>

namespace tb::io
{

const std::string& QuakeMapTokenizer::NumberDelim()
{
  static const std::string numberDelim(Whitespace() + ")");
  return numberDelim;
}

QuakeMapTokenizer::QuakeMapTokenizer(const std::string_view str)
  : Tokenizer{str, "\"", '\\'}
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
    const auto startLine = line();
    const auto startColumn = column();
    const auto startLocation = location();
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
          return Token{
            QuakeMapToken::Comment, c, c + 3, offset(c), startLine, startColumn};
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
      return Token{QuakeMapToken::OBrace, c, c + 1, offset(c), startLine, startColumn};
    case '}':
      advance();
      return Token{QuakeMapToken::CBrace, c, c + 1, offset(c), startLine, startColumn};
    case '(':
      advance();
      return Token{
        QuakeMapToken::OParenthesis, c, c + 1, offset(c), startLine, startColumn};
    case ')':
      advance();
      return Token{
        QuakeMapToken::CParenthesis, c, c + 1, offset(c), startLine, startColumn};
    case '[':
      advance();
      return Token{QuakeMapToken::OBracket, c, c + 1, offset(c), startLine, startColumn};
    case ']':
      advance();
      return Token{QuakeMapToken::CBracket, c, c + 1, offset(c), startLine, startColumn};
    case '"': { // quoted string
      advance();
      c = curPos();
      const auto* e = readQuotedString('"', "\n}");
      return Token{QuakeMapToken::String, c, e, offset(c), startLine, startColumn};
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
        return Token{QuakeMapToken::Eol, c, c + 1, offset(c), startLine, startColumn};
      }
      switchFallthrough();
    case ' ':
    case '\t':
      discardWhile(Whitespace());
      break;
    default: // whitespace, integer, decimal or word
      if (const auto* e = readInteger(NumberDelim()))
      {
        return Token{QuakeMapToken::Integer, c, e, offset(c), startLine, startColumn};
      }

      if (const auto e = readDecimal(NumberDelim()))
      {
        return Token{QuakeMapToken::Decimal, c, e, offset(c), startLine, startColumn};
      }

      if (const auto e = readUntil(Whitespace()))
      {
        return Token{QuakeMapToken::String, c, e, offset(c), startLine, startColumn};
      }

      throw ParserException{startLocation, fmt::format("Unexpected character: {}", *c)};
    }
  }
  return Token{QuakeMapToken::Eof, nullptr, nullptr, length(), line(), column()};
}

const std::string StandardMapParser::BrushPrimitiveId = "brushDef";
const std::string StandardMapParser::PatchId = "patchDef2";

StandardMapParser::StandardMapParser(
  const std::string_view str,
  const mdl::MapFormat sourceMapFormat,
  const mdl::MapFormat targetMapFormat)
  : m_tokenizer{str}
  , m_sourceMapFormat{sourceMapFormat}
  , m_targetMapFormat{targetMapFormat}
{
  assert(m_sourceMapFormat != mdl::MapFormat::Unknown);
  assert(targetMapFormat != mdl::MapFormat::Unknown);
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
    parseObject(status);
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
  auto token = m_tokenizer.nextToken();
  if (token.type() == QuakeMapToken::Eof)
  {
    return;
  }

  expect(QuakeMapToken::OBrace, token);
  const auto startLocation = token.location();

  auto properties = std::vector<mdl::EntityProperty>();
  auto propertyKeys = EntityPropertyKeys();
  parseEntityProperties(properties, propertyKeys, status);

  onBeginEntity(startLocation, properties, status);
  parseObjects(status);

  token = m_tokenizer.nextToken(QuakeMapToken::Comment);
  expect(QuakeMapToken::CBrace, token);

  onEndEntity(token.location(), status);
}

void StandardMapParser::parseEntityProperties(
  std::vector<mdl::EntityProperty>& properties,
  EntityPropertyKeys& keys,
  ParserStatus& status)
{
  auto token = m_tokenizer.peekToken(QuakeMapToken::Comment);
  while (token.hasType(QuakeMapToken::String))
  {
    parseEntityProperty(properties, keys, status);
    token = m_tokenizer.peekToken(QuakeMapToken::Comment);
  }
}

void StandardMapParser::parseEntityProperty(
  std::vector<mdl::EntityProperty>& properties,
  EntityPropertyKeys& keys,
  ParserStatus& status)
{
  auto token = m_tokenizer.nextToken(QuakeMapToken::Comment);
  assert(token.type() == QuakeMapToken::String);
  const auto name = token.data();

  const auto location = token.location();

  expect(QuakeMapToken::String, token = m_tokenizer.nextToken());
  const auto value = token.data();

  if (keys.count(name) == 0)
  {
    properties.emplace_back(name, value);
    keys.insert(name);
  }
  else
  {
    status.warn(location, fmt::format("Ignoring duplicate entity property '{}'", name));
  }
}

void StandardMapParser::parseObjects(ParserStatus& status)
{
  auto token = m_tokenizer.peekToken(QuakeMapToken::Comment);
  while (token.hasType(QuakeMapToken::OBrace))
  {
    parseObject(status);
    token = m_tokenizer.peekToken(QuakeMapToken::Comment);
  }
}

void StandardMapParser::parseObject(ParserStatus& status)
{
  // consume initial opening brace
  auto token = expect(
    QuakeMapToken::OBrace | QuakeMapToken::CBrace | QuakeMapToken::Eof,
    m_tokenizer.nextToken(QuakeMapToken::Comment));

  if (token.hasType(QuakeMapToken::Eof | QuakeMapToken::CBrace))
  {
    return;
  }

  const auto startLocation = token.location();

  token = m_tokenizer.peekToken();
  if (m_sourceMapFormat == mdl::MapFormat::Quake3)
  {
    // We expect either a brush primitive, a patch or a regular brush.
    expect(QuakeMapToken::String | QuakeMapToken::OParenthesis, token);
    if (token.hasType(QuakeMapToken::String))
    {
      expect({BrushPrimitiveId, PatchId}, token);
      if (token.data() == BrushPrimitiveId)
      {
        parseBrushPrimitive(status, startLocation);
      }
      else
      {
        parsePatch(status, startLocation);
      }
    }
    else
    {
      parseBrush(status, startLocation, false);
    }
  }
  else if (
    m_sourceMapFormat == mdl::MapFormat::Quake3_Valve
    || m_sourceMapFormat == mdl::MapFormat::Quake3_Legacy)
  {
    // We expect either a patch or a regular brush.
    expect(QuakeMapToken::String | QuakeMapToken::OParenthesis, token);
    if (token.hasType(QuakeMapToken::String))
    {
      expect(PatchId, token);
      parsePatch(status, startLocation);
    }
    else
    {
      parseBrush(status, startLocation, false);
    }
  }
  else
  {
    expect(QuakeMapToken::OParenthesis, token);
    parseBrush(status, startLocation, false);
  }

  // consume final closing brace
  expect(QuakeMapToken::CBrace, m_tokenizer.nextToken());
}

void StandardMapParser::parseBrushPrimitive(
  ParserStatus& status, const FileLocation& startLocation)
{
  auto token = expect(QuakeMapToken::String, m_tokenizer.nextToken());
  expect(BrushPrimitiveId, token);
  expect(QuakeMapToken::OBrace, m_tokenizer.nextToken());
  parseBrush(status, startLocation, true);
  expect(QuakeMapToken::CBrace, m_tokenizer.nextToken());
}

void StandardMapParser::parseBrush(
  ParserStatus& status, const FileLocation& startLocation, const bool primitive)
{
  auto beginBrushCalled = false;

  auto token = m_tokenizer.peekToken(QuakeMapToken::Comment);
  while (!token.hasType(QuakeMapToken::Eof))
  {
    switch (token.type())
    {
    case QuakeMapToken::OParenthesis:
      // TODO 2427: handle brush primitives
      if (!beginBrushCalled && !primitive)
      {
        onBeginBrush(startLocation, status);
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
          onBeginBrush(startLocation, status);
        }
        onEndBrush(token.location(), status);
      }
      else
      {
        status.warn(startLocation, "Skipping brush primitive: currently not supported");
      }
      return;
    default: {
      expect(QuakeMapToken::OParenthesis | QuakeMapToken::CParenthesis, token);
    }
    }

    token = m_tokenizer.peekToken(QuakeMapToken::Comment);
  }
}

void StandardMapParser::parseFace(ParserStatus& status, const bool primitive)
{
  switch (m_sourceMapFormat)
  {
  case mdl::MapFormat::Standard:
    parseQuakeFace(status);
    break;
  case mdl::MapFormat::Quake2:
  case mdl::MapFormat::Quake3_Legacy:
    parseQuake2Face(status);
    break;
  case mdl::MapFormat::Quake2_Valve:
  case mdl::MapFormat::Quake3_Valve:
    parseQuake2ValveFace(status);
    break;
  case mdl::MapFormat::Hexen2:
    parseHexen2Face(status);
    break;
  case mdl::MapFormat::Daikatana:
    parseDaikatanaFace(status);
    break;
  case mdl::MapFormat::Valve:
    parseValveFace(status);
    break;
  case mdl::MapFormat::Quake3:
    if (primitive)
    {
      parsePrimitiveFace(status);
    }
    else
    {
      parseQuake2Face(status);
    }
    break;
  case mdl::MapFormat::Unknown:
    // cannot happen
    break;
    switchDefault();
  }
}

void StandardMapParser::parseQuakeFace(ParserStatus& status)
{
  const auto location = m_tokenizer.location();

  const auto [p1, p2, p3] = parseFacePoints(status);
  const auto materialName = parseMaterialName(status);

  auto attribs = mdl::BrushFaceAttributes{materialName};
  attribs.setXOffset(parseFloat());
  attribs.setYOffset(parseFloat());
  attribs.setRotation(parseFloat());
  attribs.setXScale(parseFloat());
  attribs.setYScale(parseFloat());

  onStandardBrushFace(location, m_targetMapFormat, p1, p2, p3, attribs, status);
}

void StandardMapParser::parseQuake2Face(ParserStatus& status)
{
  const auto location = m_tokenizer.location();

  const auto [p1, p2, p3] = parseFacePoints(status);
  const auto materialName = parseMaterialName(status);

  auto attribs = mdl::BrushFaceAttributes{materialName};
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

  onStandardBrushFace(location, m_targetMapFormat, p1, p2, p3, attribs, status);
}

void StandardMapParser::parseQuake2ValveFace(ParserStatus& status)
{
  const auto location = m_tokenizer.location();

  const auto [p1, p2, p3] = parseFacePoints(status);
  const auto materialName = parseMaterialName(status);

  const auto [uAxis, uOffset, vAxis, vOffset] = parseValveUVAxes(status);

  auto attribs = mdl::BrushFaceAttributes{materialName};
  attribs.setXOffset(uOffset);
  attribs.setYOffset(vOffset);
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

  onValveBrushFace(
    location, m_targetMapFormat, p1, p2, p3, attribs, uAxis, vAxis, status);
}

void StandardMapParser::parseHexen2Face(ParserStatus& status)
{
  const auto location = m_tokenizer.location();

  const auto [p1, p2, p3] = parseFacePoints(status);
  const auto materialName = parseMaterialName(status);

  auto attribs = mdl::BrushFaceAttributes{materialName};
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

  onStandardBrushFace(location, m_targetMapFormat, p1, p2, p3, attribs, status);
}

void StandardMapParser::parseDaikatanaFace(ParserStatus& status)
{
  const auto location = m_tokenizer.location();

  const auto [p1, p2, p3] = parseFacePoints(status);
  const auto materialName = parseMaterialName(status);

  auto attribs = mdl::BrushFaceAttributes{materialName};
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
      attribs.setColor(Color{parseInteger(), parseInteger(), parseInteger()});
    }
  }

  onStandardBrushFace(location, m_targetMapFormat, p1, p2, p3, attribs, status);
}

void StandardMapParser::parseValveFace(ParserStatus& status)
{
  const auto location = m_tokenizer.location();

  const auto [p1, p2, p3] = parseFacePoints(status);
  const auto materialName = parseMaterialName(status);

  const auto [uAxis, uOffset, vAxis, vOffset] = parseValveUVAxes(status);

  auto attribs = mdl::BrushFaceAttributes{materialName};
  attribs.setXOffset(uOffset);
  attribs.setYOffset(vOffset);
  attribs.setRotation(parseFloat());
  attribs.setXScale(parseFloat());
  attribs.setYScale(parseFloat());

  onValveBrushFace(
    location, m_targetMapFormat, p1, p2, p3, attribs, uAxis, vAxis, status);
}

void StandardMapParser::parsePrimitiveFace(ParserStatus& status)
{
  /* const auto line = */ m_tokenizer.line();

  /* const auto [p1, p2, p3] = */ parseFacePoints(status);

  expect(QuakeMapToken::OParenthesis, m_tokenizer.nextToken());

  /* const auto [uAxis, vAxis] = */ parsePrimitiveUVAxes(status);
  expect(QuakeMapToken::CParenthesis, m_tokenizer.nextToken());

  const auto materialName = parseMaterialName(status);

  // TODO 2427: what to set for offset, rotation, scale?!
  auto attribs = mdl::BrushFaceAttributes{materialName};

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
  // brushFace(line, p1, p2, p3, attribs, uAxis, vAxis, status);
}

void StandardMapParser::parsePatch(
  ParserStatus& status, const FileLocation& startLocation)
{
  auto token = expect(QuakeMapToken::String, m_tokenizer.nextToken());
  expect(PatchId, token);
  expect(QuakeMapToken::OBrace, m_tokenizer.nextToken());

  auto materialName = parseMaterialName(status);
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
    status.warn(token.location(), "Invalid patch height, assuming 3");
    rowCountInt = 3;
  }

  token = expect(QuakeMapToken::Integer, m_tokenizer.nextToken());
  int columnCountInt = token.toInteger<int>();
  if (columnCountInt < 3 || columnCountInt % 2 != 1)
  {
    status.warn(token.location(), "Invalid patch width, assuming 3");
    columnCountInt = 3;
  }

  const auto rowCount = static_cast<size_t>(rowCountInt);
  const auto columnCount = static_cast<size_t>(columnCountInt);

  expect(QuakeMapToken::Integer, m_tokenizer.nextToken());
  expect(QuakeMapToken::Integer, m_tokenizer.nextToken());
  expect(QuakeMapToken::Integer, m_tokenizer.nextToken());
  expect(QuakeMapToken::CParenthesis, m_tokenizer.nextToken());

  auto controlPoints = std::vector<vm::vec<double, 5>>{};
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
  onPatch(
    startLocation,
    token.location(),
    m_targetMapFormat,
    rowCount,
    columnCount,
    std::move(controlPoints),
    std::move(materialName),
    status);
}

std::tuple<vm::vec3d, vm::vec3d, vm::vec3d> StandardMapParser::parseFacePoints(
  ParserStatus& /* status */)
{
  const auto p1 =
    correct(parseFloatVector(QuakeMapToken::OParenthesis, QuakeMapToken::CParenthesis));
  const auto p2 =
    correct(parseFloatVector(QuakeMapToken::OParenthesis, QuakeMapToken::CParenthesis));
  const auto p3 =
    correct(parseFloatVector(QuakeMapToken::OParenthesis, QuakeMapToken::CParenthesis));

  return {p1, p2, p3};
}

std::string StandardMapParser::parseMaterialName(ParserStatus& /* status */)
{
  const auto [materialName, wasQuoted] =
    m_tokenizer.readAnyString(QuakeMapTokenizer::Whitespace());
  return wasQuoted ? kdl::str_unescape(materialName, "\"\\") : std::string{materialName};
}

std::tuple<vm::vec3d, float, vm::vec3d, float> StandardMapParser::parseValveUVAxes(
  ParserStatus& /* status */)
{
  const auto firstAxis =
    parseFloatVector<4>(QuakeMapToken::OBracket, QuakeMapToken::CBracket);
  const auto uAxis = firstAxis.xyz();
  const auto uOffset = static_cast<float>(firstAxis.w());

  const auto secondAxis =
    parseFloatVector<4>(QuakeMapToken::OBracket, QuakeMapToken::CBracket);
  const auto vAxis = secondAxis.xyz();
  const auto vOffset = static_cast<float>(secondAxis.w());

  return {uAxis, uOffset, vAxis, vOffset};
}

std::tuple<vm::vec3d, vm::vec3d> StandardMapParser::parsePrimitiveUVAxes(
  ParserStatus& /* status */)
{
  const auto uAxis =
    correct(parseFloatVector(QuakeMapToken::OParenthesis, QuakeMapToken::CParenthesis));
  const auto vAxis =
    correct(parseFloatVector(QuakeMapToken::OParenthesis, QuakeMapToken::CParenthesis));
  return {uAxis, vAxis};
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

  return {
    {Integer, "integer"},
    {Decimal, "decimal"},
    {String, "string"},
    {OParenthesis, "'('"},
    {CParenthesis, "')'"},
    {OBrace, "'{'"},
    {CBrace, "'}'"},
    {OBracket, "'['"},
    {CBracket, "']'"},
    {Comment, "comment"},
    {Eof, "end of file"},
  };
}

} // namespace tb::io
