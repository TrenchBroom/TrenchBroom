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

#ifndef TrenchBroom_VboManager
#define TrenchBroom_VboManager

#include "Renderer/GL.h"

#include <cstddef> // for size_t

namespace TrenchBroom {
    namespace Renderer {
        class Vbo;

        enum class VboType {
            ArrayBuffer,
            ElementArrayBuffer
        };

        class VboManager {
        public:
            VboManager();
            /**
            * Immediately creates and binds to an OpenGL buffer of the given type and capacity.
            * The contents are initially unspecified. See Vbo class.
            */
            Vbo* allocateBlock(VboType type, size_t capacity);
        };
    }
}

#endif /* defined(TrenchBroom_VboManager) */
