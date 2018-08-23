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
        enum class PaletteTransparency {
            Opaque, Index255Transparent
        };

        class Palette {
        private:
            class Data {
            private:
                size_t m_size;
                unsigned char* m_data;
            public:
                Data(const size_t size, unsigned char* data);
                ~Data();

                template <typename IndexT, typename ColorT>
                void indexedToRgba(const Buffer<IndexT>& indexedImage, const size_t pixelCount, Buffer<ColorT>& rgbaImage, Color& averageColor, const PaletteTransparency transparency) const {
                    indexedToRgba(&indexedImage[0], pixelCount, rgbaImage, averageColor, transparency);
                }
                
                template <typename IndexT, typename ColorT>
                void indexedToRgba(const IndexT* indexedImage, const size_t pixelCount, Buffer<ColorT>& rgbaImage, Color& averageColor, const PaletteTransparency transparency) const {
                    double avg[3];
                    avg[0] = avg[1] = avg[2] = 0.0;
                    for (size_t i = 0; i < pixelCount; ++i) {
                        const size_t index = static_cast<size_t>(static_cast<unsigned char>(indexedImage[i]));
                        assert(index < m_size);
                        for (size_t j = 0; j < 3; ++j) {
                            const unsigned char c = m_data[index * 3 + j];
                            rgbaImage[i * 4 + j] = c;
                            avg[j] += static_cast<double>(c);
                        }
                        switch (transparency) {
                            case PaletteTransparency::Opaque:
                                rgbaImage[i * 4 + 3] = 0xFF;
                                break;
                            case PaletteTransparency::Index255Transparent:
                                rgbaImage[i * 4 + 3] = (index == 255) ? 0x00 : 0xFF;
                                break;
                        }
                    }
                    
                    for (size_t i = 0; i < 3; ++i)
                        averageColor[i] = static_cast<float>(avg[i] / pixelCount / 0xFF);
                    averageColor[3] = 1.0f;
                }
            };
            
            typedef std::shared_ptr<Data> DataPtr;
            DataPtr m_data;
        public:
            Palette(const size_t size, unsigned char* data);
            
            static Palette loadFile(const IO::FileSystem& fs, const IO::Path& path);
            static Palette loadLmp(IO::MappedFile::Ptr file);
            static Palette loadPcx(IO::MappedFile::Ptr file);
            
            template <typename IndexT, typename ColorT>
            void indexedToRgba(const Buffer<IndexT>& indexedImage, const size_t pixelCount, Buffer<ColorT>& rgbaImage, Color& averageColor, const PaletteTransparency transparency = PaletteTransparency::Opaque) const {
                m_data->indexedToRgba(indexedImage, pixelCount, rgbaImage, averageColor, transparency);
            }
            
            template <typename IndexT, typename ColorT>
            void indexedToRgba(const IndexT* indexedImage, const size_t pixelCount, Buffer<ColorT>& rgbaImage, Color& averageColor, const PaletteTransparency transparency = PaletteTransparency::Opaque) const {
                m_data->indexedToRgba(indexedImage, pixelCount, rgbaImage, averageColor, transparency);
            }
        };
    }
}

#endif /* defined(TrenchBroom_Palette) */
