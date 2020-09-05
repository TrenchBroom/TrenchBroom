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
#include "IO/Reader.h"

#include <cassert>
#include <memory>
#include <vector>

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
                std::vector<unsigned char> m_data;
            public:
                Data(std::vector<unsigned char>&& data);

                /**
                 * Converts the given index buffer to an RGBA image.
                 *
                 * @tparam IndexT the index type
                 * @tparam ColorT the pixel type
                 * @param indexedImage the index buffer
                 * @param pixelCount the number of pixels
                 * @param rgbaImage the pixel buffer
                 * @param transparency controls whether or not the given index buffer contains a transparent index
                 * @param averageColor output parameter for the average color of the generated pixel buffer
                 * @return true if the given index buffer did contain a transparent index, unless the transparency parameter
                 *     indicates that the image is opaque
                 */
                template <typename IndexT, typename ColorT>
                bool indexedToRgba(const std::vector<IndexT>& indexedImage, const size_t pixelCount, std::vector<ColorT>& rgbaImage, const PaletteTransparency transparency, Color& averageColor) const {
                    auto reader = IO::Reader::from(&indexedImage[0], &indexedImage[0] + pixelCount);
                    return indexedToRgba(reader, pixelCount, rgbaImage, transparency, averageColor);
                }

                /**
                 * Converts the given index buffer to an RGBA image.
                 *
                 * @tparam ColorT the pixel type
                 * @param reader the index buffer reader
                 * @param pixelCount the number of pixels
                 * @param rgbaImage the pixel buffer
                 * @param transparency controls whether or not the given index buffer contains a transparent index
                 * @param averageColor output parameter for the average color of the generated pixel buffer
                 * @return true if the given index buffer did contain a transparent index, unless the transparency parameter
                 *     indicates that the image is opaque
                 */
                template <typename ColorT>
                bool indexedToRgba(IO::Reader& reader, const size_t pixelCount, std::vector<ColorT>& rgbaImage, const PaletteTransparency transparency, Color& averageColor) const {
                    double avg[3];
                    avg[0] = avg[1] = avg[2] = 0.0;
                    bool hasTransparency = false;
                    for (size_t i = 0; i < pixelCount; ++i) {
                        const size_t index = reader.readSize<unsigned char>();
                        assert(index < m_data.size());
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
                                rgbaImage[i * 4 + 3] = static_cast<ColorT>((index == 255) ? 0x00 : 0xFF);
                                hasTransparency |= (index == 255);
                                break;
                        }
                    }

                    for (size_t i = 0; i < 3; ++i) {
                        averageColor[i] = static_cast<float>(avg[i] / static_cast<double>(pixelCount) / static_cast<double>(0xFF));
                    }
                    averageColor[3] = 1.0f;

                    return hasTransparency;
                }
            };

            using DataPtr = std::shared_ptr<Data>;
            DataPtr m_data;
        public:
            Palette();
            Palette(std::vector<unsigned char> data);

            static Palette loadFile(const IO::FileSystem& fs, const IO::Path& path);
            static Palette loadLmp(IO::Reader& reader);
            static Palette loadPcx(IO::Reader& reader);
            static Palette loadBmp(IO::Reader& reader);
            static Palette fromRaw(IO::Reader& reader);

            bool initialized() const;

            /**
             * Converts the given index buffer to an RGBA image.
             *
             * @tparam IndexT the index type
             * @tparam ColorT the pixel type
             * @param indexedImage the index buffer
             * @param pixelCount the number of pixels
             * @param rgbaImage the pixel buffer
             * @param transparency controls whether or not the given index buffer contains a transparent index
             * @param averageColor output parameter for the average color of the generated pixel buffer
             * @return true if the given index buffer did contain a transparent index, unless the transparency parameter
             *     indicates that the image is opaque
             */
            template <typename IndexT, typename ColorT>
            bool indexedToRgba(const std::vector<IndexT>& indexedImage, const size_t pixelCount, std::vector<ColorT>& rgbaImage, const PaletteTransparency transparency, Color& averageColor) const {
                return m_data->indexedToRgba(indexedImage, pixelCount, rgbaImage, transparency, averageColor);
            }

            /**
             * Converts the given index buffer to an RGBA image.
             *
             * @tparam ColorT the pixel type
             * @param reader the index buffer
             * @param pixelCount the number of pixels
             * @param rgbaImage the pixel buffer
             * @param transparency controls whether or not the given index buffer contains a transparent index
             * @param averageColor output parameter for the average color of the generated pixel buffer
             * @return true if the given index buffer did contain a transparent index, unless the transparency parameter
             *     indicates that the image is opaque
             */
            template <typename ColorT>
            bool indexedToRgba(IO::Reader& reader, const size_t pixelCount, std::vector<ColorT>& rgbaImage, const PaletteTransparency transparency, Color& averageColor) const {
                return m_data->indexedToRgba(reader, pixelCount, rgbaImage, transparency, averageColor);
            }
        };
    }
}

#endif /* defined(TrenchBroom_Palette) */
