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

#include "mdl/Map.h"

#include "io/GameConfigParser.h"
#include "io/LoadMaterialCollections.h"
#include "io/NodeReader.h"
#include "io/NodeWriter.h"
#include "io/WorldReader.h"
#include "mdl/ChangeBrushFaceAttributesRequest.h"
#include "mdl/CommandProcessor.h"
#include "mdl/EditorContext.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityModelManager.h"
#include "mdl/Game.h"
#include "mdl/GameFactory.h"
#include "mdl/Grid.h"
#include "mdl/LinkedGroupUtils.h"
#include "mdl/MapTextEncoding.h"
#include "mdl/MaterialManager.h"
#include "mdl/MissingModValidator.h"
#include "mdl/ModelUtils.h"
#include "mdl/Node.h"
#include "mdl/PushSelection.h"
#include "mdl/RepeatStack.h"
#include "mdl/ResourceManager.h"
#include "mdl/SoftMapBoundsValidator.h"
#include "mdl/TagManager.h"
#include "mdl/Transaction.h"
#include "mdl/UpdateLinkedGroupsHelper.h"
#include "mdl/VertexHandleManager.h"
#include "mdl/WorldNode.h"

#include "kdl/task_manager.h"

#include <fmt/format.h>
#include <fmt/std.h>

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <string>

namespace tb::mdl
{

const vm::bbox3d Map::DefaultWorldBounds(-32768.0, 32768.0);
const std::string Map::DefaultDocumentName("unnamed.map");

Map::Map(kdl::task_manager& taskManager, Logger& logger)
  : m_logger{logger}
  , m_taskManager{taskManager}
  , m_resourceManager{std::make_unique<ResourceManager>()}
  , m_entityDefinitionManager{std::make_unique<EntityDefinitionManager>()}
  , m_entityModelManager{std::make_unique<EntityModelManager>(
      [&](auto resourceLoader) {
        auto resource =
          std::make_shared<EntityModelDataResource>(std::move(resourceLoader));
        m_resourceManager->addResource(resource);
        return resource;
      },
      m_logger)}
  , m_materialManager{std::make_unique<MaterialManager>(m_logger)}
  , m_tagManager{std::make_unique<TagManager>()}
  , m_editorContext{std::make_unique<EditorContext>()}
  , m_grid{std::make_unique<Grid>(4)}
  , m_worldBounds{DefaultWorldBounds}
  , m_vertexHandles{std::make_unique<VertexHandleManager>()}
  , m_edgeHandles{std::make_unique<EdgeHandleManager>()}
  , m_faceHandles{std::make_unique<FaceHandleManager>()}
  , m_currentMaterialName{BrushFaceAttributes::NoMaterialName}
  , m_repeatStack{std::make_unique<RepeatStack>()}
  , m_commandProcessor{std::make_unique<CommandProcessor>(*this)}
{
  connectObservers();
}

Map::~Map()
{
  clearWorld();
}

Logger& Map::logger()
{
  return m_logger;
}

kdl::task_manager& Map::taskManager()
{
  return m_taskManager;
}

EntityDefinitionManager& Map::entityDefinitionManager()
{
  return *m_entityDefinitionManager;
}

const EntityDefinitionManager& Map::entityDefinitionManager() const
{
  return *m_entityDefinitionManager;
}

EntityModelManager& Map::entityModelManager()
{
  return *m_entityModelManager;
}

const EntityModelManager& Map::entityModelManager() const
{
  return *m_entityModelManager;
}

MaterialManager& Map::materialManager()
{
  return *m_materialManager;
}

const MaterialManager& Map::materialManager() const
{
  return *m_materialManager;
}

TagManager& Map::tagManager()
{
  return *m_tagManager;
}

const TagManager& Map::tagManager() const
{
  return *m_tagManager;
}

EditorContext& Map::editorContext()
{
  return *m_editorContext;
}

const EditorContext& Map::editorContext() const
{
  return *m_editorContext;
}

Grid& Map::grid()
{
  return *m_grid;
}

const Grid& Map::grid() const
{
  return *m_grid;
}

const Game* Map::game() const
{
  return m_game.get();
}

const vm::bbox3d& Map::worldBounds() const
{
  return m_worldBounds;
}

WorldNode* Map::world() const
{
  return m_world.get();
}

MapTextEncoding Map::encoding() const
{
  return MapTextEncoding::Quake;
}

VertexHandleManager& Map::vertexHandles()
{
  return *m_vertexHandles;
}

const VertexHandleManager& Map::vertexHandles() const
{
  return *m_vertexHandles;
}

EdgeHandleManager& Map::edgeHandles()
{
  return *m_edgeHandles;
}

const EdgeHandleManager& Map::edgeHandles() const
{
  return *m_edgeHandles;
}

FaceHandleManager& Map::faceHandles()
{
  return *m_faceHandles;
}

const FaceHandleManager& Map::faceHandles() const
{
  return *m_faceHandles;
}

const std::string& Map::currentMaterialName() const
{
  return m_currentMaterialName;
}

void Map::setCurrentMaterialName(const std::string& currentMaterialName)
{
  if (m_currentMaterialName != currentMaterialName)
  {
    m_currentMaterialName = currentMaterialName;
    currentMaterialNameDidChangeNotifier(m_currentMaterialName);
  }
}

} // namespace tb::mdl
