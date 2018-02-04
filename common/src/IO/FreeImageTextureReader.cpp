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
#include "StringUtils.h"
#include "Assets/Texture.h"
#include "IO/CharArrayReader.h"
#include "IO/Path.h"

#include "FreeImage.h"

namespace TrenchBroom {

    namespace IO {

        FreeImageTextureReader::FreeImageTextureReader(const NameStrategy& nameStrategy) :
            TextureReader(nameStrategy) {}

        Assets::Texture* FreeImageTextureReader::doReadTexture(const char* const begin, const char* const end, const Path& path) const {
            BYTE*                   imageBegin      = reinterpret_cast<BYTE*>(const_cast<char*>(begin));
            size_t                  imageSize       = static_cast<size_t>(end - begin);
            FIMEMORY*               imageMemory     = FreeImage_OpenMemory(imageBegin, imageSize);
            FREE_IMAGE_FORMAT       imageFormat     = FreeImage_GetFileTypeFromMemory(imageMemory);
            FIBITMAP*               image           = FreeImage_LoadFromMemory(imageFormat, imageMemory);

            String                  imageName       = path.filename();
            size_t                  imageWidth      = FreeImage_GetWidth(image);
            size_t                  imageHeight     = FreeImage_GetHeight(image);
            FREE_IMAGE_COLOR_TYPE   imageColourType = FreeImage_GetColorType(image);

            Assets::TextureBuffer::List buffers(4);
            Assets::setMipBufferSize(buffers, imageWidth, imageHeight);

            // TODO: Alpha channel seems to be unsupported by the Texture class
            if (imageColourType != FIC_RGB) {
                FIBITMAP* tempImage = FreeImage_ConvertTo24Bits(image);
                FreeImage_Unload(image);
                image = tempImage;
            }

            std::memcpy(buffers[0].ptr(), FreeImage_GetBits(image), buffers[0].size());
            for (size_t mip = 1; mip < buffers.size(); ++mip) {
                FIBITMAP* mipImage = FreeImage_Rescale(image, imageWidth >> mip, imageHeight >> mip, FILTER_BICUBIC);
                std::memcpy(buffers[mip].ptr(), FreeImage_GetBits(mipImage), buffers[mip].size());
                FreeImage_Unload(mipImage);
            }

            FreeImage_Unload(image);
            FreeImage_CloseMemory(imageMemory);

            Assets::Texture* texture = new Assets::Texture(textureName(imageName, path), imageWidth, imageHeight, Color(), buffers);
            texture->setFormat(0x80E0 /* GL_BGR */);
            return texture;
        }
    }

}
