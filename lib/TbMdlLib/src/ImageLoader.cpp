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

#include "mdl/ImageLoader.h"

#include "mdl/ImageLoaderImpl.h"

namespace tb::mdl
{

ImageLoader::ImageLoader(const char* begin, const char* end)
  : m_impl{std::make_unique<ImageLoaderImpl>(begin, end)}
{
}

ImageLoader::~ImageLoader() = default;

size_t ImageLoader::width() const
{
  return m_impl->width();
}

size_t ImageLoader::height() const
{
  return m_impl->height();
}

bool ImageLoader::hasPalette() const
{
  return m_impl->hasPalette();
}

std::vector<unsigned char> ImageLoader::loadPalette() const
{
  return m_impl->loadPalette();
}

std::vector<unsigned char> ImageLoader::loadPixels() const
{
  return m_impl->loadPixels();
}

} // namespace tb::mdl
