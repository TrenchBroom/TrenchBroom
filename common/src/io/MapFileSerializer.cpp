/*
 Copyright (C) 2010 Kristian Duske

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

#include "MapFileSerializer.h"

#include "Ensure.h"
#include "Exceptions.h"
#include "Macros.h"
#include "mdl/BezierPatch.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/EntityProperties.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"

#include "kdl/overload.h"
#include "kdl/string_format.h"
#include "kdl/task_manager.h"

#include <fmt/format.h>

#include <iterator>
#include <memory>
#include <sstream>
#include <utility>
#include <variant>
#include <vector>

namespace tb::io
{

class QuakeFileSerializer : public MapFileSerializer
{
public:
  explicit QuakeFileSerializer(std::ostream& stream)
    : MapFileSerializer(stream)
  {
  }

private:
  void doWriteBrushFace(std::ostream& stream, const mdl::BrushFace& face) const override
  {
    writeFacePoints(stream, face);
    writeMaterialInfo(stream, face);
    fmt::format_to(std::ostreambuf_iterator<char>(stream), "\n");
  }

protected:
  void writeFacePoints(std::ostream& stream, const mdl::BrushFace& face) const
  {
    const mdl::BrushFace::Points& points = face.points();

    fmt::format_to(
      std::ostreambuf_iterator<char>(stream),
      "( {} {} {} ) ( {} {} {} ) ( {} {} {} )",
      points[0].x(),
      points[0].y(),
      points[0].z(),
      points[1].x(),
      points[1].y(),
      points[1].z(),
      points[2].x(),
      points[2].y(),
      points[2].z());
  }

  static bool shouldQuoteMaterialName(const std::string& materialName)
  {
    return materialName.empty()
           || materialName.find_first_of("\"\\ \t") != std::string::npos;
  }

  static std::string quoteMaterialName(const std::string& materialName)
  {
    return "\"" + kdl::str_escape(materialName, "\"") + "\"";
  }

  void writeMaterialInfo(std::ostream& stream, const mdl::BrushFace& face) const
  {
    const std::string& materialName = face.attributes().materialName().empty()
                                        ? mdl::BrushFaceAttributes::NoMaterialName
                                        : face.attributes().materialName();

    fmt::format_to(
      std::ostreambuf_iterator<char>(stream),
      " {} {} {} {} {} {}",
      shouldQuoteMaterialName(materialName) ? quoteMaterialName(materialName)
                                            : materialName,
      face.attributes().xOffset(),
      face.attributes().yOffset(),
      face.attributes().rotation(),
      face.attributes().xScale(),
      face.attributes().yScale());
  }

  void writeValveMaterialInfo(std::ostream& stream, const mdl::BrushFace& face) const
  {
    const std::string& materialName = face.attributes().materialName().empty()
                                        ? mdl::BrushFaceAttributes::NoMaterialName
                                        : face.attributes().materialName();
    const vm::vec3d uAxis = face.uAxis();
    const vm::vec3d vAxis = face.vAxis();

    fmt::format_to(
      std::ostreambuf_iterator<char>(stream),
      " {} [ {} {} {} {} ] [ {} {} {} {} ] {} {} {}",
      shouldQuoteMaterialName(materialName) ? quoteMaterialName(materialName)
                                            : materialName,

      uAxis.x(),
      uAxis.y(),
      uAxis.z(),
      face.attributes().xOffset(),

      vAxis.x(),
      vAxis.y(),
      vAxis.z(),
      face.attributes().yOffset(),

      face.attributes().rotation(),
      face.attributes().xScale(),
      face.attributes().yScale());
  }
};

class Quake2FileSerializer : public QuakeFileSerializer
{
public:
  explicit Quake2FileSerializer(std::ostream& stream)
    : QuakeFileSerializer(stream)
  {
  }

private:
  void doWriteBrushFace(std::ostream& stream, const mdl::BrushFace& face) const override
  {
    writeFacePoints(stream, face);
    writeMaterialInfo(stream, face);

    if (face.attributes().hasSurfaceAttributes())
    {
      writeSurfaceAttributes(stream, face);
    }

    fmt::format_to(std::ostreambuf_iterator<char>(stream), "\n");
  }

protected:
  void writeSurfaceAttributes(std::ostream& stream, const mdl::BrushFace& face) const
  {
    fmt::format_to(
      std::ostreambuf_iterator<char>(stream),
      " {} {} {}",
      face.resolvedSurfaceContents(),
      face.resolvedSurfaceFlags(),
      face.resolvedSurfaceValue());
  }
};

class Quake2ValveFileSerializer : public Quake2FileSerializer
{
public:
  explicit Quake2ValveFileSerializer(std::ostream& stream)
    : Quake2FileSerializer(stream)
  {
  }

private:
  void doWriteBrushFace(std::ostream& stream, const mdl::BrushFace& face) const override
  {
    writeFacePoints(stream, face);
    writeValveMaterialInfo(stream, face);

    if (face.attributes().hasSurfaceAttributes())
    {
      writeSurfaceAttributes(stream, face);
    }

    fmt::format_to(std::ostreambuf_iterator<char>(stream), "\n");
  }
};

class DaikatanaFileSerializer : public Quake2FileSerializer
{
private:
  std::string SurfaceColorFormat;

public:
  explicit DaikatanaFileSerializer(std::ostream& stream)
    : Quake2FileSerializer(stream)
    , SurfaceColorFormat(" %d %d %d")
  {
  }

private:
  void doWriteBrushFace(std::ostream& stream, const mdl::BrushFace& face) const override
  {
    writeFacePoints(stream, face);
    writeMaterialInfo(stream, face);

    if (face.attributes().hasSurfaceAttributes() || face.attributes().hasColor())
    {
      writeSurfaceAttributes(stream, face);
    }
    if (face.attributes().hasColor())
    {
      writeSurfaceColor(stream, face);
    }

    fmt::format_to(std::ostreambuf_iterator<char>(stream), "\n");
  }

protected:
  void writeSurfaceColor(std::ostream& stream, const mdl::BrushFace& face) const
  {
    fmt::format_to(
      std::ostreambuf_iterator<char>(stream),
      " {} {} {}",
      static_cast<int>(face.resolvedColor().r()),
      static_cast<int>(face.resolvedColor().g()),
      static_cast<int>(face.resolvedColor().b()));
  }
};

class Hexen2FileSerializer : public QuakeFileSerializer
{
public:
  explicit Hexen2FileSerializer(std::ostream& stream)
    : QuakeFileSerializer(stream)
  {
  }

private:
  void doWriteBrushFace(std::ostream& stream, const mdl::BrushFace& face) const override
  {
    writeFacePoints(stream, face);
    writeMaterialInfo(stream, face);
    fmt::format_to(
      std::ostreambuf_iterator<char>(stream), " 0\n"); // extra value written here
  }
};

class ValveFileSerializer : public QuakeFileSerializer
{
public:
  explicit ValveFileSerializer(std::ostream& stream)
    : QuakeFileSerializer(stream)
  {
  }

private:
  void doWriteBrushFace(std::ostream& stream, const mdl::BrushFace& face) const override
  {
    writeFacePoints(stream, face);
    writeValveMaterialInfo(stream, face);
    fmt::format_to(std::ostreambuf_iterator<char>(stream), "\n");
  }
};

std::unique_ptr<NodeSerializer> MapFileSerializer::create(
  const mdl::MapFormat format, std::ostream& stream)
{
  switch (format)
  {
  case mdl::MapFormat::Standard:
    return std::make_unique<QuakeFileSerializer>(stream);
  case mdl::MapFormat::Quake2:
    // TODO 2427: Implement Quake3 serializers and use them
  case mdl::MapFormat::Quake3:
  case mdl::MapFormat::Quake3_Legacy:
    return std::make_unique<Quake2FileSerializer>(stream);
  case mdl::MapFormat::Quake2_Valve:
  case mdl::MapFormat::Quake3_Valve:
    return std::make_unique<Quake2ValveFileSerializer>(stream);
  case mdl::MapFormat::Daikatana:
    return std::make_unique<DaikatanaFileSerializer>(stream);
  case mdl::MapFormat::Valve:
    return std::make_unique<ValveFileSerializer>(stream);
  case mdl::MapFormat::Hexen2:
    return std::make_unique<Hexen2FileSerializer>(stream);
  case mdl::MapFormat::Unknown:
    throw FileFormatException("Unknown map file format");
    switchDefault();
  }
}

MapFileSerializer::MapFileSerializer(std::ostream& stream)
  : m_line(1)
  , m_stream(stream)
{
}

void MapFileSerializer::doBeginFile(
  const std::vector<const mdl::Node*>& rootNodes, kdl::task_manager& taskManager)
{
  ensure(m_nodeToPrecomputedString.empty(), "MapFileSerializer may not be reused");

  // collect nodes
  std::vector<std::variant<const mdl::BrushNode*, const mdl::PatchNode*>>
    nodesToSerialize;
  nodesToSerialize.reserve(rootNodes.size());

  mdl::Node::visitAll(
    rootNodes,
    kdl::overload(
      [](auto&& thisLambda, const mdl::WorldNode* world) {
        world->visitChildren(thisLambda);
      },
      [](auto&& thisLambda, const mdl::LayerNode* layer) {
        layer->visitChildren(thisLambda);
      },
      [](auto&& thisLambda, const mdl::GroupNode* group) {
        group->visitChildren(thisLambda);
      },
      [](auto&& thisLambda, const mdl::EntityNode* entity) {
        entity->visitChildren(thisLambda);
      },
      [&](const mdl::BrushNode* brush) { nodesToSerialize.emplace_back(brush); },
      [&](const mdl::PatchNode* patchNode) {
        nodesToSerialize.emplace_back(patchNode);
      }));

  // serialize brushes to strings in parallel
  using Entry = std::pair<const mdl::Node*, PrecomputedString>;
  auto tasks = nodesToSerialize | std::views::transform([&](const auto& node) {
                 return std::function{[&]() {
                   return std::visit(
                     kdl::overload(
                       [&](const mdl::BrushNode* brushNode) {
                         return Entry{brushNode, writeBrushFaces(brushNode->brush())};
                       },
                       [&](const mdl::PatchNode* patchNode) {
                         return Entry{patchNode, writePatch(patchNode->patch())};
                       }),
                     node);
                 }};
               });

  // render strings and move them into a map
  for (auto& entry : taskManager.run_tasks_and_wait(std::move(tasks)))
  {
    m_nodeToPrecomputedString.insert(std::move(entry));
  }
}

void MapFileSerializer::doEndFile() {}

void MapFileSerializer::doBeginEntity(const mdl::Node* /* node */)
{
  fmt::format_to(std::ostreambuf_iterator<char>(m_stream), "// entity {}\n", entityNo());
  ++m_line;
  m_startLineStack.push_back(m_line);
  fmt::format_to(std::ostreambuf_iterator<char>(m_stream), "{{\n");
  ++m_line;
}

void MapFileSerializer::doEndEntity(const mdl::Node* node)
{
  fmt::format_to(std::ostreambuf_iterator<char>(m_stream), "}}\n");
  ++m_line;
  setFilePosition(node);
}

void MapFileSerializer::doEntityProperty(const mdl::EntityProperty& attribute)
{
  fmt::format_to(
    std::ostreambuf_iterator<char>(m_stream),
    "\"{}\" \"{}\"\n",
    escapeEntityProperties(attribute.key()),
    escapeEntityProperties(attribute.value()));
  ++m_line;
}

void MapFileSerializer::doBrush(const mdl::BrushNode* brush)
{
  fmt::format_to(std::ostreambuf_iterator<char>(m_stream), "// brush {}\n", brushNo());
  ++m_line;
  m_startLineStack.push_back(m_line);
  fmt::format_to(std::ostreambuf_iterator<char>(m_stream), "{{\n");
  ++m_line;

  // write pre-serialized brush faces
  auto it = m_nodeToPrecomputedString.find(brush);
  ensure(
    it != std::end(m_nodeToPrecomputedString),
    "attempted to serialize a brush which was not passed to doBeginFile");
  const PrecomputedString& precomputedString = it->second;
  m_stream << precomputedString.string;
  m_line += precomputedString.lineCount;

  fmt::format_to(std::ostreambuf_iterator<char>(m_stream), "}}\n");
  ++m_line;
  setFilePosition(brush);
}

void MapFileSerializer::doBrushFace(const mdl::BrushFace& face)
{
  const size_t lines = 1u;
  doWriteBrushFace(m_stream, face);
  face.setFilePosition(m_line, lines);
  m_line += lines;
}

void MapFileSerializer::doPatch(const mdl::PatchNode* patchNode)
{
  fmt::format_to(std::ostreambuf_iterator<char>(m_stream), "// brush {}\n", brushNo());
  ++m_line;
  m_startLineStack.push_back(m_line);

  // write pre-serialized patch
  auto it = m_nodeToPrecomputedString.find(patchNode);
  ensure(
    it != std::end(m_nodeToPrecomputedString),
    "attempted to serialize a patch which was not passed to doBeginFile");
  const PrecomputedString& precomputedString = it->second;
  m_stream << precomputedString.string;
  m_line += precomputedString.lineCount;

  setFilePosition(patchNode);
}

void MapFileSerializer::setFilePosition(const mdl::Node* node)
{
  const size_t start = startLine();
  node->setFilePosition(start, m_line - start);
}

size_t MapFileSerializer::startLine()
{
  assert(!m_startLineStack.empty());
  const size_t result = m_startLineStack.back();
  m_startLineStack.pop_back();
  return result;
}

/**
 * Threadsafe
 */
MapFileSerializer::PrecomputedString MapFileSerializer::writeBrushFaces(
  const mdl::Brush& brush) const
{
  std::stringstream stream;
  for (const mdl::BrushFace& face : brush.faces())
  {
    doWriteBrushFace(stream, face);
  }
  return PrecomputedString{stream.str(), brush.faces().size()};
}

MapFileSerializer::PrecomputedString MapFileSerializer::writePatch(
  const mdl::BezierPatch& patch) const
{
  size_t lineCount = 0u;
  std::stringstream stream;

  fmt::format_to(std::ostreambuf_iterator<char>(stream), "{{\n");
  ++lineCount;
  fmt::format_to(std::ostreambuf_iterator<char>(stream), "patchDef2\n");
  ++lineCount;
  fmt::format_to(std::ostreambuf_iterator<char>(stream), "{{\n");
  ++lineCount;
  fmt::format_to(std::ostreambuf_iterator<char>(stream), "{}\n", patch.materialName());
  ++lineCount;
  fmt::format_to(
    std::ostreambuf_iterator<char>(stream),
    "( {} {} 0 0 0 )\n",
    patch.pointRowCount(),
    patch.pointColumnCount());
  ++lineCount;
  fmt::format_to(std::ostreambuf_iterator<char>(stream), "(\n");
  ++lineCount;

  for (size_t row = 0u; row < patch.pointRowCount(); ++row)
  {
    fmt::format_to(std::ostreambuf_iterator<char>(stream), "( ");
    for (size_t col = 0u; col < patch.pointColumnCount(); ++col)
    {
      const auto& p = patch.controlPoint(row, col);
      fmt::format_to(
        std::ostreambuf_iterator<char>(stream),
        "( {} {} {} {} {} ) ",
        p[0],
        p[1],
        p[2],
        p[3],
        p[4]);
    }
    fmt::format_to(std::ostreambuf_iterator<char>(stream), ")\n");
    ++lineCount;
  }

  fmt::format_to(std::ostreambuf_iterator<char>(stream), ")\n");
  ++lineCount;
  fmt::format_to(std::ostreambuf_iterator<char>(stream), "}}\n");
  ++lineCount;
  fmt::format_to(std::ostreambuf_iterator<char>(stream), "}}\n");
  ++lineCount;

  return PrecomputedString{stream.str(), lineCount};
}

} // namespace tb::io
