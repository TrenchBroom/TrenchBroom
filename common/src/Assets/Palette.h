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

#ifndef TrenchBroom_Palette
#define TrenchBroom_Palette

#include "Color.h"
#include "StringUtils.h"
#include "ByteBuffer.h"
#include "IO/Path.h"

#include <cassert>

namespace TrenchBroom {
    namespace Assets {
        class Palette {
        private:
            unsigned char* m_data;
            size_t m_size;
        public:
            Palette(const IO::Path& path);
            Palette(const Palette& other);
            ~Palette();
            
            void operator=(Palette other);

            template <typename IndexT, typename ColorT>
            void indexedToRgb(const Buffer<IndexT>& indexedImage, const size_t pixelCount, Buffer<ColorT>& rgbImage, Color& averageColor) const {
                indexedToRgb(&indexedImage[0], pixelCount, rgbImage, averageColor);
            }
            
            template <typename IndexT, typename ColorT>
            void indexedToRgb(const IndexT* indexedImage, const size_t pixelCount, Buffer<ColorT>& rgbImage, Color& averageColor) const {
                double avg[3];
                avg[0] = avg[1] = avg[2] = 0.0;
                for (size_t i = 0; i < pixelCount; ++i) {
                    const size_t index = static_cast<size_t>(static_cast<unsigned char>(indexedImage[i]));
                    assert(index < m_size);
                    for (size_t j = 0; j < 3; ++j) {
                        const unsigned char c = m_data[index * 3 + j];
                        rgbImage[i * 3 + j] = c;
                        avg[j] += static_cast<double>(c);
                    }
                }
                
                for (size_t i = 0; i < 3; ++i)
                    averageColor[i] = static_cast<float>(avg[i] / pixelCount / 0xFF);
                averageColor[3] = 1.0f;
            }
        private:
            void loadLmpPalette(const IO::Path& path);
            void loadPcxPalette(const IO::Path& path);
        };
    }
}

#endif /* defined(TrenchBroom_Palette) */
