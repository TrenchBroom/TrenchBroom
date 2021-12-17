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

#include "GroupRenderer.h"

#include "Model/EditorContext.h"
#include "Model/GroupNode.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/GLVertexType.h"
#include "Renderer/PrimType.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderService.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/TextAnchor.h"

#include <vector>

namespace TrenchBroom {
namespace Renderer {
class GroupRenderer::GroupNameAnchor : public TextAnchor3D {
private:
  const Model::GroupNode* m_group;

public:
  GroupNameAnchor(const Model::GroupNode* group)
    : m_group(group) {}

private:
  vm::vec3f basePosition() const override {
    auto position = vm::vec3f(m_group->logicalBounds().center());
    position[2] = float(m_group->logicalBounds().max.z());
    position[2] += 2.0f;
    return position;
  }

  TextAlignment::Type alignment() const override { return TextAlignment::Bottom; }
};

GroupRenderer::GroupRenderer(const Model::EditorContext& editorContext)
  : m_editorContext(editorContext)
  , m_boundsValid(false)
  , m_showOverlays(true)
  , m_showOccludedOverlays(false) {}

void GroupRenderer::invalidate() {
  invalidateBounds();
}

void GroupRenderer::clear() {
  m_groups.clear();
  m_boundsRenderer = DirectEdgeRenderer();
}

void GroupRenderer::addGroup(const Model::GroupNode* group) {
  if (m_groups.insert(group).second) {
    invalidate();
  }
}

void GroupRenderer::removeGroup(const Model::GroupNode* group) {
  if (auto it = m_groups.find(group); it != std::end(m_groups)) {
    m_groups.erase(it);
    invalidate();
  }
}

void GroupRenderer::invalidateGroup(const Model::GroupNode*) {
  invalidate();
}

void GroupRenderer::setShowOverlays(const bool showOverlays) {
  m_showOverlays = showOverlays;
}

void GroupRenderer::setOverlayTextColor(const RenderType type, const Color& overlayTextColor) {
  m_overlayTextColor[type] = overlayTextColor;
}

void GroupRenderer::setOverlayBackgroundColor(
  const RenderType type, const Color& overlayBackgroundColor) {
  m_overlayBackgroundColor[type] = overlayBackgroundColor;
}

void GroupRenderer::setShowOccludedOverlays(const bool showOccludedOverlays) {
  m_showOccludedOverlays = showOccludedOverlays;
}

void GroupRenderer::render(RenderContext& renderContext, RenderBatch& renderBatch) {
  if (!m_groups.empty()) {
    if (renderContext.showGroupBounds()) {
      renderBounds(renderContext, renderBatch);
      renderNames(renderContext, renderBatch);
    }
  }
}

void GroupRenderer::renderBounds(RenderContext&, RenderBatch& renderBatch) {
  if (!m_boundsValid) {
    validateBounds();
  }

  m_boundsRenderer.renderOnTop(renderBatch);
  m_boundsRenderer.render(renderBatch);
}

static RenderType renderType(const Model::GroupNode* group) {
  if (group->locked()) {
    return RenderType::Locked;
  } else if (selected(group) || group->opened()) {
    return RenderType::Selected;
  } else {
    return RenderType::Default;
  }
}

void GroupRenderer::renderNames(RenderContext& renderContext, RenderBatch& renderBatch) {
  if (m_showOverlays) {
    Renderer::RenderService renderService(renderContext, renderBatch);
    for (const auto* group : m_groups) {
      const auto type = renderType(group);

      renderService.setBackgroundColor(m_overlayBackgroundColor[type]);

      if (shouldRenderGroup(group)) {
        if (type != RenderType::Default) {
          renderService.setForegroundColor(m_overlayTextColor[type]);
        } else {
          renderService.setForegroundColor(groupColor(group));
        }

        const GroupNameAnchor anchor(group);
        if (m_showOccludedOverlays) {
          renderService.setShowOccludedObjects();
        } else {
          renderService.setHideOccludedObjects();
        }
        renderService.renderString(groupString(group), anchor);
      }
    }
  }
}

void GroupRenderer::invalidateBounds() {
  m_boundsValid = false;
}

void GroupRenderer::validateBounds() {
  std::vector<GLVertexTypes::P3C4::Vertex> vertices;
  vertices.reserve(24 * m_groups.size());

  for (const Model::GroupNode* group : m_groups) {
    if (shouldRenderGroup(group)) {
      const auto color = groupColor(group);
      group->logicalBounds().for_each_edge([&](const vm::vec3& v1, const vm::vec3& v2) {
        vertices.emplace_back(vm::vec3f(v1), color);
        vertices.emplace_back(vm::vec3f(v2), color);
      });
    }
  }

  m_boundsRenderer = DirectEdgeRenderer(VertexArray::move(std::move(vertices)), PrimType::Lines);

  m_boundsValid = true;
}

bool GroupRenderer::shouldRenderGroup(const Model::GroupNode* group) const {
  const auto& currentGroup = m_editorContext.currentGroup();
  const auto* parentGroup = group->containingGroup();
  return parentGroup == currentGroup && m_editorContext.visible(group);
}

AttrString GroupRenderer::groupString(const Model::GroupNode* groupNode) const {
  if (groupNode->group().linkedGroupId()) {
    return groupNode->name() + " (linked)";
  } else {
    return groupNode->name();
  }
}

Color GroupRenderer::groupColor(const Model::GroupNode* groupNode) const {
  const auto type = renderType(groupNode);
  if (type == RenderType::Locked) {
    return pref(Preferences::LockedEdgeColor);
  } else if (type == RenderType::Selected) {
    return pref(Preferences::SelectedEdgeColor);
  }

  return groupNode->group().linkedGroupId() ? pref(Preferences::LinkedGroupColor)
                                            : pref(Preferences::DefaultGroupColor);
}
} // namespace Renderer
} // namespace TrenchBroom
