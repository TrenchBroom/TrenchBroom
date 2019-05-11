/*
 Copyright (C) 2010-2016 Kristian Duske

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

#include "MipTextureReader.h"

#include "Color.h"
#include "StringUtils.h"
#include "Assets/Palette.h"
#include "Assets/Texture.h"
#include "IO/File.h"
#include "IO/Reader.h"
#include "IO/Path.h"

namespace TrenchBroom {
    namespace IO {
        namespace MipLayout {
            static const size_t TextureNameLength = 16;
        }

        MipTextureReader::MipTextureReader(const NameStrategy& nameStrategy) :
        TextureReader(nameStrategy) {}

        MipTextureReader::~MipTextureReader() = default;

        size_t MipTextureReader::mipFileSize(const size_t width, const size_t height, const size_t mipLevels) {
            size_t result = 0;
            for (size_t i = 0; i < mipLevels; ++i) {
                result += mipSize(width, height, i);
            }
            return result;
        }

        Assets::Texture* MipTextureReader::doReadTexture(std::shared_ptr<File> file) const {
            static const size_t MipLevels = 4;

            Color averageColor;
            Assets::TextureBuffer::List buffers(MipLevels);
            size_t offset[MipLevels];

            const auto path = file->path();
            const auto basename = path.lastComponent().deleteExtension().asString();
            const auto name = textureName(basename, path);
            try {
                auto reader = file->reader().buffer();
                reader.readString(MipLayout::TextureNameLength);

                const auto width = reader.readSize<int32_t>();
                const auto height = reader.readSize<int32_t>();

                if (!checkTextureDimensions(width, height)) {
                    return new Assets::Texture(name, 16, 16);
                }

                for (size_t i = 0; i < MipLevels; ++i) {
                    offset[i] = reader.readSize<int32_t>();
                }

                const auto transparent = (!name.empty() && name.at(0) == '{')
                                         ? Assets::PaletteTransparency::Index255Transparent
                                         : Assets::PaletteTransparency::Opaque;

                Assets::setMipBufferSize(buffers, MipLevels, width, height, GL_RGBA);
                auto palette = doGetPalette(reader, offset, width, height);

                if (!palette.initialized()) {
                    return new Assets::Texture(name, width, height);
                }

                for (size_t i = 0; i < MipLevels; ++i) {
                    reader.seekFromBegin(offset[i]);
                    const size_t size = mipSize(width, height, i);

                    Color tempColor;
                    palette.indexedToRgba(reader, size, buffers[i], transparent, tempColor);
                    if (i == 0) {
                        averageColor = tempColor;
                    }
                }

                const auto type = (transparent == Assets::PaletteTransparency::Index255Transparent)
                                  ? Assets::TextureType::Masked
                                  : Assets::TextureType::Opaque;
                return new Assets::Texture(name, width, height, averageColor, buffers, GL_RGBA, type);
            } catch (const ReaderException&) {
                return new Assets::Texture(name, 16, 16);
            }
        }
    }
}
