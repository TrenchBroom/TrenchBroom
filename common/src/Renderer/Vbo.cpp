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

#include "Vbo.h"

#include "Ensure.h"
#include "Macros.h"
#include "Exceptions.h"
#include "Renderer/VboBlock.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <memory>

namespace TrenchBroom {
    namespace Renderer {
        GLenum toOpenGL(const VboType type) {
            switch (type) {
                case VboType::ArrayBuffer:
                    return GL_ARRAY_BUFFER;
                case VboType::ElementArrayBuffer:
                    return GL_ELEMENT_ARRAY_BUFFER;
                switchDefault()
            }
        }

        Vbo::Vbo() {}

        VboBlock* Vbo::allocateBlock(VboType type, const size_t capacity) {
            return new VboBlock(toOpenGL(type), capacity);
        }
    }
}
