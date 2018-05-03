/*
 Copyright (C) 2018 Eric Wasylishen
 
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

#include <gtest/gtest.h>

#include "Assets/Texture.h"
#include "Renderer/TexturedIndexArrayBuilder.h"
#include "Renderer/TexturedIndexArrayMap.h"
#include "Renderer/BrushRenderer.h"

namespace TrenchBroom {
    namespace Renderer {
        static constexpr size_t NumFaces = 1'000'000;

        TEST(TexturedIndexArrayBuilderTest, bench) {
            Assets::Texture texture1("testTexture1", 64, 64);

            TexturedIndexArrayMap::Size sizes;
            for (size_t i = 0; i < NumFaces; ++i) {
                sizes.inc(&texture1, GL_TRIANGLES, 6); // one quad = two tris
            }

            TexturedIndexArrayBuilder b(sizes);
            for (size_t i = 0; i < NumFaces; ++i) {
                b.addPolygon(&texture1, 0, 4); // one quad
            }
        }

        TEST(TexturedIndexArrayBuilderTest, benchBrushRenderer) {
            BrushRenderer r(false);

        }
    }
}

