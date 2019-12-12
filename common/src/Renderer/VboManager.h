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

#include "Macros.h"
#include "Renderer/GL.h"

#include <memory>
#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class Vbo;

        enum class VboType {
            ArrayBuffer,
            ElementArrayBuffer
        };

        /**
         * e.g. GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER
         */
        GLenum toOpenGL(VboType type);

        class VboManager {
        public:
            VboManager();
            Vbo* allocateBlock(VboType type, size_t capacity);
        };
    }
}

#endif /* defined(TrenchBroom_VboManager) */
