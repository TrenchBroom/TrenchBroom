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

#include "gl/FontManager.h"

#include "gl/FontDescriptor.h"
#include "gl/FreeTypeFontFactory.h"
#include "gl/TextureFont.h"

#include <algorithm>
#include <string>

namespace tb::gl
{

FontManager::FontManager(FindFontFunc findFontFunc)
  : m_factory{std::make_unique<FreeTypeFontFactory>(std::move(findFontFunc))}
{
}

FontManager::~FontManager() = default;

TextureFont& FontManager::font(const FontDescriptor& fontDescriptor)
{
  auto it = m_cache.lower_bound(fontDescriptor);
  if (it == std::end(m_cache) || it->first != fontDescriptor)
  {
    it = m_cache.insert(it, {fontDescriptor, m_factory->createFont(fontDescriptor)});
  }

  return *it->second;
}

FontDescriptor FontManager::selectFontSize(
  const FontDescriptor& fontDescriptor,
  const std::string& string,
  const float maxWidth,
  const size_t minFontSize)
{
  auto actualDescriptor = fontDescriptor;
  auto actualBounds = font(actualDescriptor).measure(string);
  while (actualBounds.x() > maxWidth && actualDescriptor.size() > minFontSize)
  {
    actualDescriptor =
      FontDescriptor(actualDescriptor.path(), actualDescriptor.size() - 1);
    actualBounds = font(actualDescriptor).measure(string);
  }
  return actualDescriptor;
}

void FontManager::clearCache()
{
  // extract() already empties m_cache, and unlike a views pipeline over the map, its
  // extracted values are a plain, ordinary (non-proxy) vector that can be moved from
  auto [keys, values] = m_cache.extract();
  std::ranges::move(values, std::back_inserter(m_fontsToDestroy));
}

void FontManager::destroyPendingFonts(Gl& gl)
{
  for (auto& font : m_fontsToDestroy)
  {
    font->destroy(gl);
  }
  m_fontsToDestroy.clear();
}

} // namespace tb::gl
