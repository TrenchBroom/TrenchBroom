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

#include <ft2build.h>
#include FT_FREETYPE_H

#include "IO/Reader.h"
#include "Renderer/FontFactory.h"

#include <memory>
#include <utility>

namespace TrenchBroom {
namespace Renderer {
class FontDescriptor;
class TextureFont;

class FreeTypeFontFactory : public FontFactory {
private:
  FT_Library m_library;

public:
  FreeTypeFontFactory();
  ~FreeTypeFontFactory() override;

private:
  std::unique_ptr<TextureFont> doCreateFont(const FontDescriptor& fontDescriptor) override;

  std::pair<FT_Face, IO::BufferedReader> loadFont(const FontDescriptor& fontDescriptor);
  std::unique_ptr<TextureFont> buildFont(
    FT_Face face, unsigned char firstChar, unsigned char charCount);

  Metrics computeMetrics(FT_Face face, unsigned char firstChar, unsigned char charCount) const;
};
} // namespace Renderer
} // namespace TrenchBroom
