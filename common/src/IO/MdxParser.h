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

#pragma once

#include "Assets/EntityModel_Forward.h"
#include "IO/EntityModelParser.h"

#include <vecmath/forward.h>
#include <vecmath/vec.h>

#include <string>
#include <vector>

namespace TrenchBroom {
class Logger;

namespace IO {
class FileSystem;
class Path;
class Reader;

namespace MdxLayout {
static const int Ident = (('X' << 24) + ('P' << 16) + ('D' << 8) + 'I');
static const int Version = 4;
static const size_t SkinNameLength = 64;
static const size_t FrameNameLength = 16;
} // namespace MdxLayout

// see https://web.archive.org/web/20020404103848/http://members.cheapnet.co.uk/~tical/misc/mdx.htm
class MdxParser : public EntityModelParser {
private:
  static const vm::vec3f Normals[162];

  using MdxSkinList = std::vector<std::string>;

  struct MdxVertex {
    unsigned char x, y, z;
    unsigned char normalIndex;
  };
  using MdxVertexList = std::vector<MdxVertex>;

  struct MdxFrame {
    vm::vec3f scale;
    vm::vec3f offset;
    std::string name;
    MdxVertexList vertices;

    explicit MdxFrame(size_t vertexCount);
    vm::vec3f vertex(size_t index) const;
    const vm::vec3f& normal(size_t index) const;
  };

  struct MdxMeshVertex {
    vm::vec2f texCoords;
    size_t vertexIndex;
  };
  using MdxMeshVertexList = std::vector<MdxMeshVertex>;

  struct MdxMesh {
    enum Type {
      Fan,
      Strip
    };

    Type type;
    size_t vertexCount;
    MdxMeshVertexList vertices;

    explicit MdxMesh(int i_vertexCount);
  };
  using MdxMeshList = std::vector<MdxMesh>;

  std::string m_name;
  const Reader& m_reader;
  const FileSystem& m_fs;

public:
  MdxParser(const std::string& name, const Reader& reader, const FileSystem& fs);

  static bool canParse(const Path& path, Reader reader);

private:
  std::unique_ptr<Assets::EntityModel> doInitializeModel(Logger& logger) override;
  void doLoadFrame(size_t frameIndex, Assets::EntityModel& model, Logger& logger) override;

  MdxSkinList parseSkins(Reader reader, size_t skinCount);
  MdxFrame parseFrame(Reader reader, size_t frameIndex, size_t vertexCount);
  MdxMeshList parseMeshes(Reader reader, size_t commandCount);

  void loadSkins(Assets::EntityModelSurface& surface, const MdxSkinList& skins, Logger& logger);

  void buildFrame(
    Assets::EntityModel& model, Assets::EntityModelSurface& surface, size_t frameIndex,
    const MdxFrame& frame, const MdxMeshList& meshes);
  std::vector<Assets::EntityModelVertex> getVertices(
    const MdxFrame& frame, const MdxMeshVertexList& meshVertices) const;
};
} // namespace IO
} // namespace TrenchBroom
