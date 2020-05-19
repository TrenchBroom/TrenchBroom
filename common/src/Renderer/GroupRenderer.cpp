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

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/EditorContext.h"
#include "Model/GroupNode.h"
#include "Renderer/GLVertexType.h"
#include "Renderer/PrimType.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderService.h"
#include "Renderer/TextAnchor.h"

#include <vector>

#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class GroupRenderer::GroupNameAnchor : public TextAnchor3D {
        private:
            const Model::GroupNode* m_group;
        public:
            GroupNameAnchor(const Model::GroupNode* group) :
            m_group(group) {}
        private:
            vm::vec3f basePosition() const override {
                auto position = vm::vec3f(m_group->logicalBounds().center());
                position[2] = float(m_group->logicalBounds().max.z());
                position[2] += 2.0f;
                return position;
            }

            TextAlignment::Type alignment() const override {
                return TextAlignment::Bottom;
            }
        };

        GroupRenderer::GroupRenderer(const Model::EditorContext& editorContext) :
        m_editorContext(editorContext),
        m_boundsValid(false),
        m_showOverlays(true),
        m_showOccludedOverlays(false),
        m_overrideBoundsColor(false),
        m_showOccludedBounds(false) {}

        void GroupRenderer::setGroups(const std::vector<Model::GroupNode*>& groups) {
            m_groups = groups;
            invalidate();
        }

        void GroupRenderer::invalidate() {
            invalidateBounds();
        }

        void GroupRenderer::clear() {
            m_groups.clear();
            m_boundsRenderer = DirectEdgeRenderer();
        }

        void GroupRenderer::setShowOverlays(const bool showOverlays) {
            m_showOverlays = showOverlays;
        }

        void GroupRenderer::setOverlayTextColor(const Color& overlayTextColor) {
            m_overlayTextColor = overlayTextColor;
        }

        void GroupRenderer::setOverlayBackgroundColor(const Color& overlayBackgroundColor) {
            m_overlayBackgroundColor = overlayBackgroundColor;
        }

        void GroupRenderer::setShowOccludedOverlays(const bool showOccludedOverlays) {
            m_showOccludedOverlays = showOccludedOverlays;
        }

        void GroupRenderer::setOverrideBoundsColor(const bool overrideBoundsColor) {
            m_overrideBoundsColor = overrideBoundsColor;
        }

        void GroupRenderer::setBoundsColor(const Color& boundsColor) {
            m_boundsColor = boundsColor;
        }

        void GroupRenderer::setShowOccludedBounds(const bool showOccludedBounds) {
            m_showOccludedBounds = showOccludedBounds;
        }

        void GroupRenderer::setOccludedBoundsColor(const Color& occludedBoundsColor) {
            m_occludedBoundsColor = occludedBoundsColor;
        }

        void GroupRenderer::render(RenderContext& renderContext, RenderBatch& renderBatch) {
            if (!m_groups.empty()) {
                if (renderContext.showGroupBounds()) {
                    renderBounds(renderContext, renderBatch);
                }
                renderNames(renderContext, renderBatch);
            }
        }

        void GroupRenderer::renderBounds(RenderContext&, RenderBatch& renderBatch) {
            if (!m_boundsValid) {
                validateBounds();
            }

            if (m_showOccludedBounds) {
                m_boundsRenderer.renderOnTop(renderBatch, m_overrideBoundsColor, m_occludedBoundsColor);
            }

            m_boundsRenderer.render(renderBatch, m_overrideBoundsColor, m_boundsColor);
        }

        void GroupRenderer::renderNames(RenderContext& renderContext, RenderBatch& renderBatch) {
            if (m_showOverlays) {
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(m_overlayTextColor);
                renderService.setBackgroundColor(m_overlayBackgroundColor);

                for (const auto* group : m_groups) {
                    if (shouldRenderGroup(group)) {
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

        struct GroupRenderer::BuildColoredBoundsVertices {
            using Vertex = GLVertexTypes::P3C4::Vertex;
            std::vector<Vertex>& vertices;
            Color color;

            BuildColoredBoundsVertices(std::vector<Vertex>& i_vertices, const Color& i_color) :
            vertices(i_vertices),
            color(i_color) {}

            void operator()(const vm::vec3& v1, const vm::vec3& v2) {
                vertices.emplace_back(vm::vec3f(v1), color);
                vertices.emplace_back(vm::vec3f(v2), color);
            }
        };

        struct GroupRenderer::BuildBoundsVertices {
            std::vector<GLVertexTypes::P3::Vertex>& vertices;

            BuildBoundsVertices(std::vector<GLVertexTypes::P3::Vertex>& i_vertices) :
            vertices(i_vertices) {}

            void operator()(const vm::vec3& v1, const vm::vec3& v2) {
                vertices.emplace_back(vm::vec3f(v1));
                vertices.emplace_back(vm::vec3f(v2));
            }
        };

        void GroupRenderer::validateBounds() {
            if (m_overrideBoundsColor) {
                std::vector<GLVertexTypes::P3::Vertex> vertices;
                vertices.reserve(24 * m_groups.size());

                BuildBoundsVertices boundsBuilder(vertices);
                for (const Model::GroupNode* group : m_groups) {
                    if (shouldRenderGroup(group)) {
                        group->logicalBounds().for_each_edge(boundsBuilder);
                    }
                }

                m_boundsRenderer = DirectEdgeRenderer(VertexArray::move(std::move(vertices)), PrimType::Lines);
            } else {
                std::vector<GLVertexTypes::P3C4::Vertex> vertices;
                vertices.reserve(24 * m_groups.size());

                for (const Model::GroupNode* group : m_groups) {
                    if (shouldRenderGroup(group)) {
                        BuildColoredBoundsVertices boundsBuilder(vertices, boundsColor(group));
                        group->logicalBounds().for_each_edge(boundsBuilder);
                    }
                }

                m_boundsRenderer = DirectEdgeRenderer(VertexArray::move(std::move(vertices)), PrimType::Lines);
            }

            m_boundsValid = true;
        }

        bool GroupRenderer::shouldRenderGroup(const Model::GroupNode* group) const {
            const auto& currentGroup = m_editorContext.currentGroup();
            const auto* parentGroup = group->group();
            return parentGroup == currentGroup && m_editorContext.visible(group);
        }

        AttrString GroupRenderer::groupString(const Model::GroupNode* group) const {
            return group->name();
        }

        const Color& GroupRenderer::boundsColor(const Model::GroupNode* /* group */) const {
            return pref(Preferences::DefaultGroupColor);
        }
    }
}
