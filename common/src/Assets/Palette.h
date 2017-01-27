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

#ifndef TrenchBroom_Palette
#define TrenchBroom_Palette

#include "Color.h"
#include "ByteBuffer.h"
#include "IO/MappedFile.h"

#include <cassert>

namespace TrenchBroom {
    namespace IO {
        class FileSystem;
        class Path;
    }
    
    namespace Assets {
        class Palette {
        public:
            typedef std::shared_ptr<Palette> Ptr;
        private:
            size_t m_size;
            unsigned char* m_data;
        public:
            Palette(const size_t size, unsigned char* data);
            Palette(const Palette& other);
            ~Palette();
            
            Palette& operator=(Palette other);
            friend void swap(Palette& lhs, Palette& rhs);

            static Palette loadFile(const IO::FileSystem& fs, const IO::Path& path);
            static Palette loadLmp(IO::MappedFile::Ptr file);
            static Palette loadPcx(IO::MappedFile::Ptr file);
            
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
        };
    }
}

#endif /* defined(TrenchBroom_Palette) */
