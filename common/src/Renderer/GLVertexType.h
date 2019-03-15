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

#ifndef TrenchBroom_VertexSpec
#define TrenchBroom_VertexSpec

#include "Renderer/GL.h"
#include "Renderer/GLVertexAttributeType.h"
#include "Renderer/GLVertex.h"

#include <vector>

#include <vecmath/forward.h>

namespace TrenchBroom {
    namespace Renderer {
        template <typename... Attrs>
        struct GLVertexType;

        template <typename Attr, typename... Attrs>
        struct GLVertexType<Attr, Attrs...> {
            using Vertex = GLVertex<Attr, Attrs...>;
            static const size_t Size = sizeof(Vertex);

            static void setup(const size_t baseOffset) {
                doSetup(0, Size, baseOffset);
            }

            static void cleanup() {
                doCleanup(0);
            }

            static void doSetup(const size_t index, const size_t stride, const size_t offset) {
                Attr::setup(index, stride, offset);
                GLVertexType<Attrs...>::doSetup(index + 1, stride, offset + Attr::Size);
            }

            static void doCleanup(const size_t index) {
                GLVertexType<Attrs...>::doCleanup(index + 1);
                Attr::cleanup(index);
            }
        };

        template <typename Attr>
        struct GLVertexType<Attr> {
            using Vertex = GLVertex<Attr>;
            static const size_t Size = sizeof(Vertex);

            static void setup(const size_t baseOffset) {
                doSetup(0, Size, baseOffset);
            }

            static void cleanup() {
                doCleanup(0);
            }

            static void doSetup(const size_t index, const size_t stride, const size_t offset) {
                Attr::setup(index, stride, offset);
            }

            static void doCleanup(const size_t index) {
                Attr::cleanup(index);
            }
        };

        namespace GLVertexTypes {
            using P2     = GLVertexType<GLVertexAttributeTypes::P2>;
            using P3     = GLVertexType<GLVertexAttributeTypes::P3>;
            using P2C4   = GLVertexType<GLVertexAttributeTypes::P2, GLVertexAttributeTypes::C4>;
            using P3C4   = GLVertexType<GLVertexAttributeTypes::P3, GLVertexAttributeTypes::C4>;
            using P2T2   = GLVertexType<GLVertexAttributeTypes::P2, GLVertexAttributeTypes::T02>;
            using P3T2   = GLVertexType<GLVertexAttributeTypes::P3, GLVertexAttributeTypes::T02>;
            using P2T2C4 = GLVertexType<GLVertexAttributeTypes::P2, GLVertexAttributeTypes::T02, GLVertexAttributeTypes::C4>;
            using P3T2C4 = GLVertexType<GLVertexAttributeTypes::P3, GLVertexAttributeTypes::T02, GLVertexAttributeTypes::C4>;
            using P3N    = GLVertexType<GLVertexAttributeTypes::P3, GLVertexAttributeTypes::N>;
            using P3NC4  = GLVertexType<GLVertexAttributeTypes::P3, GLVertexAttributeTypes::N, GLVertexAttributeTypes::C4>;
            using P3NT2  = GLVertexType<GLVertexAttributeTypes::P3, GLVertexAttributeTypes::N, GLVertexAttributeTypes::T02>;
        }
    }
}

#endif /* defined(TrenchBroom_VertexSpec) */
