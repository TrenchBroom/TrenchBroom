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

#include "Assets/EntityModel.h"
#include "Assets/Texture.h"
#include "FloatType.h"
#include "IO/File.h"
#include "IO/ReadFreeImageTexture.h"
#include "Renderer/IndexRangeMapBuilder.h"
#include "Renderer/PrimType.h"

#include "kdl/path_utils.h"
#include "kdl/result.h"
#include "kdl/string_format.h"

#include "vm/bbox.h"
#include "vm/vec.h"

#include <vector>

namespace TrenchBroom::IO
{
ImageSpriteParser::ImageSpriteParser(
  std::string name, std::shared_ptr<File> file, const FileSystem& fs)
  : m_name{std::move(name)}
  , m_file{std::move(file)}
  , m_fs{fs}
{
}

bool ImageSpriteParser::canParse(const std::filesystem::path& path)
{
  return isSupportedFreeImageExtension(path.extension().string());
}

std::unique_ptr<Assets::EntityModel> ImageSpriteParser::initializeModel(Logger& logger)
{
  auto textures = std::vector<Assets::Texture>{};

  auto reader = m_file->reader().buffer();
  textures.push_back(readFreeImageTexture(m_name, reader)
                       .or_else(makeReadTextureErrorHandler(m_fs, logger))
                       .value());

  auto model = std::make_unique<Assets::EntityModel>(
    m_name, Assets::PitchType::Normal, Assets::Orientation::ViewPlaneParallel);
  model->addFrame();

  auto& surface = model->addSurface(m_name);
  surface.setSkins(std::move(textures));

  return model;
}

void ImageSpriteParser::loadFrame(
  const size_t frameIndex, Assets::EntityModel& model, Logger&)
{
  auto& surface = model.surface(0);

  if (const auto* texture = surface.skin(0))
  {
    const auto w = static_cast<float>(texture->width());
    const auto h = static_cast<float>(texture->height());
    const auto x1 = -w / 2.0f;
    const auto y1 = -h / 2.0f;
    const auto x2 = x1 + w;
    const auto y2 = y1 + h;


    const auto bboxMin = vm::vec3f{vm::min(x1, x2), vm::min(x1, x2), vm::min(y1, y2)};
    const auto bboxMax = vm::vec3f{vm::max(x1, x2), vm::max(x1, x2), vm::max(y1, y2)};
    auto& frame = model.loadFrame(frameIndex, m_name, {bboxMin, bboxMax});

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

    auto builder =
      Renderer::IndexRangeMapBuilder<Assets::EntityModelVertex::Type>{6, size};
    builder.addTriangles(triangles);

    surface.addIndexedMesh(frame, builder.vertices(), builder.indices());
  }
}
} // namespace TrenchBroom::IO
