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

#include "Logger.h"
#include "Map.h"
#include "io/SimpleParserStatus.h"
#include "mdl/AssetUtils.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityModelManager.h"
#include "mdl/EntityNode.h"
#include "mdl/Game.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/MaterialManager.h"
#include "mdl/PatchNode.h"
#include "mdl/PushSelection.h"
#include "mdl/ResourceManager.h"
#include "mdl/Transaction.h"
#include "mdl/WorldNode.h" // IWYU pragma: keep

#include "kdl/range_to_vector.h"
#include "kdl/string_utils.h"

#include <fmt/format.h>
#include <fmt/std.h>

#include <vector>

namespace tb::mdl
{
namespace
{

auto makeSetMaterialsVisitor(MaterialManager& manager)
{
  return kdl::overload(
    [](auto&& thisLambda, WorldNode* worldNode) { worldNode->visitChildren(thisLambda); },
    [](auto&& thisLambda, LayerNode* layerNode) { layerNode->visitChildren(thisLambda); },
    [](auto&& thisLambda, GroupNode* groupNode) { groupNode->visitChildren(thisLambda); },
    [](auto&& thisLambda, EntityNode* entityNode) {
      entityNode->visitChildren(thisLambda);
    },
    [&](BrushNode* brushNode) {
      const auto& brush = brushNode->brush();
      for (size_t i = 0u; i < brush.faceCount(); ++i)
      {
        const auto& face = brush.face(i);
        auto* material = manager.material(face.attributes().materialName());
        brushNode->setFaceMaterial(i, material);
      }
    },
    [&](PatchNode* patchNode) {
      auto* material = manager.material(patchNode->patch().materialName());
      patchNode->setMaterial(material);
    });
}

auto makeUnsetMaterialsVisitor()
{
  return kdl::overload(
    [](auto&& thisLambda, WorldNode* worldNode) { worldNode->visitChildren(thisLambda); },
    [](auto&& thisLambda, LayerNode* layerNode) { layerNode->visitChildren(thisLambda); },
    [](auto&& thisLambda, GroupNode* groupNode) { groupNode->visitChildren(thisLambda); },
    [](auto&& thisLambda, EntityNode* entityNode) {
      entityNode->visitChildren(thisLambda);
    },
    [](BrushNode* brushNode) {
      const auto& brush = brushNode->brush();
      for (size_t i = 0u; i < brush.faceCount(); ++i)
      {
        brushNode->setFaceMaterial(i, nullptr);
      }
    },
    [](PatchNode* patchNode) { patchNode->setMaterial(nullptr); });
}

auto makeSetEntityDefinitionsVisitor(EntityDefinitionManager& manager)
{
  // this helper lambda must be captured by value
  const auto setEntityDefinition = [&](auto* node) {
    const auto* definition = manager.definition(node);
    node->setDefinition(definition);
  };

  return kdl::overload(
    [=](auto&& thisLambda, WorldNode* worldNode) {
      setEntityDefinition(worldNode);
      worldNode->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, LayerNode* layerNode) { layerNode->visitChildren(thisLambda); },
    [](auto&& thisLambda, GroupNode* groupNode) { groupNode->visitChildren(thisLambda); },
    [=](EntityNode* entityNode) { setEntityDefinition(entityNode); },
    [](BrushNode*) {},
    [](PatchNode*) {});
}

auto makeUnsetEntityDefinitionsVisitor()
{
  return kdl::overload(
    [](auto&& thisLambda, WorldNode* worldNode) {
      worldNode->setDefinition(nullptr);
      worldNode->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, LayerNode* layerNode) { layerNode->visitChildren(thisLambda); },
    [](auto&& thisLambda, GroupNode* groupNode) { groupNode->visitChildren(thisLambda); },
    [](EntityNode* entityNode) { entityNode->setDefinition(nullptr); },
    [](BrushNode*) {},
    [](PatchNode*) {});
}

auto makeSetEntityModelsVisitor(EntityModelManager& manager, Logger& logger)
{
  return kdl::overload(
    [](auto&& thisLambda, WorldNode* world) { world->visitChildren(thisLambda); },
    [](auto&& thisLambda, LayerNode* layer) { layer->visitChildren(thisLambda); },
    [](auto&& thisLambda, GroupNode* group) { group->visitChildren(thisLambda); },
    [&](EntityNode* entityNode) {
      const auto modelSpec =
        safeGetModelSpecification(logger, entityNode->entity().classname(), [&]() {
          return entityNode->entity().modelSpecification();
        });
      const auto* model = manager.model(modelSpec.path);
      entityNode->setModel(model);
    },
    [](BrushNode*) {},
    [](PatchNode*) {});
}

auto makeUnsetEntityModelsVisitor()
{
  return kdl::overload(
    [](auto&& thisLambda, WorldNode* world) { world->visitChildren(thisLambda); },
    [](auto&& thisLambda, LayerNode* layer) { layer->visitChildren(thisLambda); },
    [](auto&& thisLambda, GroupNode* group) { group->visitChildren(thisLambda); },
    [](EntityNode* entity) { entity->setModel(nullptr); },
    [](BrushNode*) {},
    [](PatchNode*) {});
}

} // namespace

EntityDefinitionFileSpec Map::entityDefinitionFile() const
{
  auto* worldNode = world();
  return worldNode ? game()->extractEntityDefinitionFile(worldNode->entity())
                   : EntityDefinitionFileSpec{};
}

std::vector<EntityDefinitionFileSpec> Map::allEntityDefinitionFiles() const
{
  return game()->allEntityDefinitionFiles();
}

void Map::setEntityDefinitionFile(const EntityDefinitionFileSpec& spec)
{
  // to avoid backslashes being misinterpreted as escape sequences
  const auto formatted = kdl::str_replace_every(spec.asString(), "\\", "/");

  auto entity = world()->entity();
  entity.addOrUpdateProperty(EntityPropertyKeys::EntityDefinitions, formatted);
  updateNodeContents(
    "Set Entity Definitions", {{world(), NodeContents{std::move(entity)}}}, {});
}

void Map::setEntityDefinitions(std::vector<EntityDefinition> definitions)
{
  entityDefinitionManager().setDefinitions(std::move(definitions));
}

std::vector<std::filesystem::path> Map::enabledMaterialCollections() const
{
  if (auto* worldNode = world())
  {
    if (
      const auto* materialCollectionStr =
        worldNode->entity().property(EntityPropertyKeys::EnabledMaterialCollections))
    {
      const auto strs = kdl::str_split(*materialCollectionStr, ";");
      return kdl::vec_sort_and_remove_duplicates(
        strs | std::views::transform([](const auto& str) {
          return std::filesystem::path{str};
        })
        | kdl::to_vector);
    }

    // Otherwise, enable all texture collections
    return kdl::vec_sort_and_remove_duplicates(
      m_materialManager->collections()
      | std::views::transform([](const auto& collection) { return collection.path(); })
      | kdl::to_vector);
  }
  return {};
}

std::vector<std::filesystem::path> Map::disabledMaterialCollections() const
{
  if (world())
  {
    auto materialCollections = kdl::vec_sort_and_remove_duplicates(kdl::vec_transform(
      m_materialManager->collections(),
      [](const auto& collection) { return collection.path(); }));

    return kdl::set_difference(materialCollections, enabledMaterialCollections());
  }
  return {};
}

void Map::setEnabledMaterialCollections(
  const std::vector<std::filesystem::path>& enabledMaterialCollections)
{
  const auto enabledMaterialCollectionStr = kdl::str_join(
    kdl::vec_transform(
      kdl::vec_sort_and_remove_duplicates(enabledMaterialCollections),
      [](const auto& path) { return path.string(); }),
    ";");

  auto transaction = Transaction{*this, "Set enabled material collections"};

  const auto pushSelection = PushSelection{*this};
  deselectAll();

  const auto success = setEntityProperty(
    EntityPropertyKeys::EnabledMaterialCollections, enabledMaterialCollectionStr);
  transaction.finish(success);
}

void Map::reloadMaterialCollections()
{
  const auto nodes = std::vector<Node*>{world()};
  const auto notifyNodes =
    NotifyBeforeAndAfter{nodesWillChangeNotifier, nodesDidChangeNotifier, nodes};
  const auto notifyMaterialCollections = NotifyBeforeAndAfter{
    materialCollectionsWillChangeNotifier, materialCollectionsDidChangeNotifier};

  logger().info() << "Reloading material collections";
  clearMaterials();
  // materialCollectionsDidChange will load the collections again
}

void Map::reloadEntityDefinitions()
{
  const auto nodes = std::vector<Node*>{m_world.get()};
  const auto notifyNodes =
    NotifyBeforeAndAfter{nodesWillChangeNotifier, nodesDidChangeNotifier, nodes};
  const auto notifyEntityDefinitions = NotifyBeforeAndAfter{
    entityDefinitionsWillChangeNotifier, entityDefinitionsDidChangeNotifier};

  logger().info() << "Reloading entity definitions";
}

void Map::loadAssets()
{
  loadEntityDefinitions();
  setEntityDefinitions();
  setEntityModels();
  loadMaterials();
  setMaterials();
}

void Map::clearAssets()
{
  clearEntityDefinitions();
  clearEntityModels();
  clearMaterials();
}

void Map::loadEntityDefinitions()
{
  const auto spec = m_world ? game()->extractEntityDefinitionFile(m_world->entity())
                            : EntityDefinitionFileSpec{};
  const auto path = game()->findEntityDefinitionFile(spec, externalSearchPaths());
  auto status = io::SimpleParserStatus{m_logger};

  entityDefinitionManager().loadDefinitions(path, *game(), status)
    | kdl::transform([&]() {
        m_logger.info() << fmt::format(
          "Loaded entity definition file {}", path.filename());
        // TODO:
        // createEntityDefinitionActions();
      })
    | kdl::transform_error([&](auto e) {
        if (spec.builtin())
        {
          m_logger.error() << "Could not load builtin entity definition file '"
                           << spec.path() << "': " << e.msg;
        }
        else
        {
          m_logger.error() << "Could not load external entity definition file '"
                           << spec.path() << "': " << e.msg;
        }
      });
}

void Map::clearEntityDefinitions()
{
  unsetEntityDefinitions();
  m_entityDefinitionManager->clear();
}

void Map::reloadMaterials()
{
  clearMaterials();
  loadMaterials();
}

void Map::loadMaterials()
{
  if (const auto* wadStr = m_world->entity().property(EntityPropertyKeys::Wad))
  {
    const auto wadPaths = kdl::vec_transform(
      kdl::str_split(*wadStr, ";"),
      [](const auto& str) { return std::filesystem::path{str}; });
    m_game->reloadWads(path(), wadPaths, m_logger);
  }
  m_materialManager->reload(
    m_game->gameFileSystem(),
    m_game->config().materialConfig,
    [&](auto resourceLoader) {
      auto resource = std::make_shared<TextureResource>(std::move(resourceLoader));
      m_resourceManager->addResource(resource);
      return resource;
    },
    m_taskManager);
}

void Map::clearMaterials()
{
  unsetMaterials();
  materialManager().clear();
}

void Map::setMaterials()
{
  m_world->accept(makeSetMaterialsVisitor(*m_materialManager));
  materialUsageCountsDidChangeNotifier();
}

void Map::setMaterials(const std::vector<Node*>& nodes)
{
  Node::visitAll(nodes, makeSetMaterialsVisitor(*m_materialManager));
  materialUsageCountsDidChangeNotifier();
}

void Map::setMaterials(const std::vector<BrushFaceHandle>& faceHandles)
{
  for (const auto& faceHandle : faceHandles)
  {
    BrushNode* node = faceHandle.node();
    const BrushFace& face = faceHandle.face();
    auto* material = m_materialManager->material(face.attributes().materialName());
    node->setFaceMaterial(faceHandle.faceIndex(), material);
  }
  materialUsageCountsDidChangeNotifier();
}

void Map::unsetMaterials()
{
  m_world->accept(makeUnsetMaterialsVisitor());
  materialUsageCountsDidChangeNotifier();
}

void Map::unsetMaterials(const std::vector<Node*>& nodes)
{
  Node::visitAll(nodes, makeUnsetMaterialsVisitor());
  materialUsageCountsDidChangeNotifier();
}

void Map::setEntityDefinitions()
{
  m_world->accept(makeSetEntityDefinitionsVisitor(*m_entityDefinitionManager));
}

void Map::setEntityDefinitions(const std::vector<Node*>& nodes)
{
  Node::visitAll(nodes, makeSetEntityDefinitionsVisitor(*m_entityDefinitionManager));
}

void Map::unsetEntityDefinitions()
{
  m_world->accept(makeUnsetEntityDefinitionsVisitor());
}

void Map::unsetEntityDefinitions(const std::vector<Node*>& nodes)
{
  Node::visitAll(nodes, makeUnsetEntityDefinitionsVisitor());
}

void Map::clearEntityModels()
{
  unsetEntityModels();
  m_entityModelManager->clear();
}

void Map::setEntityModels()
{
  m_world->accept(makeSetEntityModelsVisitor(*m_entityModelManager, m_logger));
}

void Map::setEntityModels(const std::vector<Node*>& nodes)
{
  Node::visitAll(nodes, makeSetEntityModelsVisitor(*m_entityModelManager, m_logger));
}

void Map::unsetEntityModels()
{
  m_world->accept(makeUnsetEntityModelsVisitor());
}

void Map::unsetEntityModels(const std::vector<Node*>& nodes)
{
  Node::visitAll(nodes, makeUnsetEntityModelsVisitor());
}

} // namespace tb::mdl
