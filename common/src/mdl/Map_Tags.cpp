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

#include "Ensure.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/Game.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/MaterialManager.h"
#include "mdl/PatchNode.h"
#include "mdl/TagManager.h"
#include "mdl/WorldNode.h"

namespace tb::mdl
{
namespace
{

auto makeInitializeNodeTagsVisitor(TagManager& tagManager)
{
  return kdl::overload(
    [&](auto&& thisLambda, WorldNode* world) {
      world->initializeTags(tagManager);
      world->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, LayerNode* layer) {
      layer->initializeTags(tagManager);
      layer->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, GroupNode* group) {
      group->initializeTags(tagManager);
      group->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, EntityNode* entity) {
      entity->initializeTags(tagManager);
      entity->visitChildren(thisLambda);
    },
    [&](BrushNode* brush) { brush->initializeTags(tagManager); },
    [&](PatchNode* patch) { patch->initializeTags(tagManager); });
}

auto makeClearNodeTagsVisitor()
{
  return kdl::overload(
    [](auto&& thisLambda, WorldNode* world) {
      world->clearTags();
      world->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, LayerNode* layer) {
      layer->clearTags();
      layer->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, GroupNode* group) {
      group->clearTags();
      group->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, EntityNode* entity) {
      entity->clearTags();
      entity->visitChildren(thisLambda);
    },
    [](BrushNode* brush) { brush->clearTags(); },
    [](PatchNode* patch) { patch->clearTags(); });
}

} // namespace

void Map::registerSmartTags()
{
  ensure(game() != nullptr, "game is null");

  m_tagManager->clearSmartTags();
  m_tagManager->registerSmartTags(game()->config().smartTags);
}

const std::vector<SmartTag>& Map::smartTags() const
{
  return m_tagManager->smartTags();
}

bool Map::isRegisteredSmartTag(const std::string& name) const
{
  return m_tagManager->isRegisteredSmartTag(name);
}

const SmartTag& Map::smartTag(const std::string& name) const
{
  return m_tagManager->smartTag(name);
}

bool Map::isRegisteredSmartTag(const size_t index) const
{
  return m_tagManager->isRegisteredSmartTag(index);
}

const SmartTag& Map::smartTag(const size_t index) const
{
  return m_tagManager->smartTag(index);
}

void Map::initializeAllNodeTags()
{
  m_world->accept(makeInitializeNodeTagsVisitor(*m_tagManager));
}

void Map::initializeNodeTags(const std::vector<Node*>& nodes)
{
  Node::visitAll(nodes, makeInitializeNodeTagsVisitor(*m_tagManager));
}

void Map::clearNodeTags(const std::vector<Node*>& nodes)
{
  Node::visitAll(nodes, makeClearNodeTagsVisitor());
}

void Map::updateNodeTags(const std::vector<Node*>& nodes)
{
  for (auto* node : nodes)
  {
    node->updateTags(*m_tagManager);
  }
}

void Map::updateFaceTags(const std::vector<BrushFaceHandle>& faceHandles)
{
  for (const auto& faceHandle : faceHandles)
  {
    BrushNode* node = faceHandle.node();
    node->updateFaceTags(faceHandle.faceIndex(), *m_tagManager);
  }
}

void Map::updateAllFaceTags()
{
  m_world->accept(kdl::overload(
    [](auto&& thisLambda, WorldNode* world) { world->visitChildren(thisLambda); },
    [](auto&& thisLambda, LayerNode* layer) { layer->visitChildren(thisLambda); },
    [](auto&& thisLambda, GroupNode* group) { group->visitChildren(thisLambda); },
    [](auto&& thisLambda, EntityNode* entity) { entity->visitChildren(thisLambda); },
    [&](BrushNode* brush) { brush->initializeTags(*m_tagManager); },
    [](PatchNode*) {}));
}

void Map::updateFaceTagsAfterResourcesWhereProcessed(
  const std::vector<ResourceId>& resourceIds)
{
  // Some textures contain embedded default values for surface flags and such, so we must
  // update the face tags after the resources have been processed.

  const auto materials = m_materialManager->findMaterialsByTextureResourceId(resourceIds);
  const auto materialSet =
    std::unordered_set<const Material*>{materials.begin(), materials.end()};

  m_world->accept(kdl::overload(
    [](auto&& thisLambda, WorldNode* world) { world->visitChildren(thisLambda); },
    [](auto&& thisLambda, LayerNode* layer) { layer->visitChildren(thisLambda); },
    [](auto&& thisLambda, GroupNode* group) { group->visitChildren(thisLambda); },
    [](auto&& thisLambda, EntityNode* entity) { entity->visitChildren(thisLambda); },
    [&](BrushNode* brushNode) {
      const auto& faces = brushNode->brush().faces();
      for (size_t i = 0; i < faces.size(); ++i)
      {
        {
          const auto& face = faces[i];
          if (materialSet.contains(face.material()))
          {
            brushNode->updateFaceTags(i, *m_tagManager);
          }
        }
      }
    },
    [](PatchNode*) {}));
}

} // namespace tb::mdl
