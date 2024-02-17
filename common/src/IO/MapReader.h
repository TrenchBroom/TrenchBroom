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
#include "IO/StandardMapParser.h"
#include "Model/BezierPatch.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/EntityProperties.h"
#include "Model/IdType.h"

#include "kdl/result.h"

#include <vecmath/bbox.h>
#include <vecmath/forward.h>

#include <optional>
#include <string_view>
#include <variant>
#include <vector>

namespace TrenchBroom::Model
{
class BrushNode;
class EntityNode;
class EntityNodeBase;
class EntityProperty;
class GroupNode;
class LayerNode;
enum class MapFormat;
class Node;
class WorldNode;
} // namespace TrenchBroom::Model

namespace TrenchBroom::IO
{

class ParserStatus;

/**
 * Abstract superclass containing common code for:
 *
 *  - WorldReader (loading a whole .map)
 *  - NodeReader (reading part of a map, for pasting into an existing map)
 *  - BrushFaceReader (reading faces when copy/pasting texture alignment)
 *
 * The flow of control is:
 *
 * 1. MapParser callbacks get called with the raw data, which we just store
 * (m_objectInfos).
 * 2. Convert the raw data to nodes in parallel (createNodes) and record any additional
 * information necessary to restore the parent / child relationships.
 * 3. Validate the created nodes.
 * 4. Post process the nodes to find the correct parent nodes (createNodes).
 * 5. Call the appropriate callbacks (onWorldspawn, onLayer, ...).
 */
class MapReader : public StandardMapParser
{
public: // only public so that helper methods can see these declarations
  struct EntityInfo
  {
    std::vector<Model::EntityProperty> properties;
    size_t startLine;
    size_t lineCount;
  };

  struct BrushInfo
  {
    std::vector<Model::BrushFace> faces;
    size_t startLine;
    size_t lineCount;
    std::optional<size_t> parentIndex;
  };

  struct PatchInfo
  {
    size_t rowCount;
    size_t columnCount;
    std::vector<Model::BezierPatch::Point> controlPoints;
    std::string textureName;
    size_t startLine;
    size_t lineCount;
    std::optional<size_t> parentIndex;
  };

  using ObjectInfo = std::variant<EntityInfo, BrushInfo, PatchInfo>;

private:
  Model::EntityPropertyConfig m_entityPropertyConfig;
  vm::bbox3 m_worldBounds;

private: // data populated in response to MapParser callbacks
  std::vector<ObjectInfo> m_objectInfos;
  std::optional<size_t> m_currentEntityInfo;

protected:
  /**
   * Creates a new reader where the given string is expected to be formatted in the given
   * source map format, and the created objects are converted to the given target format.
   *
   * @param str the string to parse
   * @param sourceMapFormat the expected format of the given string
   * @param targetMapFormat the format to convert the created objects to
   * @param entityPropertyConfig the entity property config to use
   * if orphaned
   */
  MapReader(
    std::string_view str,
    Model::MapFormat sourceMapFormat,
    Model::MapFormat targetMapFormat,
    Model::EntityPropertyConfig entityPropertyConfig);

  /**
   * Attempts to parse as one or more entities.
   *
   * @throws ParserException if parsing fails
   */
  void readEntities(const vm::bbox3& worldBounds, ParserStatus& status);
  /**
   * Attempts to parse as one or more brushes without any enclosing entity.
   *
   * @throws ParserException if parsing fails
   */
  void readBrushes(const vm::bbox3& worldBounds, ParserStatus& status);
  /**
   * Attempts to parse as one or more brush faces.
   *
   * @throws ParserException if parsing fails
   */
  void readBrushFaces(const vm::bbox3& worldBounds, ParserStatus& status);

protected: // implement MapParser interface
  void onBeginEntity(
    size_t line,
    std::vector<Model::EntityProperty> properties,
    ParserStatus& status) override;
  void onEndEntity(size_t startLine, size_t lineCount, ParserStatus& status) override;
  void onBeginBrush(size_t line, ParserStatus& status) override;
  void onEndBrush(size_t startLine, size_t lineCount, ParserStatus& status) override;
  void onStandardBrushFace(
    size_t line,
    Model::MapFormat targetMapFormat,
    const vm::vec3& point1,
    const vm::vec3& point2,
    const vm::vec3& point3,
    const Model::BrushFaceAttributes& attribs,
    ParserStatus& status) override;
  void onValveBrushFace(
    size_t line,
    Model::MapFormat targetMapFormat,
    const vm::vec3& point1,
    const vm::vec3& point2,
    const vm::vec3& point3,
    const Model::BrushFaceAttributes& attribs,
    const vm::vec3& texAxisX,
    const vm::vec3& texAxisY,
    ParserStatus& status) override;
  void onPatch(
    size_t startLine,
    size_t lineCount,
    Model::MapFormat targetMapFormat,
    size_t rowCount,
    size_t columnCount,
    std::vector<vm::vec<FloatType, 5>> controlPoints,
    std::string textureName,
    ParserStatus& status) override;

private: // helper methods
  void createNodes(ParserStatus& status);

private: // subclassing interface - these will be called in the order that nodes should be
         // inserted
  /**
   * Called for the first worldspawn entity. Subclasses cannot capture the given world
   * node but must create their own instead.
   *
   * If a world node was created, then this function is guaranteed to be called before any
   * other callback.
   *
   * Returns a pointer to a node which should become the parent of any node that belongs
   * to the world. This could be the default layer of the world node, or a dummy entity.
   */
  virtual Model::Node* onWorldNode(
    std::unique_ptr<Model::WorldNode> worldNode, ParserStatus& status) = 0;

  /**
   * Called for each custom layer.
   */
  virtual void onLayerNode(
    std::unique_ptr<Model::Node> layerNode, ParserStatus& status) = 0;

  /**
   * Called for each group, entity entity or brush node. The given parent can be null.
   */
  virtual void onNode(
    Model::Node* parentNode, std::unique_ptr<Model::Node> node, ParserStatus& status) = 0;

  /**
   * Called for each brush face.
   */
  virtual void onBrushFace(Model::BrushFace face, ParserStatus& status);
};

} // namespace TrenchBroom::IO
