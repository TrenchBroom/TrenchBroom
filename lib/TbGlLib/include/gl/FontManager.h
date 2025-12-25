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

#include "Macros.h"

#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace tb::gl
{
class FontDescriptor;
class FontFactory;
class TextureFont;

using FindFontFunc = std::function<std::filesystem::path(const std::filesystem::path&)>;

class FontManager
{
private:
  std::unique_ptr<FontFactory> m_factory;
  std::map<FontDescriptor, std::unique_ptr<TextureFont>> m_cache;

public:
  explicit FontManager(FindFontFunc findFontFunc);
  ~FontManager();

  TextureFont& font(const FontDescriptor& fontDescriptor);
  FontDescriptor selectFontSize(
    const FontDescriptor& fontDescriptor,
    const std::string& string,
    float maxWidth,
    size_t minFontSize);
  void clearCache();

  deleteCopyAndMove(FontManager);
};

} // namespace tb::gl
