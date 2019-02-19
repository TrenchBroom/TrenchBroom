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
#include "Renderer/BrushRenderer.h"

#include <vector>
#include <chrono>
#include <string>
#include <iostream>
#include <tuple>
#include <algorithm>

namespace TrenchBroom {
    namespace Renderer {
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
            const vm::bbox3 worldBounds(4096.0);
            Model::World world(Model::MapFormat::Standard, worldBounds);

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

            BrushRenderer tempRenderer;
            tempRenderer.addBrushes(result);
            tempRenderer.validate();
            tempRenderer.clear();

            return {result, textures};
        }

#ifdef __GNUC__
#define TB_NOINLINE __attribute__((noinline))
#else
#define TB_NOINLINE
#endif

        // the noinline is so you can see the timeLambda when profiling
        template<class L>
        TB_NOINLINE static void timeLambda(L&& lambda, const std::string& message) {
            const auto start = std::chrono::high_resolution_clock::now();
            lambda();
            const auto end = std::chrono::high_resolution_clock::now();

            printf("Time elapsed for '%s': %fms\n", message.c_str(),
                   std::chrono::duration<double>(end - start).count() * 1000.0);
        }

        TEST(BrushRendererBenchmark, benchBrushRenderer) {
            auto brushesTextures = makeBrushes();
            std::vector<Model::Brush*> brushes = brushesTextures.first;
            std::vector<Assets::Texture*> textures = brushesTextures.second;

            BrushRenderer r;

            timeLambda([&](){ r.addBrushes(brushes); }, "add " + std::to_string(brushes.size()) + " brushes to BrushRenderer");
            timeLambda([&](){
                if (!r.valid()) {
                    r.validate();
                }
            }, "validate after adding " + std::to_string(brushes.size()) + " brushes to BrushRenderer");

            // Tiny change: remove the last brush
            std::vector<Model::Brush*> brushesMinusOne = brushes;
            brushesMinusOne.resize(brushes.size() - 1);

            timeLambda([&](){ r.setBrushes(brushesMinusOne); }, "setBrushes to " + std::to_string(brushesMinusOne.size()) + " (removing one)");
            timeLambda([&](){
                if (!r.valid()) {
                    r.validate();
                }
            }, "validate after removing one brush");

            // Large change: keep every second brush
            Model::BrushList brushesToKeep;
            for (size_t i = 0; i < brushes.size(); ++i) {
                if ((i % 2) == 0) {
                    brushesToKeep.push_back(brushes.at(i));
                }
            }

            timeLambda([&](){ r.setBrushes(brushesToKeep); },
                       "set brushes from " + std::to_string(brushes.size()) +
                       " to " + std::to_string(brushesToKeep.size()));

            timeLambda([&](){
                           if (!r.valid()) {
                               r.validate();
                           }
                       }, "validate with " + std::to_string(brushesToKeep.size()) + " brushes");

            VectorUtils::clearAndDelete(brushes);
            VectorUtils::clearAndDelete(textures);
        }
    }
}

