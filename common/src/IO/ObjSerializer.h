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

#include "FloatType.h"
#include "IO/ExportOptions.h"
#include "IO/NodeSerializer.h"

#include <vecmath/forward.h>

#include <array>
#include <iosfwd>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace TrenchBroom {
namespace Assets {
class Texture;
}

namespace Model {
class BrushNode;
class BrushFace;
class EntityProperty;
class Node;
} // namespace Model

namespace IO {
class ObjSerializer : public NodeSerializer {
public:
  template <typename V> class IndexMap {
  private:
    std::map<V, size_t> m_map;
    std::vector<V> m_list;

  public:
    const std::vector<V>& list() const { return m_list; }

    size_t index(const V& v) {
      const auto it = m_map.emplace(v, m_list.size()).first;
      const size_t index = it->second;
      if (index == m_list.size()) {
        m_list.push_back(v);
      }
      return index;
    }

    /**
     * Values inserted after this is called will not reuse indices from before this
     * is called.
     */
    void clearIndices() { m_map.clear(); }
  };

  struct IndexedVertex {
    size_t vertex;
    size_t texCoords;
    size_t normal;
  };

  struct BrushFace {
    std::vector<IndexedVertex> verts;
    std::string textureName;
    const Assets::Texture* texture;
  };

  struct BrushObject {
    size_t entityNo;
    size_t brushNo;
    std::vector<BrushFace> faces;
  };

  struct PatchQuad {
    std::array<IndexedVertex, 4u> verts;
  };

  struct PatchObject {
    size_t entityNo;
    size_t patchNo;
    std::vector<PatchQuad> quads;
    std::string textureName;
    const Assets::Texture* texture;
  };

  using Object = std::variant<BrushObject, PatchObject>;

  friend std::ostream& operator<<(std::ostream& str, const IndexedVertex& vertex);
  friend std::ostream& operator<<(std::ostream& str, const BrushFace& face);
  friend std::ostream& operator<<(std::ostream& str, const BrushObject& object);
  friend std::ostream& operator<<(std::ostream& str, const PatchQuad& quad);
  friend std::ostream& operator<<(std::ostream& str, const PatchObject& object);
  friend std::ostream& operator<<(std::ostream& str, const Object& object);

private:
  std::ostream& m_objStream;
  std::ostream& m_mtlStream;
  std::string m_mtlFilename;
  ObjExportOptions m_options;

  IndexMap<vm::vec3> m_vertices;
  IndexMap<vm::vec2f> m_texCoords;
  IndexMap<vm::vec3> m_normals;

  std::optional<BrushObject> m_currentBrush;
  std::vector<Object> m_objects;

public:
  ObjSerializer(
    std::ostream& objStream, std::ostream& mtlStream, std::string mtlFilename,
    ObjExportOptions options);

private:
  void doBeginFile(const std::vector<const Model::Node*>& rootNodes) override;
  void doEndFile() override;

  void doBeginEntity(const Model::Node* node) override;
  void doEndEntity(const Model::Node* node) override;
  void doEntityProperty(const Model::EntityProperty& property) override;

  void doBrush(const Model::BrushNode* brush) override;
  void doBrushFace(const Model::BrushFace& face) override;

  void doPatch(const Model::PatchNode* patchNode) override;
};
} // namespace IO
} // namespace TrenchBroom
