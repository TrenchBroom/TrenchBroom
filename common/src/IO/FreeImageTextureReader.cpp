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
        FreeImageTextureReader::FreeImageTextureReader(const NameStrategy& nameStrategy) :
        TextureReader(nameStrategy) {}

        /**
         * The byte order of a 32bpp FIBITMAP is defined by the macros FI_RGBA_RED,
         * FI_RGBA_GREEN, FI_RGBA_BLUE, FI_RGBA_ALPHA.
         * From looking at FreeImage.h, there are only two possible orders,
         * so we can handle both possible orders and map them to the relevant GL_RGBA
         * or GL_BGRA constant.
         */
        static constexpr GLenum freeImage32BPPFormatToGLFormat() {            
            if (FI_RGBA_RED == 0
                && FI_RGBA_GREEN == 1
                && FI_RGBA_BLUE == 2
                && FI_RGBA_ALPHA == 3) {

                return GL_RGBA;
            } else if (FI_RGBA_BLUE == 0
                && FI_RGBA_GREEN == 1
                && FI_RGBA_RED == 2
                && FI_RGBA_ALPHA == 3) {

                return GL_BGRA;
            } else {
                throw std::runtime_error("Expected FreeImage to use RGBA or BGRA");
            }
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

            const auto mipCount = 1U;
            constexpr auto format = freeImage32BPPFormatToGLFormat();
            Assets::TextureBuffer::List buffers(mipCount);
            Assets::setMipBufferSize(buffers, mipCount, imageWidth, imageHeight, format);

            const auto inputBytesPerPixel = FreeImage_GetLine(image) / FreeImage_GetWidth(image);
            if (imageColourType != FIC_RGBALPHA || inputBytesPerPixel != 4) {
                FIBITMAP* tempImage = FreeImage_ConvertTo32Bits(image);
                FreeImage_Unload(image);
                image = tempImage;
            }

            const auto bytesPerPixel = FreeImage_GetLine(image) / FreeImage_GetWidth(image);
            ensure(bytesPerPixel == 4, "expected to have converted image to 32-bit");

            unsigned char* outBytes = buffers.at(0).ptr();
            const auto outBytesPerRow = static_cast<int>(imageWidth * 4);

            FreeImage_ConvertToRawBits(outBytes, image, outBytesPerRow, 32, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, TRUE);

            FreeImage_Unload(image);
            FreeImage_CloseMemory(imageMemory);

            
            const auto textureType = transparent ? Assets::TextureType::Masked : Assets::TextureType::Opaque;

            return new Assets::Texture(textureName(imageName, path), imageWidth, imageHeight, Color(), buffers, format, textureType);
        }
    }
}
