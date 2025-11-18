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

#include "ImageSpriteLoader.h"

#include "io/File.h"
#include "io/MaterialUtils.h"
#include "io/ReadFreeImageTexture.h"
#include "io/ReaderException.h"
#include "mdl/EntityModel.h"
#include "mdl/Material.h"
#include "mdl/Texture.h"
#include "render/IndexRangeMapBuilder.h"
#include "render/PrimType.h"

#include "kd/result.h"

#include "vm/vec.h"

#include <vector>

namespace tb::io
{
namespace
{

auto loadMaterial(const FileSystem& fs, File& file, std::string name, Logger& logger)
{
  auto reader = file.reader().buffer();
  return readFreeImageTexture(reader)
         | kdl::or_else(makeReadTextureErrorHandler(fs, logger))
         | kdl::and_then([&](auto texture) {
             auto textureResource = createTextureResource(std::move(texture));
             return Result<mdl::Material>{
               mdl::Material{std::move(name), std::move(textureResource)}};
           });
}

void createFrame(mdl::EntityModelData& modelData)
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

    const auto triangles = std::vector<mdl::EntityModelVertex>{
      mdl::EntityModelVertex{{x1, y1, 0}, {0, 1}},
      mdl::EntityModelVertex{{x1, y2, 0}, {0, 0}},
      mdl::EntityModelVertex{{x2, y2, 0}, {1, 0}},

      mdl::EntityModelVertex{{x2, y2, 0}, {1, 0}},
      mdl::EntityModelVertex{{x2, y1, 0}, {1, 1}},
      mdl::EntityModelVertex{{x1, y1, 0}, {0, 1}},
    };

    auto size = render::IndexRangeMap::Size{};
    size.inc(render::PrimType::Triangles, 2);

    auto builder = render::IndexRangeMapBuilder<mdl::EntityModelVertex::Type>{6, size};
    builder.addTriangles(triangles);

    surface.addMesh(frame, builder.vertices(), builder.indices());
  }
}

} // namespace

ImageSpriteLoader::ImageSpriteLoader(
  std::string name, std::shared_ptr<File> file, const FileSystem& fs)
  : m_name{std::move(name)}
  , m_file{std::move(file)}
  , m_fs{fs}
{
}

bool ImageSpriteLoader::canParse(const std::filesystem::path& path)
{
  return isSupportedFreeImageExtension(path.extension());
}

Result<mdl::EntityModelData> ImageSpriteLoader::load(Logger& logger)
{
  try
  {
    return loadMaterial(m_fs, *m_file, m_name, logger)
           | kdl::transform([&](auto material) {
               auto data = mdl::EntityModelData{
                 mdl::PitchType::Normal, mdl::Orientation::ViewPlaneParallel};

               auto& surface = data.addSurface(m_name, 1);
               surface.setSkins(kdl::vec_from(std::move(material)));

               createFrame(data);
               return data;
             });
  }
  catch (const ReaderException& e)
  {
    return Error{e.what()};
  }
}

} // namespace tb::io
