/*
 Copyright (C) 2024 Kristian Duske

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

#include "SetLinkIdsCommand.h"

#include "Ensure.h"
#include "Model/BezierPatch.h"
#include "Model/Brush.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Group.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/Node.h"
#include "Model/PatchNode.h"
#include "View/MapDocumentCommandFacade.h"

#include <kdl/result.h>
#include <kdl/vector_utils.h>

namespace TrenchBroom::View
{

SetLinkIdsCommand::SetLinkIdsCommand(
  const std::string& name, std::vector<std::tuple<Model::Node*, std::string>> linkIds)
  : UndoableCommand{name, true}
  , m_linkIds{std::move(linkIds)}
{
}

SetLinkIdsCommand::~SetLinkIdsCommand() = default;

namespace
{
auto setLinkIds(const std::vector<std::tuple<Model::Node*, std::string>>& linkIds)
{
  return kdl::vec_transform(linkIds, [](const auto& nodeAndLinkId) {
    auto* node = std::get<Model::Node*>(nodeAndLinkId);
    const auto& linkId = std::get<std::string>(nodeAndLinkId);
    return node->accept(kdl::overload(
      [&](const Model::WorldNode*) -> std::tuple<Model::Node*, std::string> {
        ensure(false, "no unexpected world node");
      },
      [](const Model::LayerNode*) -> std::tuple<Model::Node*, std::string> {
        ensure(false, "no unexpected layer node");
      },
      [&](const Model::GroupNode* groupNode) -> std::tuple<Model::Node*, std::string> {
        auto group = groupNode->group();
        auto oldLinkId = group.linkId();
        group.setLinkId(std::move(linkId));
        return {node, std::move(oldLinkId)};
      },
      [&](const Model::EntityNode* entityNode) -> std::tuple<Model::Node*, std::string> {
        auto entity = entityNode->entity();
        auto oldLinkId = entity.linkId();
        entity.setLinkId(std::move(linkId));
        return {node, std::move(oldLinkId)};
      },
      [&](const Model::BrushNode* brushNode) -> std::tuple<Model::Node*, std::string> {
        auto brush = brushNode->brush();
        auto oldLinkId = brush.linkId();
        brush.setLinkId(std::move(linkId));
        return {node, std::move(oldLinkId)};
      },
      [&](const Model::PatchNode* patchNode) -> std::tuple<Model::Node*, std::string> {
        auto patch = patchNode->patch();
        auto oldLinkId = patch.linkId();
        patch.setLinkId(std::move(linkId));
        return {node, std::move(oldLinkId)};
      }));
  });
}
} // namespace

std::unique_ptr<CommandResult> SetLinkIdsCommand::doPerformDo(MapDocumentCommandFacade*)
{
  m_linkIds = setLinkIds(m_linkIds);
  return std::make_unique<CommandResult>(true);
}

std::unique_ptr<CommandResult> SetLinkIdsCommand::doPerformUndo(MapDocumentCommandFacade*)
{
  m_linkIds = setLinkIds(m_linkIds);
  return std::make_unique<CommandResult>(true);
}

bool SetLinkIdsCommand::doCollateWith(UndoableCommand&)
{
  return false;
}

} // namespace TrenchBroom::View
