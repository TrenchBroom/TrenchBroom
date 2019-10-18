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

#include "ImageUtils.h"
#include "Ensure.h"

#include <vecmath/vec.h>

#include <FreeImage.h>

namespace TrenchBroom {
    namespace Assets {
        void resizeMips(TextureBuffer::List& buffers, const vm::vec2s& oldSize, const vm::vec2s& newSize) {
            if (oldSize == newSize)
                return;

            for (size_t i = 0; i < buffers.size(); ++i) {
                const auto div = size_t(1) << i;
                const auto oldWidth = static_cast<int>(oldSize.x() / div);
                const auto oldHeight = static_cast<int>(oldSize.y() / div);
                const auto oldPitch = oldWidth * 3;
                auto* oldPtr = buffers[i].ptr();

                auto* oldBitmap = FreeImage_ConvertFromRawBits(oldPtr, oldWidth, oldHeight, oldPitch, 24, 0xFF0000, 0x00FF00, 0x0000FF, true);
                ensure(oldBitmap != nullptr, "oldBitmap is null");

                const auto newWidth = static_cast<int>(newSize.x() / div);
                const auto newHeight = static_cast<int>(newSize.y() / div);
                const auto newPitch = newWidth * 3;
                auto* newBitmap = FreeImage_Rescale(oldBitmap, newWidth, newHeight, FILTER_BICUBIC);
                ensure(newBitmap != nullptr, "newBitmap is null");

                buffers[i] = TextureBuffer(3 * newSize.x() * newSize.y());
                auto* newPtr = buffers[i].ptr();

                FreeImage_ConvertToRawBits(newPtr, newBitmap, newPitch, 24, 0xFF0000, 0x00FF00, 0x0000FF, true);
                FreeImage_Unload(oldBitmap);
                FreeImage_Unload(newBitmap);
            }
        }
    }
}
