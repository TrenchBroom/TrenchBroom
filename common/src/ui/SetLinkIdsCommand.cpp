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
#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h" // IWYU pragma: keep
#include "mdl/GroupNode.h"  // IWYU pragma: keep
#include "mdl/LayerNode.h"
#include "mdl/Node.h"
#include "mdl/Object.h"
#include "mdl/PatchNode.h" // IWYU pragma: keep
#include "mdl/WorldNode.h"
#include "ui/MapDocumentCommandFacade.h"

#include "kdl/range_to_vector.h"
#include "kdl/vector_utils.h"

#include <ranges>

namespace tb::ui
{
namespace
{

auto setLinkIds(const std::vector<std::tuple<mdl::Node*, std::string>>& linkIds)
{
  return linkIds | std::views::transform([](const auto& nodeAndLinkId) {
           auto* node = std::get<mdl::Node*>(nodeAndLinkId);
           const auto& linkId = std::get<std::string>(nodeAndLinkId);
           return node->accept(kdl::overload(
             [&](const mdl::WorldNode*) -> std::tuple<mdl::Node*, std::string> {
               ensure(false, "no unexpected world node");
             },
             [](const mdl::LayerNode*) -> std::tuple<mdl::Node*, std::string> {
               ensure(false, "no unexpected layer node");
             },
             [&](mdl::Object* object) -> std::tuple<mdl::Node*, std::string> {
               auto oldLinkId = object->linkId();
               object->setLinkId(std::move(linkId));
               return {node, std::move(oldLinkId)};
             }));
         })
         | kdl::to_vector;
}

} // namespace

SetLinkIdsCommand::SetLinkIdsCommand(
  const std::string& name, std::vector<std::tuple<mdl::Node*, std::string>> linkIds)
  : UndoableCommand{name, true}
  , m_linkIds{std::move(linkIds)}
{
}

SetLinkIdsCommand::~SetLinkIdsCommand() = default;

std::unique_ptr<CommandResult> SetLinkIdsCommand::doPerformDo(MapDocumentCommandFacade&)
{
  m_linkIds = setLinkIds(m_linkIds);
  return std::make_unique<CommandResult>(true);
}

std::unique_ptr<CommandResult> SetLinkIdsCommand::doPerformUndo(MapDocumentCommandFacade&)
{
  m_linkIds = setLinkIds(m_linkIds);
  return std::make_unique<CommandResult>(true);
}

bool SetLinkIdsCommand::doCollateWith(UndoableCommand&)
{
  return false;
}

} // namespace tb::ui
