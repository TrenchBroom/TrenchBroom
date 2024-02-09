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

#include "Error.h"
#include "Exceptions.h"
#include "IO/DiskIO.h"
#include "IO/File.h" // IWYU pragma: keep
#include "IO/Reader.h"
#include "IO/SystemPaths.h"
#include "Macros.h"
#include "Renderer/FontDescriptor.h"
#include "Renderer/FontGlyph.h"
#include "Renderer/FontGlyphBuilder.h"
#include "Renderer/FontTexture.h"
#include "Renderer/TextureFont.h"

#include <algorithm>
#include <string>

namespace TrenchBroom::Renderer
{

namespace
{
auto initializeFreeType()
{
  auto library = FT_Library{nullptr};
  if (const auto error = FT_Init_FreeType(&library); error != 0)
  {
    throw RenderException{"Error initializing FreeType: " + std::to_string(error)};
  }

  return kdl::resource{library, FT_Done_FreeType};
}


auto loadFont(FT_Library library, const FontDescriptor& fontDescriptor)
{
  const auto fontPath = fontDescriptor.path().is_absolute()
                          ? fontDescriptor.path()
                          : IO::SystemPaths::findResourceFile(fontDescriptor.path());

  return IO::Disk::openFile(fontPath)
    .and_then(
      [&](auto file) -> Result<std::pair<kdl::resource<FT_Face>, IO::BufferedReader>> {
        auto reader = file->reader().buffer();

        auto face = FT_Face{};
        const auto error = FT_New_Memory_Face(
          library,
          reinterpret_cast<const FT_Byte*>(reader.begin()),
          FT_Long(reader.size()),
          0,
          &face);
        if (error)
        {
          return Error{"FT_New_Memory_Face returned " + std::to_string(error)};
        }

        FT_Set_Pixel_Sizes(face, 0, FT_UInt(fontDescriptor.size()));
        return std::pair{kdl::resource{face, FT_Done_Face}, std::move(reader)};
      })
    .if_error([&](auto e) {
      throw RenderException{
        "Error loading font '" + fontDescriptor.name() + "': " + e.msg};
    })
    .value();
}

FontFactory::Metrics computeMetrics(FT_Face face)
{
  const auto ascend = size_t(face->size->metrics.ascender >> 6);
  const auto descend = size_t(-face->size->metrics.descender >> 6);
  const auto lineHeight = size_t(face->size->metrics.height >> 6);
  const auto width = size_t(face->size->metrics.max_advance >> 6);

  const auto cellSize = std::max(width, ascend + descend);
  return {cellSize, ascend, descend, lineHeight};
}

std::unique_ptr<TextureFont> buildFont(
  FT_Face face, const unsigned char firstChar, const unsigned char charCount)
{
  const auto metrics = computeMetrics(face);

  auto texture =
    std::make_unique<FontTexture>(charCount, metrics.cellSize, metrics.lineHeight);
  auto glyphBuilder = FontGlyphBuilder{metrics.ascend, metrics.cellSize, 3, *texture};

  const auto glyph = face->glyph;
  auto glyphs = std::vector<FontGlyph>{};
  for (unsigned char c = firstChar; c < firstChar + charCount; ++c)
  {
    if (FT_Load_Char(face, FT_ULong(c), FT_LOAD_RENDER) == 0)
    {
      glyphs.push_back(glyphBuilder.createGlyph(
        size_t(glyph->bitmap_left),
        size_t(glyph->bitmap_top),
        size_t(glyph->bitmap.width),
        size_t(glyph->bitmap.rows),
        size_t(glyph->advance.x >> 6),
        reinterpret_cast<char*>(glyph->bitmap.buffer),
        size_t(glyph->bitmap.pitch)));
    }
    else
    {
      glyphs.emplace_back(0, 0, 0, 0, 0);
    }
  }

  return std::make_unique<TextureFont>(
    std::move(texture),
    glyphs,
    int(metrics.ascend),
    int(metrics.descend),
    int(metrics.lineHeight),
    firstChar,
    charCount);
}

} // namespace

FreeTypeFontFactory::FreeTypeFontFactory()
  : m_library{initializeFreeType()}
{
}

std::unique_ptr<TextureFont> FreeTypeFontFactory::doCreateFont(
  const FontDescriptor& fontDescriptor)
{
  auto [face, bufferedReader] = loadFont(*m_library, fontDescriptor);
  auto font = buildFont(*face, fontDescriptor.minChar(), fontDescriptor.charCount());

  // NOTE: bufferedReader is returned from loadFont() just to keep the buffer from
  // being deallocated until after we call FT_Done_Face
  unused(bufferedReader);

  return font;
}

} // namespace TrenchBroom::Renderer
