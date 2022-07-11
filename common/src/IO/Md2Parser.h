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
namespace Assets {
class Palette;
}

namespace IO {
class FileSystem;
class Reader;

namespace Md2Layout {
static const int Ident = (('2' << 24) + ('P' << 16) + ('D' << 8) + 'I');
static const int Version = 8;
static const size_t SkinNameLength = 64;
static const size_t FrameNameLength = 16;
} // namespace Md2Layout

// see http://tfc.duke.free.fr/coding/md2-specs-en.html
class Md2Parser : public EntityModelParser {
private:
  static const vm::vec3f Normals[162];

  using Md2SkinList = std::vector<std::string>;

  struct Md2Vertex {
    unsigned char x, y, z;
    unsigned char normalIndex;
  };
  using Md2VertexList = std::vector<Md2Vertex>;

  struct Md2Frame {
    vm::vec3f scale;
    vm::vec3f offset;
    std::string name;
    Md2VertexList vertices;

    explicit Md2Frame(size_t vertexCount);
    vm::vec3f vertex(size_t index) const;
    const vm::vec3f& normal(size_t index) const;
  };

  struct Md2MeshVertex {
    vm::vec2f texCoords;
    size_t vertexIndex;
  };
  using Md2MeshVertexList = std::vector<Md2MeshVertex>;

  struct Md2Mesh {
    enum Type {
      Fan,
      Strip
    };

    Type type;
    size_t vertexCount;
    Md2MeshVertexList vertices;

    explicit Md2Mesh(int i_vertexCount);
  };
  using Md2MeshList = std::vector<Md2Mesh>;

  std::string m_name;
  const Reader& m_reader;
  const Assets::Palette& m_palette;
  const FileSystem& m_fs;

public:
  Md2Parser(
    const std::string& name, const Reader& reader, const Assets::Palette& palette,
    const FileSystem& fs);

private:
  std::unique_ptr<Assets::EntityModel> doInitializeModel(Logger& logger) override;
  void doLoadFrame(size_t frameIndex, Assets::EntityModel& model, Logger& logger) override;

  Md2SkinList parseSkins(Reader reader, size_t skinCount);
  Md2Frame parseFrame(Reader reader, size_t frameIndex, size_t vertexCount);
  Md2MeshList parseMeshes(Reader reader, size_t commandCount);

  void loadSkins(Assets::EntityModelSurface& surface, const Md2SkinList& skins, Logger& logger);

  void buildFrame(
    Assets::EntityModel& model, Assets::EntityModelSurface& surface, size_t frameIndex,
    const Md2Frame& frame, const Md2MeshList& meshes);
  std::vector<Assets::EntityModelVertex> getVertices(
    const Md2Frame& frame, const Md2MeshVertexList& meshVertices) const;
};
} // namespace IO
} // namespace TrenchBroom
