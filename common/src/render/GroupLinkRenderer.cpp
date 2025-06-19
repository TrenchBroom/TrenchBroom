/*
 Copyright (C) 2020 Kristian Duske

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

#include "GroupLinkRenderer.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "mdl/EditorContext.h"
#include "mdl/GroupNode.h"
#include "mdl/LinkedGroupUtils.h"
#include "mdl/ModelUtils.h"
#include "ui/MapDocument.h"

#include "kdl/memory_utils.h"

#include <utility>

namespace tb::render
{

GroupLinkRenderer::GroupLinkRenderer(std::weak_ptr<ui::MapDocument> document)
  : m_document{std::move(document)}
{
}

static vm::vec3f getLinkAnchorPosition(const mdl::GroupNode& groupNode)
{
  return vm::vec3f(groupNode.logicalBounds().center());
}

std::vector<LinkRenderer::LineVertex> GroupLinkRenderer::getLinks()
{
  auto document = kdl::mem_lock(m_document);
  auto links = std::vector<LineVertex>{};

  const auto selectedGroupNodes = document->selection().groups;

  const auto& editorContext = document->editorContext();
  const auto* groupNode = selectedGroupNodes.size() == 1 ? selectedGroupNodes.front()
                                                         : editorContext.currentGroup();

  if (groupNode)
  {
    const auto& linkId = groupNode->linkId();
    const auto linkedGroupNodes =
      mdl::collectGroupsWithLinkId({document->world()}, linkId);

    const auto linkColor = pref(Preferences::LinkedGroupColor);
    const auto sourcePosition = getLinkAnchorPosition(*groupNode);
    for (const auto* linkedGroupNode : linkedGroupNodes)
    {
      if (linkedGroupNode != groupNode && editorContext.visible(*linkedGroupNode))
      {
        const auto targetPosition = getLinkAnchorPosition(*linkedGroupNode);
        links.emplace_back(sourcePosition, linkColor);
        links.emplace_back(targetPosition, linkColor);
      }
    }
  }

  return links;
}

} // namespace tb::render
