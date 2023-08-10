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

#include "FreeTypeFontFactory.h"

#include "Exceptions.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/Reader.h"
#include "IO/SystemPaths.h"
#include "Macros.h"
#include "Renderer/FontDescriptor.h"
#include "Renderer/FontGlyph.h"
#include "Renderer/FontGlyphBuilder.h"
#include "Renderer/FontTexture.h"
#include "Renderer/RenderError.h"
#include "Renderer/TextureFont.h"

#include <algorithm>
#include <string>

namespace TrenchBroom
{
namespace Renderer
{
FreeTypeFontFactory::FreeTypeFontFactory()
  : m_library(nullptr)
{
  FT_Error error = FT_Init_FreeType(&m_library);
  if (error != 0)
  {
    m_library = nullptr;
    throw RenderException("Error initializing FreeType: " + std::to_string(error));
  }
}

FreeTypeFontFactory::~FreeTypeFontFactory()
{
  if (m_library != nullptr)
  {
    FT_Done_FreeType(m_library);
    m_library = nullptr;
  }
}

std::unique_ptr<TextureFont> FreeTypeFontFactory::doCreateFont(
  const FontDescriptor& fontDescriptor)
{
  auto [face, bufferedReader] = loadFont(fontDescriptor);
  auto font = buildFont(face, fontDescriptor.minChar(), fontDescriptor.charCount());
  FT_Done_Face(face);

  // NOTE: bufferedReader is returned from loadFont() just to keep the buffer from
  // being deallocated until after we call FT_Done_Face
  unused(bufferedReader);

  return font;
}

std::pair<FT_Face, IO::BufferedReader> FreeTypeFontFactory::loadFont(
  const FontDescriptor& fontDescriptor)
{
  const auto fontPath = fontDescriptor.path().is_absolute()
                          ? fontDescriptor.path()
                          : IO::SystemPaths::findResourceFile(fontDescriptor.path());

  return IO::Disk::openFile(fontPath)
    .and_then(
      [&](auto file) -> kdl::result<std::pair<FT_Face, IO::BufferedReader>, RenderError> {
        auto reader = file->reader().buffer();

        auto face = FT_Face{};
        const auto error = FT_New_Memory_Face(
          m_library,
          reinterpret_cast<const FT_Byte*>(reader.begin()),
          FT_Long(reader.size()),
          0,
          &face);
        if (error)
        {
          return RenderError{"FT_New_Memory_Face returned " + std::to_string(error)};
        }

        FT_Set_Pixel_Sizes(face, 0, FT_UInt(fontDescriptor.size()));
        return std::pair{face, std::move(reader)};
      })
    .if_error([&](auto e) {
      throw RenderException{
        "Error loading font '" + fontDescriptor.name() + "': " + e.msg};
    })
    .value();
}

std::unique_ptr<TextureFont> FreeTypeFontFactory::buildFont(
  FT_Face face, const unsigned char firstChar, const unsigned char charCount)
{
  const Metrics metrics = computeMetrics(face, firstChar, charCount);

  std::unique_ptr<FontTexture> texture =
    std::make_unique<FontTexture>(charCount, metrics.cellSize, metrics.lineHeight);
  FontGlyphBuilder glyphBuilder(metrics.maxAscend, metrics.cellSize, 3, *texture);

  FT_GlyphSlot glyph = face->glyph;
  std::vector<FontGlyph> glyphs;
  for (unsigned char c = firstChar; c < firstChar + charCount; ++c)
  {
    FT_Error error = FT_Load_Char(face, static_cast<FT_ULong>(c), FT_LOAD_RENDER);
    if (error != 0)
    {
      glyphs.push_back(FontGlyph(0, 0, 0, 0, 0));
    }
    else
    {
      glyphs.push_back(glyphBuilder.createGlyph(
        static_cast<size_t>(glyph->bitmap_left),
        static_cast<size_t>(glyph->bitmap_top),
        static_cast<size_t>(glyph->bitmap.width),
        static_cast<size_t>(glyph->bitmap.rows),
        static_cast<size_t>(glyph->advance.x >> 6),
        reinterpret_cast<char*>(glyph->bitmap.buffer),
        static_cast<size_t>(glyph->bitmap.pitch)));
    }
  }

  return std::make_unique<TextureFont>(
    std::move(texture),
    glyphs,
    static_cast<int>(metrics.lineHeight),
    firstChar,
    charCount);
}

FreeTypeFontFactory::Metrics FreeTypeFontFactory::computeMetrics(
  FT_Face face, const unsigned char firstChar, const unsigned char charCount) const
{
  FT_GlyphSlot glyph = face->glyph;

  int maxWidth = 0;
  int maxAscend = 0;
  int maxDescend = 0;
  int lineHeight = 0;

  for (unsigned char c = firstChar; c < firstChar + charCount; ++c)
  {
    FT_Error error = FT_Load_Char(face, static_cast<FT_ULong>(c), FT_LOAD_RENDER);
    if (error != 0)
    {
      continue;
    }

    maxWidth =
      std::max(maxWidth, glyph->bitmap_left + static_cast<FT_Int>(glyph->bitmap.width));
    maxAscend = std::max(maxAscend, glyph->bitmap_top);
    maxDescend =
      std::max(maxDescend, static_cast<FT_Int>(glyph->bitmap.rows) - glyph->bitmap_top);
    lineHeight = std::max(lineHeight, static_cast<int>(glyph->metrics.height >> 6));
  }

  const int cellSize = std::max(maxWidth, maxAscend + maxDescend);
  return {
    static_cast<size_t>(cellSize),
    static_cast<size_t>(maxAscend),
    static_cast<size_t>(lineHeight)};
}
} // namespace Renderer
} // namespace TrenchBroom
