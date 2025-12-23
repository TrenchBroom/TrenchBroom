/*
 Copyright (C) 2025 Kristian Duske

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

#include "LoadImageSpriteModel.h"

#include "fs/ReaderException.h"
#include "mdl/LoadFreeImageTexture.h"
#include "mdl/MaterialUtils.h"
#include "render/IndexRangeMap.h"
#include "render/IndexRangeMapBuilder.h"

namespace tb::mdl
{
namespace
{

auto loadMaterial(
  const fs::FileSystem& fs, fs::Reader& reader, std::string name, Logger& logger)
{
  return loadFreeImageTexture(reader)
         | kdl::or_else(makeReadTextureErrorHandler(fs, logger))
         | kdl::and_then([&](auto texture) {
             auto textureResource = createTextureResource(std::move(texture));
             return Result<Material>{
               Material{std::move(name), std::move(textureResource)}};
           });
}

void createFrame(EntityModelData& modelData)
{
  auto& surface = modelData.surface(0);

  if (const auto* texture = getTexture(surface.skin(0)))
  {
    const auto textureSize = texture->sizef();
    const auto w = textureSize.x();
    const auto h = textureSize.y();
    const auto x1 = -w / 2.0f;
    const auto y1 = -h / 2.0f;
    const auto x2 = x1 + w;
    const auto y2 = y1 + h;


    const auto bboxMin = vm::vec3f{vm::min(x1, x2), vm::min(x1, x2), vm::min(y1, y2)};
    const auto bboxMax = vm::vec3f{vm::max(x1, x2), vm::max(x1, x2), vm::max(y1, y2)};
    auto& frame = modelData.addFrame("frame", {bboxMin, bboxMax});

    const auto triangles = std::vector<EntityModelVertex>{
      EntityModelVertex{{x1, y1, 0}, {0, 1}},
      EntityModelVertex{{x1, y2, 0}, {0, 0}},
      EntityModelVertex{{x2, y2, 0}, {1, 0}},

      EntityModelVertex{{x2, y2, 0}, {1, 0}},
      EntityModelVertex{{x2, y1, 0}, {1, 1}},
      EntityModelVertex{{x1, y1, 0}, {0, 1}},
    };

    auto size = render::IndexRangeMap::Size{};
    size.inc(render::PrimType::Triangles, 2);

    auto builder = render::IndexRangeMapBuilder<EntityModelVertex::Type>{6, size};
    builder.addTriangles(triangles);

    surface.addMesh(frame, builder.vertices(), builder.indices());
  }
}

} // namespace

bool canLoadImageSpriteModel(const std::filesystem::path& path)
{
  return isSupportedFreeImageExtension(path.extension());
}

Result<EntityModelData> loadImageSpriteModel(
  const std::string& name, fs::Reader reader, const fs::FileSystem& fs, Logger& logger)
{
  try
  {
    return loadMaterial(fs, reader, name, logger) | kdl::transform([&](auto material) {
             auto data =
               EntityModelData{PitchType::Normal, Orientation::ViewPlaneParallel};

             auto& surface = data.addSurface(name, 1);
             surface.setSkins(kdl::vec_from(std::move(material)));

             createFrame(data);
             return data;
           });
  }
  catch (const fs::ReaderException& e)
  {
    return Error{e.what()};
  }
}

} // namespace tb::mdl
