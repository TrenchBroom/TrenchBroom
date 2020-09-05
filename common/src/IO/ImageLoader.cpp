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

#include "ImageLoader.h"

#include "IO/Path.h"
#include "IO/ImageLoaderImpl.h"

namespace TrenchBroom {
    namespace IO {
        ImageLoader::ImageLoader(const Format format, const Path& path) :
        m_impl(new ImageLoaderImpl(format, path)) {}

        ImageLoader::ImageLoader(const Format format, const char* begin, const char* end) :
        m_impl(new ImageLoaderImpl(format, begin, end)) {}

        ImageLoader::~ImageLoader() = default;

        size_t ImageLoader::paletteSize() const {
            return m_impl->paletteSize();
        }

        size_t ImageLoader::bitsPerPixel() const {
            return m_impl->bitsPerPixel();
        }

        size_t ImageLoader::width() const {
            return m_impl->width();
        }

        size_t ImageLoader::height() const {
            return m_impl->height();
        }

        size_t ImageLoader::byteWidth() const {
            return m_impl->byteWidth();
        }

        size_t ImageLoader::scanWidth() const {
            return m_impl->scanWidth();
        }

        bool ImageLoader::hasPalette() const {
            return m_impl->hasPalette();
        }

        bool ImageLoader::hasIndices() const {
            return m_impl->hasIndices();
        }

        bool ImageLoader::hasPixels() const {
            return m_impl->hasPixels();
        }

        std::vector<unsigned char> ImageLoader::loadPalette() const {
            return m_impl->loadPalette();
        }

        std::vector<unsigned char> ImageLoader::loadIndices() const {
            return m_impl->loadIndices();
        }

        std::vector<unsigned char> ImageLoader::loadPixels(const PixelFormat format) const {
            return m_impl->loadPixels(format);
        }
    }
}
