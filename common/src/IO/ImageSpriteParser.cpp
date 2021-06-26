/*
 Copyright (C) 2021 Kristian Duske

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

#include "ImageSpriteParser.h"

#include "FloatType.h"
#include "Assets/EntityModel.h"
#include "Assets/Texture.h"
#include "IO/TextureReader.h"
#include "IO/FreeImageTextureReader.h"
#include "Renderer/IndexRangeMapBuilder.h"
#include "Renderer/PrimType.h"

#include <vecmath/bbox.h>
#include <vecmath/vec.h>

#include <vector>

namespace TrenchBroom {
    namespace IO {
        ImageSpriteParser::ImageSpriteParser(std::string name, std::shared_ptr<File> file, const FileSystem& fs) :
        m_name{std::move(name)},
        m_file{std::move(file)},
        m_fs{fs} {}

        std::unique_ptr<Assets::EntityModel> ImageSpriteParser::doInitializeModel(Logger& logger) {
            auto textureReader = FreeImageTextureReader{TextureReader::StaticNameStrategy{m_name}, m_fs, logger};

            auto textures = std::vector<Assets::Texture>{};
            textures.push_back(textureReader.readTexture(m_file));

            auto model = std::make_unique<Assets::EntityModel>(m_name, Assets::PitchType::Normal, Assets::Orientation::ViewPlaneParallel);
            model->addFrames(1);

            auto& surface = model->addSurface(m_name);
            surface.setSkins(std::move(textures));

            return model;
        }

        void ImageSpriteParser::doLoadFrame(const size_t frameIndex, Assets::EntityModel& model, Logger&) {
            auto& surface = model.surface(0);

            if (const auto* texture = surface.skin(0)) {
                const auto w = static_cast<float>(texture->width());
                const auto h = static_cast<float>(texture->height());
                const auto x = w / 2.0f;
                const auto y = h / 2.0f;

                auto& frame = model.loadFrame(frameIndex, m_name, vm::bbox3f{vm::vec3f{-x, -y, 0}, vm::vec3f{x, y, 0}});

                const auto triangles = std::vector<Assets::EntityModelVertex>{
                    Assets::EntityModelVertex{{-x, -y, 0}, {0, 1}},
                    Assets::EntityModelVertex{{-x, +y, 0}, {0, 0}},
                    Assets::EntityModelVertex{{+x, +y, 0}, {1, 0}},
                    
                    Assets::EntityModelVertex{{+x, +y, 0}, {1, 0}},
                    Assets::EntityModelVertex{{+x, -y, 0}, {1, 1}},
                    Assets::EntityModelVertex{{-x, -y, 0}, {0, 1}},
                };

                auto size = Renderer::IndexRangeMap::Size{};
                size.inc(Renderer::PrimType::Triangles, 2);

                auto builder = Renderer::IndexRangeMapBuilder<Assets::EntityModelVertex::Type>{6, size};
                builder.addTriangles(triangles);

                surface.addIndexedMesh(frame, builder.vertices(), builder.indices());
            }
        }
    }
}
