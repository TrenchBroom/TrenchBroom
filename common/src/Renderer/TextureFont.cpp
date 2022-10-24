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

#include "TextureFont.h"

#include "AttrString.h"
#include "Renderer/FontGlyph.h"
#include "Renderer/FontTexture.h"

#include <kdl/vector_utils.h>

#include <vecmath/forward.h>
#include <vecmath/vec.h>

#include <string>

namespace TrenchBroom
{
namespace Renderer
{
TextureFont::TextureFont(
  std::unique_ptr<FontTexture> texture,
  const std::vector<FontGlyph>& glyphs,
  const int lineHeight,
  const unsigned char firstChar,
  const unsigned char charCount)
  : m_texture(std::move(texture))
  , m_glyphs(glyphs)
  , m_lineHeight(lineHeight)
  , m_firstChar(firstChar)
  , m_charCount(charCount)
{
}

TextureFont::~TextureFont() = default;

class MeasureString : public AttrString::LineFunc
{
private:
  const TextureFont& m_font;
  vm::vec2f m_size;

public:
  explicit MeasureString(const TextureFont& font)
    : m_font(font)
  {
  }

  const vm::vec2f& size() const { return m_size; }

private:
  void justifyLeft(const std::string& str) override { measure(str); }

  void justifyRight(const std::string& str) override { measure(str); }

  void center(const std::string& str) override { measure(str); }

  void measure(const std::string& str)
  {
    const auto size = m_font.measure(str);
    m_size[0] = std::max(m_size[0], size[0]);
    m_size[1] += size[1];
  }
};

class MeasureLines : public AttrString::LineFunc
{
private:
  const TextureFont& m_font;
  std::vector<vm::vec2f> m_sizes;

public:
  explicit MeasureLines(const TextureFont& font)
    : m_font(font)
  {
  }

  const std::vector<vm::vec2f>& sizes() const { return m_sizes; }

private:
  void justifyLeft(const std::string& str) override { measure(str); }

  void justifyRight(const std::string& str) override { measure(str); }

  void center(const std::string& str) override { measure(str); }

  void measure(const std::string& str) { m_sizes.push_back(m_font.measure(str)); }
};

class MakeQuads : public AttrString::LineFunc
{
private:
  const TextureFont& m_font;

  bool m_clockwise;
  vm::vec2f m_offset;

  const std::vector<vm::vec2f>& m_sizes;
  vm::vec2f m_maxSize;

  size_t m_index;
  float m_y;
  std::vector<vm::vec2f> m_vertices;

public:
  MakeQuads(
    const TextureFont& font,
    const bool clockwise,
    const vm::vec2f& offset,
    const std::vector<vm::vec2f>& sizes)
    : m_font(font)
    , m_clockwise(clockwise)
    , m_offset(offset)
    , m_sizes(sizes)
    , m_index(0)
    , m_y(0.0f)
  {
    for (size_t i = 0; i < m_sizes.size(); ++i)
    {
      m_maxSize = max(m_maxSize, m_sizes[i]);
      m_y += m_sizes[i].y();
    }
    m_y -= m_sizes.back().y();
  }

  const std::vector<vm::vec2f>& vertices() const { return m_vertices; }

private:
  void justifyLeft(const std::string& str) override { makeQuads(str, 0.0f); }

  void justifyRight(const std::string& str) override
  {
    const auto w = m_sizes[m_index].x();
    makeQuads(str, m_maxSize.x() - w);
  }

  void center(const std::string& str) override
  {
    const auto w = m_sizes[m_index].x();
    makeQuads(str, (m_maxSize.x() - w) / 2.0f);
  }

  void makeQuads(const std::string& str, const float x)
  {
    const auto offset = m_offset + vm::vec2f(x, m_y);
    m_vertices =
      kdl::vec_concat(std::move(m_vertices), m_font.quads(str, m_clockwise, offset));

    m_y -= m_sizes[m_index].y();
    m_index++;
  }
};

std::vector<vm::vec2f> TextureFont::quads(
  const AttrString& string, const bool clockwise, const vm::vec2f& offset) const
{
  MeasureLines measureLines(*this);
  string.lines(measureLines);
  const auto& sizes = measureLines.sizes();

  MakeQuads makeQuads(*this, clockwise, offset, sizes);
  string.lines(makeQuads);
  return makeQuads.vertices();
}

vm::vec2f TextureFont::measure(const AttrString& string) const
{
  MeasureString measureString(*this);
  string.lines(measureString);
  return measureString.size();
}

std::vector<vm::vec2f> TextureFont::quads(
  const std::string& string, const bool clockwise, const vm::vec2f& offset) const
{
  std::vector<vm::vec2f> result;
  result.reserve(string.length() * 4 * 2);

  auto x = static_cast<int>(vm::round(offset.x()));
  auto y = static_cast<int>(vm::round(offset.y()));
  for (size_t i = 0; i < string.length(); i++)
  {
    auto c = string[i];
    if (c == '\n')
    {
      x = 0;
      y += m_lineHeight;
      continue;
    }

    if (c < m_firstChar || c >= m_firstChar + m_charCount)
    {
      c = ' '; // space
    }

    const auto& glyph = m_glyphs[static_cast<size_t>(c - m_firstChar)];
    if (c != ' ')
    {
      glyph.appendVertices(result, x, y, m_texture->size(), clockwise);
    }

    x += glyph.advance();
  }
  return result;
}

vm::vec2f TextureFont::measure(const std::string& string) const
{
  vm::vec2f result;

  int x = 0;
  int y = 0;
  for (size_t i = 0; i < string.length(); i++)
  {
    char c = string[i];
    if (c == '\n')
    {
      result[0] = std::max(result[0], static_cast<float>(x));
      x = 0;
      y += m_lineHeight;
      continue;
    }

    if (c < m_firstChar || c >= m_firstChar + m_charCount)
    {
      c = 32; // space
    }

    const FontGlyph& glyph = m_glyphs[static_cast<size_t>(c - m_firstChar)];
    x += glyph.advance();
  }

  result[0] = std::max(result[0], static_cast<float>(x));
  result[1] = static_cast<float>(y + m_lineHeight);
  return result;
}

void TextureFont::activate()
{
  m_texture->activate();
}

void TextureFont::deactivate()
{
  m_texture->deactivate();
}
} // namespace Renderer
} // namespace TrenchBroom
