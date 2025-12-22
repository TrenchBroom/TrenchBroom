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

#include "kd/contracts.h"
#include "kd/overload.h"
#include "kd/string_format.h"
#include "kd/task_manager.h"

#include <fmt/format.h>

#include <iterator>
#include <memory>
#include <sstream>
#include <utility>
#include <variant>
#include <vector>

namespace tb::mdl
{

class QuakeFileSerializer : public MapFileSerializer
{
public:
  explicit QuakeFileSerializer(std::ostream& stream)
    : MapFileSerializer{stream}
  {
  }

private:
  void doWriteBrushFace(std::ostream& stream, const BrushFace& face) const override
  {
    writeFacePoints(stream, face);
    writeMaterialInfo(stream, face);
    fmt::format_to(std::ostreambuf_iterator<char>{stream}, "\n");
  }

protected:
  void writeFacePoints(std::ostream& stream, const BrushFace& face) const
  {
    const auto& points = face.points();

    fmt::format_to(
      std::ostreambuf_iterator<char>{stream},
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

  static bool shouldQuoteMaterialName(const auto& materialName)
  {
    return materialName.empty()
           || materialName.find_first_of("\"\\ \t") != std::string::npos;
  }

  static std::string quoteMaterialName(const auto& materialName)
  {
    return fmt::format(R"("{}")", kdl::str_escape(materialName, R"(")"));
  }

  void writeMaterialInfo(std::ostream& stream, const BrushFace& face) const
  {
    const auto& materialName = face.attributes().materialName().empty()
                                 ? BrushFaceAttributes::NoMaterialName
                                 : face.attributes().materialName();

    fmt::format_to(
      std::ostreambuf_iterator<char>{stream},
      " {} {} {} {} {} {}",
      shouldQuoteMaterialName(materialName) ? quoteMaterialName(materialName)
                                            : materialName,
      face.attributes().xOffset(),
      face.attributes().yOffset(),
      face.attributes().rotation(),
      face.attributes().xScale(),
      face.attributes().yScale());
  }

  void writeValveMaterialInfo(std::ostream& stream, const BrushFace& face) const
  {
    const auto& materialName = face.attributes().materialName().empty()
                                 ? BrushFaceAttributes::NoMaterialName
                                 : face.attributes().materialName();
    const auto uAxis = face.uAxis();
    const auto vAxis = face.vAxis();

    fmt::format_to(
      std::ostreambuf_iterator<char>{stream},
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
    : QuakeFileSerializer{stream}
  {
  }

private:
  void doWriteBrushFace(std::ostream& stream, const BrushFace& face) const override
  {
    writeFacePoints(stream, face);
    writeMaterialInfo(stream, face);

    if (face.attributes().hasSurfaceAttributes())
    {
      writeSurfaceAttributes(stream, face);
    }

    fmt::format_to(std::ostreambuf_iterator<char>{stream}, "\n");
  }

protected:
  void writeSurfaceAttributes(std::ostream& stream, const BrushFace& face) const
  {
    fmt::format_to(
      std::ostreambuf_iterator<char>{stream},
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
    : Quake2FileSerializer{stream}
  {
  }

private:
  void doWriteBrushFace(std::ostream& stream, const BrushFace& face) const override
  {
    writeFacePoints(stream, face);
    writeValveMaterialInfo(stream, face);

    if (face.attributes().hasSurfaceAttributes())
    {
      writeSurfaceAttributes(stream, face);
    }

    fmt::format_to(std::ostreambuf_iterator<char>{stream}, "\n");
  }
};

class DaikatanaFileSerializer : public Quake2FileSerializer
{
private:
  std::string SurfaceColorFormat;

public:
  explicit DaikatanaFileSerializer(std::ostream& stream)
    : Quake2FileSerializer{stream}
    , SurfaceColorFormat(" %d %d %d")
  {
  }

private:
  void doWriteBrushFace(std::ostream& stream, const BrushFace& face) const override
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

    fmt::format_to(std::ostreambuf_iterator<char>{stream}, "\n");
  }

protected:
  void writeSurfaceColor(std::ostream& stream, const BrushFace& face) const
  {
    if (const auto color = face.resolvedColor())
    {
      stream << " " << color->to<RgbB>().toString();
    }
  }
};

class Hexen2FileSerializer : public QuakeFileSerializer
{
public:
  explicit Hexen2FileSerializer(std::ostream& stream)
    : QuakeFileSerializer{stream}
  {
  }

private:
  void doWriteBrushFace(std::ostream& stream, const BrushFace& face) const override
  {
    writeFacePoints(stream, face);
    writeMaterialInfo(stream, face);
    fmt::format_to(
      std::ostreambuf_iterator<char>{stream}, " 0\n"); // extra value written here
  }
};

class ValveFileSerializer : public QuakeFileSerializer
{
public:
  explicit ValveFileSerializer(std::ostream& stream)
    : QuakeFileSerializer{stream}
  {
  }

private:
  void doWriteBrushFace(std::ostream& stream, const BrushFace& face) const override
  {
    writeFacePoints(stream, face);
    writeValveMaterialInfo(stream, face);
    fmt::format_to(std::ostreambuf_iterator<char>{stream}, "\n");
  }
};

std::unique_ptr<NodeSerializer> MapFileSerializer::create(
  const MapFormat format, std::ostream& stream)
{
  switch (format)
  {
  case MapFormat::Standard:
    return std::make_unique<QuakeFileSerializer>(stream);
  case MapFormat::Quake2:
    // TODO 2427: Implement Quake3 serializers and use them
  case MapFormat::Quake3:
  case MapFormat::Quake3_Legacy:
    return std::make_unique<Quake2FileSerializer>(stream);
  case MapFormat::Quake2_Valve:
  case MapFormat::Quake3_Valve:
    return std::make_unique<Quake2ValveFileSerializer>(stream);
  case MapFormat::Daikatana:
    return std::make_unique<DaikatanaFileSerializer>(stream);
  case MapFormat::Valve:
    return std::make_unique<ValveFileSerializer>(stream);
  case MapFormat::Hexen2:
    return std::make_unique<Hexen2FileSerializer>(stream);
  case MapFormat::Unknown:
    contract_assert(false);
    switchDefault();
  }
}

MapFileSerializer::MapFileSerializer(std::ostream& stream)
  : m_line{1}
  , m_stream{stream}
{
}

void MapFileSerializer::doBeginFile(
  const std::vector<const Node*>& rootNodes, kdl::task_manager& taskManager)
{
  contract_pre(m_nodeToPrecomputedString.empty());

  // collect nodes
  auto nodesToSerialize =
    std::vector<std::variant<const BrushNode*, const PatchNode*>>{};
  nodesToSerialize.reserve(rootNodes.size());

  Node::visitAll(
    rootNodes,
    kdl::overload(
      [](auto&& thisLambda, const WorldNode* world) {
        world->visitChildren(thisLambda);
      },
      [](auto&& thisLambda, const LayerNode* layer) {
        layer->visitChildren(thisLambda);
      },
      [](auto&& thisLambda, const GroupNode* group) {
        group->visitChildren(thisLambda);
      },
      [](auto&& thisLambda, const EntityNode* entity) {
        entity->visitChildren(thisLambda);
      },
      [&](const BrushNode* brush) { nodesToSerialize.emplace_back(brush); },
      [&](const PatchNode* patchNode) {
        nodesToSerialize.emplace_back(patchNode);
      }));

  // serialize brushes to strings in parallel
  using Entry = std::pair<const Node*, PrecomputedString>;
  auto tasks = nodesToSerialize | std::views::transform([&](const auto& node) {
                 return std::function{[&]() {
                   return std::visit(
                     kdl::overload(
                       [&](const BrushNode* brushNode) {
                         return Entry{brushNode, writeBrushFaces(brushNode->brush())};
                       },
                       [&](const PatchNode* patchNode) {
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

void MapFileSerializer::doBeginEntity(const Node* /* node */)
{
  fmt::format_to(std::ostreambuf_iterator<char>{m_stream}, "// entity {}\n", entityNo());
  ++m_line;
  m_startLineStack.push_back(m_line);
  fmt::format_to(std::ostreambuf_iterator<char>{m_stream}, "{{\n");
  ++m_line;
}

void MapFileSerializer::doEndEntity(const Node* node)
{
  fmt::format_to(std::ostreambuf_iterator<char>{m_stream}, "}}\n");
  ++m_line;
  setFilePosition(node);
}

void MapFileSerializer::doEntityProperty(const EntityProperty& attribute)
{
  fmt::format_to(
    std::ostreambuf_iterator<char>{m_stream},
    "\"{}\" \"{}\"\n",
    escapeEntityProperties(attribute.key()),
    escapeEntityProperties(attribute.value()));
  ++m_line;
}

void MapFileSerializer::doBrush(const BrushNode* brush)
{
  fmt::format_to(std::ostreambuf_iterator<char>{m_stream}, "// brush {}\n", brushNo());
  ++m_line;
  m_startLineStack.push_back(m_line);
  fmt::format_to(std::ostreambuf_iterator<char>{m_stream}, "{{\n");
  ++m_line;

  // write pre-serialized brush faces
  auto it = m_nodeToPrecomputedString.find(brush);
  contract_assert(it != std::end(m_nodeToPrecomputedString));

  const auto& precomputedString = it->second;
  m_stream << precomputedString.string;
  m_line += precomputedString.lineCount;

  fmt::format_to(std::ostreambuf_iterator<char>{m_stream}, "}}\n");
  ++m_line;
  setFilePosition(brush);
}

void MapFileSerializer::doBrushFace(const BrushFace& face)
{
  const size_t lines = 1u;
  doWriteBrushFace(m_stream, face);
  face.setFilePosition(m_line, lines);
  m_line += lines;
}

void MapFileSerializer::doPatch(const PatchNode* patchNode)
{
  fmt::format_to(std::ostreambuf_iterator<char>{m_stream}, "// brush {}\n", brushNo());
  ++m_line;
  m_startLineStack.push_back(m_line);

  // write pre-serialized patch
  auto it = m_nodeToPrecomputedString.find(patchNode);
  contract_assert(it != std::end(m_nodeToPrecomputedString));

  const auto& precomputedString = it->second;
  m_stream << precomputedString.string;
  m_line += precomputedString.lineCount;

  setFilePosition(patchNode);
}

void MapFileSerializer::setFilePosition(const Node* node)
{
  const auto start = startLine();
  node->setFilePosition(start, m_line - start);
}

size_t MapFileSerializer::startLine()
{
  contract_pre(!m_startLineStack.empty());

  const auto result = m_startLineStack.back();
  m_startLineStack.pop_back();
  return result;
}

/**
 * Threadsafe
 */
MapFileSerializer::PrecomputedString MapFileSerializer::writeBrushFaces(
  const Brush& brush) const
{
  auto stream = std::stringstream{};
  for (const auto& face : brush.faces())
  {
    doWriteBrushFace(stream, face);
  }
  return {stream.str(), brush.faces().size()};
}

MapFileSerializer::PrecomputedString MapFileSerializer::writePatch(
  const BezierPatch& patch) const
{
  size_t lineCount = 0u;
  auto stream = std::stringstream{};

  fmt::format_to(std::ostreambuf_iterator<char>{stream}, "{{\n");
  ++lineCount;
  fmt::format_to(std::ostreambuf_iterator<char>{stream}, "patchDef2\n");
  ++lineCount;
  fmt::format_to(std::ostreambuf_iterator<char>{stream}, "{{\n");
  ++lineCount;
  fmt::format_to(std::ostreambuf_iterator<char>{stream}, "{}\n", patch.materialName());
  ++lineCount;
  fmt::format_to(
    std::ostreambuf_iterator<char>{stream},
    "( {} {} 0 0 0 )\n",
    patch.pointRowCount(),
    patch.pointColumnCount());
  ++lineCount;
  fmt::format_to(std::ostreambuf_iterator<char>{stream}, "(\n");
  ++lineCount;

  for (size_t row = 0u; row < patch.pointRowCount(); ++row)
  {
    fmt::format_to(std::ostreambuf_iterator<char>{stream}, "( ");
    for (size_t col = 0u; col < patch.pointColumnCount(); ++col)
    {
      const auto& p = patch.controlPoint(row, col);
      fmt::format_to(
        std::ostreambuf_iterator<char>{stream},
        "( {} {} {} {} {} ) ",
        p[0],
        p[1],
        p[2],
        p[3],
        p[4]);
    }
    fmt::format_to(std::ostreambuf_iterator<char>{stream}, ")\n");
    ++lineCount;
  }

  fmt::format_to(std::ostreambuf_iterator<char>{stream}, ")\n");
  ++lineCount;
  fmt::format_to(std::ostreambuf_iterator<char>{stream}, "}}\n");
  ++lineCount;
  fmt::format_to(std::ostreambuf_iterator<char>{stream}, "}}\n");
  ++lineCount;

  return {stream.str(), lineCount};
}

} // namespace tb::mdl
