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

#include "FreeImageTextureReader.h"

#include "Color.h"
#include "FreeImage.h"
#include "StringUtils.h"
#include "Assets/Texture.h"
#include "IO/CharArrayReader.h"
#include "IO/Path.h"

#include <cstring>

namespace TrenchBroom {
    namespace IO {
        FreeImageTextureReader::FreeImageTextureReader(const NameStrategy& nameStrategy, const size_t mipCount) :
        TextureReader(nameStrategy),
        m_mipCount(mipCount) {
            assert(m_mipCount > 0);
        }

        Assets::Texture* FreeImageTextureReader::doReadTexture(const char* const begin, const char* const end, const Path& path) const {
            const size_t                imageSize       = static_cast<size_t>(end - begin);
            BYTE*                       imageBegin      = reinterpret_cast<BYTE*>(const_cast<char*>(begin));
            FIMEMORY*                   imageMemory     = FreeImage_OpenMemory(imageBegin, static_cast<DWORD>(imageSize));
            const FREE_IMAGE_FORMAT     imageFormat     = FreeImage_GetFileTypeFromMemory(imageMemory);
            FIBITMAP*                   image           = FreeImage_LoadFromMemory(imageFormat, imageMemory);
            const String                imageName       = path.filename();

            if (image == nullptr) {
                FreeImage_CloseMemory(imageMemory);
                return new Assets::Texture(textureName(imageName, path), 64, 64);
            }

            const size_t                imageWidth      = static_cast<size_t>(FreeImage_GetWidth(image));
            const size_t                imageHeight     = static_cast<size_t>(FreeImage_GetHeight(image));
            const FREE_IMAGE_COLOR_TYPE imageColourType = FreeImage_GetColorType(image);

            // This is supposed to indicate whether any pixels are transparent (alpha < 100%)
            const auto transparent = FreeImage_IsTransparent(image);

            const auto format = GL_RGBA;
            Assets::TextureBuffer::List buffers(m_mipCount);
            Assets::setMipBufferSize(buffers, m_mipCount, imageWidth, imageHeight, format);

            const auto inputBytesPerPixel = FreeImage_GetLine(image) / FreeImage_GetWidth(image);
            if (imageColourType != FIC_RGBALPHA || inputBytesPerPixel != 4) {
                FIBITMAP* tempImage = FreeImage_ConvertTo32Bits(image);
                FreeImage_Unload(image);
                image = tempImage;
            }

            FreeImage_FlipVertical(image);

            for (size_t mip = 0; mip < m_mipCount; ++mip) {
                const auto mipSize = Assets::sizeAtMipLevel(imageWidth, imageHeight, mip);
                
                FIBITMAP* mipImage;
                if (mip > 0) {
                    mipImage = FreeImage_Rescale(image, static_cast<int>(mipSize.x()), static_cast<int>(mipSize.y()), FILTER_BICUBIC);
                } else {
                    mipImage = image;
                }

                const auto bytesPerPixel = FreeImage_GetLine(mipImage) / FreeImage_GetWidth(mipImage);
                ensure(bytesPerPixel == 4, "expected to have converted image to 32-bit");

                unsigned char* outBytes = buffers.at(mip).ptr();

                for (size_t y = 0; y < mipSize.y(); ++y) {
                    BYTE* const scanline = FreeImage_GetScanLine(mipImage, static_cast<int>(y));
                    
                    BYTE* inPixel = scanline;
                    for (size_t x = 0; x < mipSize.x(); ++x) {
                        *(outBytes++) = inPixel[FI_RGBA_RED];
                        *(outBytes++) = inPixel[FI_RGBA_GREEN];
                        *(outBytes++) = inPixel[FI_RGBA_BLUE];
                        *(outBytes++) = inPixel[FI_RGBA_ALPHA];

                        inPixel += bytesPerPixel;
                    }
                }

                if (mip > 0) {
                    FreeImage_Unload(mipImage);
                }
            }

            FreeImage_Unload(image);
            FreeImage_CloseMemory(imageMemory);

            const auto textureType = transparent ? Assets::TextureType::Masked : Assets::TextureType::Opaque;

            return new Assets::Texture(textureName(imageName, path), imageWidth, imageHeight, Color(), buffers, format, textureType);
        }
    }

}
