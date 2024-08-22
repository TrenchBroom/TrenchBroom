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

#include "ObjSerializer.h"

#include "Assets/Material.h"
#include "Ensure.h"
#include "IO/ExportOptions.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/BrushNode.h"
#include "Model/PatchNode.h"
#include "Model/Polyhedron.h"

#include "kdl/overload.h"

#include <fmt/format.h>

#include <iostream>
#include <utility>

namespace TrenchBroom::IO
{

std::ostream& operator<<(std::ostream& str, const ObjSerializer::IndexedVertex& vertex)
{
  str << " " << (vertex.vertex + 1u) << "/" << (vertex.uvCoords + 1u) << "/"
      << (vertex.normal + 1u);
  return str;
}

std::ostream& operator<<(std::ostream& str, const ObjSerializer::BrushObject& object)
{
  str << "o entity" << object.entityNo << "_brush" << object.brushNo << "\n";
  for (const auto& face : object.faces)
  {
    str << "usemtl " << face.materialName << "\n";
    str << "f";
    for (const auto& vertex : face.verts)
    {
      str << " " << vertex;
    }
    str << "\n";
  }
  return str;
}

std::ostream& operator<<(std::ostream& str, const ObjSerializer::PatchObject& object)
{
  str << "o entity" << object.entityNo << "_patch" << object.patchNo << "\n";
  str << "usemtl " << object.materialName << "\n";
  for (const auto& quad : object.quads)
  {
    str << "f";
    for (const auto& vertex : quad.verts)
    {
      str << " " << vertex;
    }
    str << "\n";
  }
  return str;
}

std::ostream& operator<<(std::ostream& str, const ObjSerializer::Object& object)
{
  std::visit(
    kdl::overload(
      [&](const ObjSerializer::BrushObject& brushObject) { str << brushObject; },
      [&](const ObjSerializer::PatchObject& patchObject) { str << patchObject; }),
    object);
  return str;
}

ObjSerializer::ObjSerializer(
  std::ostream& objStream,
  std::ostream& mtlStream,
  std::string mtlFilename,
  IO::ObjExportOptions options)
  : m_objStream{objStream}
  , m_mtlStream{mtlStream}
  , m_mtlFilename{std::move(mtlFilename)}
  , m_options{std::move(options)}
{
  ensure(m_objStream.good(), "obj stream is good");
  ensure(m_mtlStream.good(), "mtl stream is good");
}

void ObjSerializer::doBeginFile(const std::vector<const Model::Node*>& /* rootNodes */) {}

static void writeMtlFile(
  std::ostream& str,
  const std::vector<ObjSerializer::Object>& objects,
  const IO::ObjExportOptions& options)
{
  auto usedMaterials = std::map<std::string, const Assets::Material*>{};

  for (const auto& object : objects)
  {
    std::visit(
      kdl::overload(
        [&](const ObjSerializer::BrushObject& brushObject) {
          for (const auto& face : brushObject.faces)
          {
            usedMaterials[face.materialName] = face.material;
          }
        },
        [&](const ObjSerializer::PatchObject& patchObject) {
          usedMaterials[patchObject.materialName] = patchObject.material;
        }),
      object);
  }

  const auto basePath = options.exportPath.parent_path();
  for (const auto& [materialName, material] : usedMaterials)
  {
    str << "newmtl " << materialName << "\n";
    if (material)
    {
      switch (options.mtlPathMode)
      {
      case ObjMtlPathMode::RelativeToGamePath:
        str << "map_Kd " << material->relativePath().generic_string() << "\n";
        break;
      case ObjMtlPathMode::RelativeToExportPath:
        // materials loaded from image files (pak files) don't have absolute paths
        if (!material->absolutePath().empty())
        {
          const auto mtlPath = material->absolutePath().lexically_relative(basePath);
          str << "map_Kd " << mtlPath.generic_string() << "\n";
        }
        break;
      }
    }
    str << "\n";
  }
}

static void writeVertices(std::ostream& str, const std::vector<vm::vec3>& vertices)
{
  str << "# vertices\n";
  for (const auto& elem : vertices)
  {
    // no idea why I have to switch Y and Z
    fmt::format_to(
      std::ostreambuf_iterator<char>(str), "v {} {} {}\n", elem.x(), elem.z(), -elem.y());
  }
}

static void writeUVCoords(std::ostream& str, const std::vector<vm::vec2f>& uvCoords)
{
  str << "# texture coordinates\n";
  for (const auto& elem : uvCoords)
  {
    // multiplying Y by -1 needed to get the UV's to appear correct in Blender and UE4
    // (see: https://github.com/TrenchBroom/TrenchBroom/issues/2851 )
    fmt::format_to(
      std::ostreambuf_iterator<char>(str), "vt {} {}\n", elem.x(), -elem.y());
  }
}

static void writeNormals(std::ostream& str, const std::vector<vm::vec3>& normals)
{
  str << "# normals\n";
  for (const auto& elem : normals)
  {
    // no idea why I have to switch Y and Z
    fmt::format_to(
      std::ostreambuf_iterator<char>(str),
      "vn {} {} {}\n",
      elem.x(),
      elem.z(),
      -elem.y());
  }
}

static void writeObjFile(
  std::ostream& str,
  const std::string mtlFilename,
  const std::vector<vm::vec3>& vertices,
  const std::vector<vm::vec2f>& uvCoords,
  const std::vector<vm::vec3>& normals,
  const std::vector<ObjSerializer::Object>& objects)
{

  str << "mtllib " << mtlFilename << "\n";
  writeVertices(str, vertices);
  str << "\n";
  writeUVCoords(str, uvCoords);
  str << "\n";
  writeNormals(str, normals);
  str << "\n";

  for (const auto& object : objects)
  {
    str << object;
    str << "\n";
  }
}

void ObjSerializer::doEndFile()
{
  writeMtlFile(m_mtlStream, m_objects, m_options);
  writeObjFile(
    m_objStream,
    m_mtlFilename,
    m_vertices.list(),
    m_uvCoords.list(),
    m_normals.list(),
    m_objects);
}

void ObjSerializer::doBeginEntity(const Model::Node*) {}
void ObjSerializer::doEndEntity(const Model::Node*) {}
void ObjSerializer::doEntityProperty(const Model::EntityProperty&) {}

void ObjSerializer::doBrush(const Model::BrushNode* brush)
{
  m_currentBrush = BrushObject{entityNo(), brushNo(), {}};
  m_currentBrush->faces.reserve(brush->brush().faceCount());

  // Vertex positions inserted from now on should get new indices
  m_vertices.clearIndices();

  for (const auto& face : brush->brush().faces())
  {
    doBrushFace(face);
  }

  m_objects.emplace_back(std::move(*m_currentBrush));
  m_currentBrush = std::nullopt;
}

void ObjSerializer::doBrushFace(const Model::BrushFace& face)
{
  const auto& normal = face.boundary().normal;
  const auto normalIndex = m_normals.index(normal);

  auto indexedVertices = std::vector<IndexedVertex>{};
  indexedVertices.reserve(face.vertexCount());

  for (const auto* vertex : face.vertices())
  {
    const auto& position = vertex->position();
    const auto uvCoords = face.uvCoords(position);

    const auto vertexIndex = m_vertices.index(position);
    const auto uvCoordsIndex = m_uvCoords.index(uvCoords);

    indexedVertices.push_back(IndexedVertex{vertexIndex, uvCoordsIndex, normalIndex});
  }

  m_currentBrush->faces.emplace_back(BrushFace{
    std::move(indexedVertices), face.attributes().materialName(), face.material()});
}

void ObjSerializer::doPatch(const Model::PatchNode* patchNode)
{
  const auto& patch = patchNode->patch();
  auto patchObject =
    PatchObject{entityNo(), brushNo(), {}, patch.materialName(), patch.material()};

  const auto& patchGrid = patchNode->grid();
  patchObject.quads.reserve(patchGrid.quadRowCount() * patchGrid.quadColumnCount());

  // Vertex positions inserted from now on should get new indices
  m_vertices.clearIndices();

  const auto makeIndexedVertex = [&](const auto& p) {
    const auto positionIndex = m_vertices.index(p.position);
    const auto uvCoordsIndex = m_uvCoords.index(vm::vec2f{p.uvCoords});
    const auto normalIndex = m_normals.index(p.normal);

    return IndexedVertex{positionIndex, uvCoordsIndex, normalIndex};
  };

  for (size_t row = 0u; row < patchGrid.pointRowCount - 1u; ++row)
  {
    for (size_t col = 0u; col < patchGrid.pointColumnCount - 1u; ++col)
    {
      // counter clockwise order
      patchObject.quads.push_back(PatchQuad{{
        makeIndexedVertex(patchGrid.point(row, col)),
        makeIndexedVertex(patchGrid.point(row + 1u, col)),
        makeIndexedVertex(patchGrid.point(row + 1u, col + 1u)),
        makeIndexedVertex(patchGrid.point(row, col + 1u)),
      }});
    }
  }

  m_objects.emplace_back(std::move(patchObject));
}

} // namespace TrenchBroom::IO
