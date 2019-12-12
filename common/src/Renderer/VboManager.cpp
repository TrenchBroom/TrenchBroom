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

#include "VboManager.h"

#include "Vbo.h"
#include "GL.h"
#include "Macros.h"

namespace TrenchBroom {
    namespace Renderer {
        /**
         * e.g. GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER
         */
        static GLenum toOpenGL(const VboType type) {
            switch (type) {
                case VboType::ArrayBuffer:
                    return GL_ARRAY_BUFFER;
                case VboType::ElementArrayBuffer:
                    return GL_ELEMENT_ARRAY_BUFFER;
                switchDefault()
            }
        }

        VboManager::VboManager() {}

        Vbo* VboManager::allocateVbo(VboType type, const size_t capacity) {
            return new Vbo(toOpenGL(type), capacity);
        }
    }
}
