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

#include "IO/SprParser.h"

#include "Color.h"
#include "Exceptions.h"
#include "Assets/EntityModel.h"
#include "Assets/Palette.h"
#include "Assets/Texture.h"
#include "IO/Reader.h"
#include "Renderer/IndexRangeMapBuilder.h"
#include "Renderer/PrimType.h"

#include <vecmath/bbox.h>
#include <vecmath/vec.h>

namespace TrenchBroom {
    namespace IO {
        SprParser::SprParser(std::string name, const char* begin, const char* end, const Assets::Palette& palette) :
        m_name{std::move(name)},
        m_begin{begin},
        m_end{end},
        m_palette{palette} {}

        struct SprPicture {
            Assets::Texture texture;
            int x;
            int y;
            size_t width;
            size_t height;
        };

        static SprPicture parsePicture(BufferedReader& reader, const Assets::Palette& palette) {
            const auto xOffset = reader.readInt<int32_t>();
            const auto yOffset = reader.readInt<int32_t>();
            const auto width = reader.readSize<int32_t>();
            const auto height = reader.readSize<int32_t>();

            Assets::TextureBuffer rgbaImage(4 * width * height);
            auto averageColor = Color{};
            palette.indexedToRgba(reader, width * height, rgbaImage, Assets::PaletteTransparency::Index255Transparent, averageColor);

            return SprPicture{
                {"", width, height, averageColor, std::move(rgbaImage), GL_RGBA, Assets::TextureType::Masked},
                xOffset,
                yOffset,
                width,
                height
            };
        }

        static void skipPicture(BufferedReader& reader) {
            /* const auto xOffset = */ reader.readInt<int32_t>();
            /* const auto yOffset = */ reader.readInt<int32_t>();
            const auto width = reader.readSize<int32_t>();
            const auto height = reader.readSize<int32_t>();

            reader.seekForward(width * height);
        }

        static SprPicture parsePictureFrame(BufferedReader& reader, const Assets::Palette& palette) {
            const auto group = reader.readInt<int32_t>();
            if (group == 0) { // single picture frame
                return parsePicture(reader, palette);
            }

            // multiple picture frame
            const auto pictureCount = reader.readSize<int32_t>();
            reader.seekForward(pictureCount * sizeof(float));
            
            auto picture = parsePicture(reader, palette);
            for (size_t i = 0; i < pictureCount - 1; ++i) {
                skipPicture(reader);
            }

            return picture;
        }

        static Assets::Orientation parseSpriteOrientationType(BufferedReader& reader) {
            const auto type = reader.readInt<int32_t>();
            if (type < 0 || type > 4) {
                throw AssetException{"Unknown SPR type: " + std::to_string(type)};
            }

            return static_cast<Assets::Orientation>(type);
        }

        std::unique_ptr<Assets::EntityModel> SprParser::doInitializeModel(Logger& /* logger */) {
            // see https://www.gamers.org/dEngine/quake/spec/quake-spec34/qkspec_6.htm#CSPRF

            auto reader = Reader::from(m_begin, m_end).buffer();

            const auto ident = reader.readString(4);
            if (ident != "IDSP") {
                throw AssetException{"Unknown SPR ident: " + ident};
            }

            const auto version = reader.readInt<int32_t>();
            if (version != 1) {
                throw AssetException{"Unknown SPR version: " + std::to_string(version)};
            }

            const auto orientationType = parseSpriteOrientationType(reader);
            /* const auto radius = */ reader.readFloat<float>();
            /* const auto maxWidth = */ reader.readSize<int32_t>();
            /* const auto maxHeight = */ reader.readSize<int32_t>();
            const auto frameCount = reader.readSize<int32_t>();
            /* const auto beamLength = */ reader.readFloat<float>();
            /* const auto synchtype = */ reader.readInt<int32_t>();

            auto model = std::make_unique<Assets::EntityModel>(m_name, Assets::PitchType::Normal, orientationType);
            for (size_t i = 0; i < frameCount; ++i) {
                auto& frame = model->addFrame();
                frame.setSkinOffset(i);
            }

            auto& surface = model->addSurface(m_name);

            auto textures = std::vector<Assets::Texture>{};
            textures.reserve(frameCount);

            for (size_t i = 0; i < frameCount; ++i) {
                auto pictureFrame = parsePictureFrame(reader, m_palette);
                textures.push_back(std::move(pictureFrame.texture));

                const auto w = static_cast<float>(pictureFrame.width);
                const auto h = static_cast<float>(pictureFrame.height);
                const auto x1 = static_cast<float>(pictureFrame.x);
                const auto y1 = -static_cast<float>(pictureFrame.y);
                const auto x2 = x1 + w;
                const auto y2 = y1 + h;

                auto& modelFrame = model->loadFrame(i, std::to_string(i), {
                    vm::vec3f{x1, y1, 0},
                    vm::vec3f{x2, y2, 0}
                });

                const auto triangles = std::vector<Assets::EntityModelVertex>{
                    Assets::EntityModelVertex{{x1, y1, 0}, {0, 1}},
                    Assets::EntityModelVertex{{x1, y2, 0}, {0, 0}},
                    Assets::EntityModelVertex{{x2, y2, 0}, {1, 0}},
                    
                    Assets::EntityModelVertex{{x2, y2, 0}, {1, 0}},
                    Assets::EntityModelVertex{{x2, y1, 0}, {1, 1}},
                    Assets::EntityModelVertex{{x1, y1, 0}, {0, 1}},
                };

                auto size = Renderer::IndexRangeMap::Size{};
                size.inc(Renderer::PrimType::Triangles, 2);

                auto builder = Renderer::IndexRangeMapBuilder<Assets::EntityModelVertex::Type>{6, size};
                builder.addTriangles(triangles);

                surface.addIndexedMesh(modelFrame, builder.vertices(), builder.indices());
            }

            surface.setSkins(std::move(textures));

            return model;
        }

        void SprParser::doLoadFrame(const size_t /* frameIndex */, Assets::EntityModel& /* model */, Logger& /* logger */) {
            // already loaded everything in doInitializeModel
        }
    }
}
