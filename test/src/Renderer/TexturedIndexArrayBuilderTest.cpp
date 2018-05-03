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

#include "CollectionUtils.h"
#include "Assets/Texture.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/World.h"
#include "Model/MapFormat.h"
#include "Renderer/TexturedIndexArrayBuilder.h"
#include "Renderer/TexturedIndexArrayMap.h"
#include "Renderer/BrushRenderer.h"

#include <vector>
#include <chrono>
#include <string>
#include <iostream>

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

        static constexpr size_t NumBrushes = 64'000;
        static std::vector<Model::Brush*> makeBrushes() {
            const BBox3 worldBounds(4096.0);
            Model::World world(Model::MapFormat::Standard, nullptr, worldBounds);

            Model::BrushBuilder builder(&world, worldBounds);

            std::vector<Model::Brush*> result;
            for (size_t i = 0; i < NumBrushes; ++i) {
                result.push_back(builder.createCube(64.0, "texture"));
            }
            return result;
        }

        template<class L>
        static void timeLambda(L&& lambda, const std::string& message) {
            const auto start = std::chrono::high_resolution_clock::now();
            lambda();
            const auto end = std::chrono::high_resolution_clock::now();

            printf("Time elapsed for '%s': %fms\n", message.c_str(),
                   std::chrono::duration<double>(end - start).count() * 1000.0);
        }

        TEST(TexturedIndexArrayBuilderTest, benchBrushRenderer) {


            std::vector<Model::Brush*> brushes = makeBrushes();
            {
                BrushRenderer r(false);

                // ensure the brushes have their vertices cached.
                // we're not benchmarking that, so we don't
                // want it mixed into the timing
                r.addBrushes(brushes);
                r.validate();
                r.clear();

                r.addBrushes(brushes);
                timeLambda([&](){ r.validate(); }, "validate");
            }

            VectorUtils::clearAndDelete(brushes);
        }
    }
}

