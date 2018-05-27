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
#include <tuple>

namespace TrenchBroom {
    namespace Renderer {
        static constexpr size_t NumFaces = 1'000'000;

        TEST(TexturedIndexArrayBuilderTest, bench) {
            Assets::Texture texture1("testTexture1", 64, 64);

            TexturedIndexArrayMap::Size sizes;
            for (size_t i = 0; i < NumFaces; ++i) {
                sizes.incTriangles(&texture1, 6); // one quad = two tris
            }

            TexturedIndexArrayBuilder b(sizes);
            for (size_t i = 0; i < NumFaces; ++i) {
                b.addPolygon(&texture1, 0, 4); // one quad
            }
        }

        static constexpr size_t NumBrushes = 64'000;
        static constexpr size_t NumTextures = 256;

        /**
         * Both returned vectors need to be freed with VectorUtils::clearAndDelete
         */
        static std::pair<std::vector<Model::Brush*>, std::vector<Assets::Texture*>> makeBrushes() {
            // make textures
            std::vector<Assets::Texture*> textures;
            for (size_t i = 0; i < NumTextures; ++i) {
                const String textureName = "texture " + std::to_string(i);

                textures.push_back(new Assets::Texture(textureName, 64, 64));
            }

            // make brushes, cycling through the textures for each face
            const BBox3 worldBounds(4096.0);
            Model::World world(Model::MapFormat::Standard, nullptr, worldBounds);

            Model::BrushBuilder builder(&world, worldBounds);

            std::vector<Model::Brush*> result;
            size_t currentTextureIndex = 0;
            for (size_t i = 0; i < NumBrushes; ++i) {
                Model::Brush* brush = builder.createCube(64.0, "");
                for (auto* face : brush->faces()) {
                    face->setTexture(textures.at((currentTextureIndex++) % NumTextures));
                }
                result.push_back(brush);
            }

            // ensure the brushes have their vertices cached.
            // we're not benchmarking that, so we don't
            // want it mixed into the timing

            BrushRenderer tempRenderer(false);
            tempRenderer.addBrushes(result);
            tempRenderer.validate();
            tempRenderer.clear();

            return {result, textures};
        }

        // the noinline is so you can see the timeLambda when profiling
        template<class L>
        __attribute__ ((noinline)) static void timeLambda(L&& lambda, const std::string& message) {
            const auto start = std::chrono::high_resolution_clock::now();
            lambda();
            const auto end = std::chrono::high_resolution_clock::now();

            printf("Time elapsed for '%s': %fms\n", message.c_str(),
                   std::chrono::duration<double>(end - start).count() * 1000.0);
        }

        TEST(TexturedIndexArrayBuilderTest, benchBrushRenderer) {
            auto brushesTextures = makeBrushes();
            std::vector<Model::Brush*> brushes = brushesTextures.first;
            std::vector<Assets::Texture*> textures = brushesTextures.second;

            BrushRenderer r(false);

            timeLambda([&](){ r.addBrushes(brushes); }, "add " + std::to_string(brushes.size()) + " brushes to BrushRenderer");
            timeLambda([&](){ r.validate(); }, "validate after adding " + std::to_string(brushes.size()) + " brushes to BrushRenderer");

            // keep every second brush
            Model::BrushList brushesToKeep;
            for (size_t i = 0; i < brushes.size(); ++i) {
                if ((i % 2) == 0) {
                    brushesToKeep.push_back(brushes.at(i));
                }
            }

            timeLambda([&](){ r.setBrushes(brushesToKeep); },
                       "set brushes from " + std::to_string(brushes.size()) +
                       " to " + std::to_string(brushesToKeep.size()));

            timeLambda([&](){ r.validate(); },
                       "validate with " + std::to_string(brushesToKeep.size()) + " brushes");

            VectorUtils::clearAndDelete(brushes);
            VectorUtils::clearAndDelete(textures);
        }
    }
}

