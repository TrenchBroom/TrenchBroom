/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef TrenchBroom_ImageLoader
#define TrenchBroom_ImageLoader

#include "ByteBuffer.h"

namespace TrenchBroom {
    namespace IO {
        class Path;
        class ImageLoaderImpl;
        
        class ImageLoader {
        public:
            enum Format {
                PCX
            };
            
            enum PixelFormat {
                RGB,
                RGBA
            };
        private:
            // we're using the PIMPL idiom here to insulate the clients from the FreeImage headers
            ImageLoaderImpl* m_impl;
        public:
            ImageLoader(const Format format, const Path& path);
            ImageLoader(const Format format, const char* begin, const char* end);
            ~ImageLoader();
            
            size_t paletteSize() const;
            size_t bitsPerPixel() const;
            size_t width() const;
            size_t height() const;
            size_t byteWidth() const;
            size_t scanWidth() const;
            
            bool hasPalette() const;
            bool hasIndices() const;
            bool hasPixels() const;
            
            const Buffer<unsigned char>& palette() const;
            const Buffer<unsigned char>& indices() const;
            const Buffer<unsigned char>& pixels(const PixelFormat format) const;
        private:
            ImageLoader(const ImageLoader& other);
            ImageLoader& operator=(const ImageLoader& other);
        };
    }
}

#endif /* defined(TrenchBroom_ImageLoader) */
