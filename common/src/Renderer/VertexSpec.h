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
#include "Renderer/AttributeSpec.h"
#include "Renderer/Vertex.h"

#include <vector>

#include <vecmath/forward.h>

namespace TrenchBroom {
    namespace Renderer {
        template <typename... Attrs>
        class VertexSpec;

        template <typename Attr, typename... Attrs>
        class VertexSpec<Attr, Attrs...> {
        public:
            using Vertex = Vertex<Attr, Attrs...>;
            static const size_t Size = sizeof(Vertex);

            static void setup(const size_t baseOffset) {
                doSetup(0, Size, baseOffset);
            }

            static void cleanup() {
                doCleanup(0);
            }

            static void doSetup(const size_t index, const size_t stride, const size_t offset) {
                Attr::setup(index, stride, offset);
                VertexSpec<Attrs...>::doSetup(index + 1, stride, offset + Attr::Size);
            }

            static void doCleanup(const size_t index) {
                VertexSpec<Attrs...>::doCleanup(index + 1);
                Attr::cleanup(index);
            }
        };

        template <typename Attr>
        class VertexSpec<Attr> {
        public:
            using Vertex = Vertex<Attr>;
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

        namespace VertexSpecs {
            using P2     = VertexSpec<AttributeSpecs::P2>;
            using P3     = VertexSpec<AttributeSpecs::P3>;
            using P2C4   = VertexSpec<AttributeSpecs::P2, AttributeSpecs::C4>;
            using P3C4   = VertexSpec<AttributeSpecs::P3, AttributeSpecs::C4>;
            using P2T2   = VertexSpec<AttributeSpecs::P2, AttributeSpecs::T02>;
            using P3T2   = VertexSpec<AttributeSpecs::P3, AttributeSpecs::T02>;
            using P2T2C4 = VertexSpec<AttributeSpecs::P2, AttributeSpecs::T02, AttributeSpecs::C4>;
            using P3T2C4 = VertexSpec<AttributeSpecs::P3, AttributeSpecs::T02, AttributeSpecs::C4>;
            using P3N    = VertexSpec<AttributeSpecs::P3, AttributeSpecs::N>;
            using P3NC4  = VertexSpec<AttributeSpecs::P3, AttributeSpecs::N, AttributeSpecs::C4>;
            using P3NT2  = VertexSpec<AttributeSpecs::P3, AttributeSpecs::N, AttributeSpecs::T02>;
        }
    }
}

#endif /* defined(TrenchBroom_VertexSpec) */
