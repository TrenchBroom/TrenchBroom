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

#include "mdl/MapFileSerializer.h"

#include "Macros.h"
#include "mdl/BezierPatch.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/EntityProperties.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/PatchNode.h"
#include "mdl/Quake3BrushPrimitive.h"
#include "mdl/WorldNode.h"

#include "kd/contracts.h"
#include "kd/overload.h"
#include "kd/string_format.h"
#include "kd/task_manager.h"

#include "vm/plane.h"

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
  void writeBrushFace(std::ostream& stream, const BrushFace& face) const override
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
  void writeBrushFace(std::ostream& stream, const BrushFace& face) const override
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
  void writeBrushFace(std::ostream& stream, const BrushFace& face) const override
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

class Quake3FileSerializer : public Quake2FileSerializer
{
public:
  explicit Quake3FileSerializer(std::ostream& stream)
    : Quake2FileSerializer{stream}
  {
  }

private:
  // Quake 3 brushes are written as brush primitives: the faces are wrapped in a
  // `brushDef { ... }` block and each face stores a texture projection matrix instead of
  // the legacy offset/rotation/scale values.
  PrecomputedString writeBrush(const Brush& brush) const override
  {
    auto stream = std::stringstream{};
    fmt::format_to(std::ostreambuf_iterator<char>{stream}, "brushDef\n{{\n");
    for (const auto& face : brush.faces())
    {
      writeBrushFace(stream, face);
    }
    fmt::format_to(std::ostreambuf_iterator<char>{stream}, "}}\n");
    // brushDef + opening brace + faces + closing brace
    return {stream.str(), brush.faces().size() + 3};
  }

  void writeBrushFace(std::ostream& stream, const BrushFace& face) const override
  {
    writeFacePoints(stream, face);
    writePrimitiveMaterialInfo(stream, face);
    writeSurfaceAttributes(stream, face);

    fmt::format_to(std::ostreambuf_iterator<char>{stream}, "\n");
  }

  void writePrimitiveMaterialInfo(std::ostream& stream, const BrushFace& face) const
  {
    const auto& materialName = face.attributes().materialName().empty()
                                 ? BrushFaceAttributes::NoMaterialName
                                 : face.attributes().materialName();

    // Reconstruct the brush primitive texture matrix from the face's parallel UV axes.
    // The stored axes are divided by their scale to obtain the effective projection, and
    // the texture size that the reader folds into the axes is removed again.
    const auto xScale = face.attributes().xScale();
    const auto yScale = face.attributes().yScale();
    const auto uAxis = xScale != 0.0f ? face.uAxis() / double(xScale) : face.uAxis();
    const auto vAxis = yScale != 0.0f ? face.vAxis() / double(yScale) : face.vAxis();

    const auto matrix = uvAxesToBrushPrimitiveMatrix(
      face.boundary().normal,
      uAxis,
      vAxis,
      face.attributes().offset(),
      face.textureSize());

    fmt::format_to(
      std::ostreambuf_iterator<char>{stream},
      " ( ( {} {} {} ) ( {} {} {} ) ) {}",
      matrix.row0.x(),
      matrix.row0.y(),
      matrix.row0.z(),
      matrix.row1.x(),
      matrix.row1.y(),
      matrix.row1.z(),
      shouldQuoteMaterialName(materialName) ? quoteMaterialName(materialName)
                                            : materialName);
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
  void writeBrushFace(std::ostream& stream, const BrushFace& face) const override
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
  void writeBrushFace(std::ostream& stream, const BrushFace& face) const override
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
  void writeBrushFace(std::ostream& stream, const BrushFace& face) const override
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
  case MapFormat::Quake3_Legacy:
  case MapFormat::Quake3:
    return std::make_unique<Quake2FileSerializer>(stream);
  case MapFormat::Quake3_BrushPrimitives:
    return std::make_unique<Quake3FileSerializer>(stream);
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
  auto nodesToSerialize = std::vector<std::variant<const BrushNode*, const PatchNode*>>{};
  nodesToSerialize.reserve(rootNodes.size());

  Node::visitAll(
    rootNodes,
    kdl::overload(
      [](auto&& thisLambda, const WorldNode& worldNode) {
        worldNode.visitChildren(thisLambda);
      },
      [](auto&& thisLambda, const LayerNode& layerNode) {
        layerNode.visitChildren(thisLambda);
      },
      [](auto&& thisLambda, const GroupNode& groupNode) {
        groupNode.visitChildren(thisLambda);
      },
      [](auto&& thisLambda, const EntityNode& entityNode) {
        entityNode.visitChildren(thisLambda);
      },
      [&](const BrushNode& brushNode) { nodesToSerialize.emplace_back(&brushNode); },
      [&](const PatchNode& patchNode) { nodesToSerialize.emplace_back(&patchNode); }));

  // serialize brushes to strings in parallel
  using Entry = std::pair<const Node*, PrecomputedString>;
  auto tasks = nodesToSerialize | std::views::transform([&](const auto& node) {
                 return std::function{[&]() {
                   return std::visit(
                     kdl::overload(
                       [&](const BrushNode* brushNode) {
                         return Entry{brushNode, writeBrush(brushNode->brush())};
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

void MapFileSerializer::doBeginEntity(const Node& /* node */)
{
  fmt::format_to(std::ostreambuf_iterator<char>{m_stream}, "// entity {}\n", entityNo());
  ++m_line;
  m_startLineStack.push_back(m_line);
  fmt::format_to(std::ostreambuf_iterator<char>{m_stream}, "{{\n");
  ++m_line;
}

void MapFileSerializer::doEndEntity(const Node& node)
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

void MapFileSerializer::doBrush(const BrushNode& brushNode)
{
  fmt::format_to(std::ostreambuf_iterator<char>{m_stream}, "// brush {}\n", brushNo());
  ++m_line;
  m_startLineStack.push_back(m_line);
  fmt::format_to(std::ostreambuf_iterator<char>{m_stream}, "{{\n");
  ++m_line;

  // write pre-serialized brush faces
  auto it = m_nodeToPrecomputedString.find(&brushNode);
  contract_assert(it != std::end(m_nodeToPrecomputedString));

  const auto& precomputedString = it->second;
  m_stream << precomputedString.string;
  m_line += precomputedString.lineCount;

  fmt::format_to(std::ostreambuf_iterator<char>{m_stream}, "}}\n");
  ++m_line;
  setFilePosition(brushNode);
}

void MapFileSerializer::doBrushFace(const BrushFace& face)
{
  const size_t lines = 1u;
  writeBrushFace(m_stream, face);
  face.setFilePosition(m_line, lines);
  m_line += lines;
}

void MapFileSerializer::doPatch(const PatchNode& patchNode)
{
  fmt::format_to(std::ostreambuf_iterator<char>{m_stream}, "// brush {}\n", brushNo());
  ++m_line;
  m_startLineStack.push_back(m_line);

  // write pre-serialized patch
  auto it = m_nodeToPrecomputedString.find(&patchNode);
  contract_assert(it != std::end(m_nodeToPrecomputedString));

  const auto& precomputedString = it->second;
  m_stream << precomputedString.string;
  m_line += precomputedString.lineCount;

  setFilePosition(patchNode);
}

void MapFileSerializer::setFilePosition(const Node& node)
{
  const auto start = startLine();
  node.setFilePosition(start, m_line - start);
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
MapFileSerializer::PrecomputedString MapFileSerializer::writeBrush(
  const Brush& brush) const
{
  auto stream = std::stringstream{};
  for (const auto& face : brush.faces())
  {
    writeBrushFace(stream, face);
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
