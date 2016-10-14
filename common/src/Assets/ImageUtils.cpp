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

#include "ImageUtils.h"

#include <FreeImage.h>

namespace TrenchBroom {
    namespace Assets {
        void resizeMips(TextureBuffer::List& buffers, const Vec2s& oldSize, const Vec2s& newSize) {
            if (oldSize == newSize)
                return;
            
            for (size_t i = 0; i < buffers.size(); ++i) {
                const size_t div = 1 << i;
                const int oldWidth = static_cast<int>(oldSize.x() / div);
                const int oldHeight = static_cast<int>(oldSize.y() / div);
                const int oldPitch = oldWidth * 3;
                unsigned char* oldPtr = buffers[i].ptr();
                
                FIBITMAP* oldBitmap = FreeImage_ConvertFromRawBits(oldPtr, oldWidth, oldHeight, oldPitch, 24, 0xFF0000, 0x00FF00, 0x0000FF, true);
                ensure(oldBitmap != NULL, "oldBitmap is null");
                
                const int newWidth = static_cast<int>(newSize.x() / div);
                const int newHeight = static_cast<int>(newSize.y() / div);
                const int newPitch = newWidth * 3;
                FIBITMAP* newBitmap = FreeImage_Rescale(oldBitmap, newWidth, newHeight, FILTER_BICUBIC);
                ensure(newBitmap != NULL, "newBitmap is null");
                
                buffers[i] = TextureBuffer(3 * newSize.x() * newSize.y());
                unsigned char* newPtr = buffers[i].ptr();
                
                FreeImage_ConvertToRawBits(newPtr, newBitmap, newPitch, 24, 0xFF0000, 0x00FF00, 0x0000FF, true);
                FreeImage_Unload(oldBitmap);
                FreeImage_Unload(newBitmap);
            }
        }
    }
}
