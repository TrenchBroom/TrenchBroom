/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
#include "Model/Group.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderService.h"
#include "Renderer/TextAnchor.h"
#include "Renderer/VertexSpec.h"

namespace TrenchBroom {
    namespace Renderer {
        class GroupRenderer::GroupNameAnchor : public TextAnchor {
        private:
            const Model::Group* m_group;
        public:
            GroupNameAnchor(const Model::Group* group) :
            m_group(group) {}
        private:
            Vec3f basePosition() const {
                Vec3f position = m_group->bounds().center();
                position[2] = float(m_group->bounds().max.z());
                position[2] += 2.0f;
                return position;
            }
            
            TextAlignment::Type alignment() const {
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
        
        void GroupRenderer::setGroups(const Model::GroupList& groups) {
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
                renderBounds(renderContext, renderBatch);
                renderNames(renderContext, renderBatch);
            }
        }
        
        void GroupRenderer::renderBounds(RenderContext& renderContext, RenderBatch& renderBatch) {
            if (!m_boundsValid)
                validateBounds();
            
            if (m_showOccludedBounds)
                m_boundsRenderer.renderOnTop(renderBatch, m_overrideBoundsColor, m_occludedBoundsColor);
            m_boundsRenderer.render(renderBatch, m_overrideBoundsColor, m_boundsColor);
        }
        
        void GroupRenderer::renderNames(RenderContext& renderContext, RenderBatch& renderBatch) {
            if (m_showOverlays) {
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(m_overlayTextColor);
                renderService.setBackgroundColor(m_overlayBackgroundColor);
                
                Model::GroupList::const_iterator it, end;
                for (it = m_groups.begin(), end = m_groups.end(); it != end; ++it) {
                    const Model::Group* group = *it;
                    if (m_editorContext.visible(group)) {
                        const GroupNameAnchor anchor(group);
                        if (m_showOccludedOverlays)
                            renderService.renderStringOnTop(groupString(group), anchor);
                        else
                            renderService.renderString(groupString(group), anchor);
                    }
                }
            }
        }
        
        void GroupRenderer::invalidateBounds() {
            m_boundsValid = false;
        }
        
        struct GroupRenderer::BuildColoredBoundsVertices {
            VertexSpecs::P3C4::Vertex::List& vertices;
            Color color;
            
            BuildColoredBoundsVertices(VertexSpecs::P3C4::Vertex::List& i_vertices, const Color& i_color) :
            vertices(i_vertices),
            color(i_color) {}
            
            void operator()(const Vec3& v1, const Vec3& v2) {
                vertices.push_back(VertexSpecs::P3C4::Vertex(v1, color));
                vertices.push_back(VertexSpecs::P3C4::Vertex(v2, color));
            }
        };
        
        struct GroupRenderer::BuildBoundsVertices {
            VertexSpecs::P3::Vertex::List& vertices;
            
            BuildBoundsVertices(VertexSpecs::P3::Vertex::List& i_vertices) :
            vertices(i_vertices) {}
            
            void operator()(const Vec3& v1, const Vec3& v2) {
                vertices.push_back(VertexSpecs::P3::Vertex(v1));
                vertices.push_back(VertexSpecs::P3::Vertex(v2));
            }
        };
        
        void GroupRenderer::validateBounds() {
            if (m_overrideBoundsColor) {
                VertexSpecs::P3::Vertex::List vertices;
                vertices.reserve(24 * m_groups.size());
                
                BuildBoundsVertices boundsBuilder(vertices);
                Model::GroupList::const_iterator it, end;
                for (it = m_groups.begin(), end = m_groups.end(); it != end; ++it) {
                    const Model::Group* group = *it;
                    if (m_editorContext.visible(group)) {
                        eachBBoxEdge(group->bounds(), boundsBuilder);
                    }
                }
                
                m_boundsRenderer = DirectEdgeRenderer(VertexArray::swap(vertices), GL_LINES);
            } else {
                VertexSpecs::P3C4::Vertex::List vertices;
                vertices.reserve(24 * m_groups.size());
                
                Model::GroupList::const_iterator it, end;
                for (it = m_groups.begin(), end = m_groups.end(); it != end; ++it) {
                    const Model::Group* group = *it;
                    if (m_editorContext.visible(group)) {
                        BuildColoredBoundsVertices boundsBuilder(vertices, boundsColor(group));
                        eachBBoxEdge(group->bounds(), boundsBuilder);
                    }
                }
                
                m_boundsRenderer = DirectEdgeRenderer(VertexArray::swap(vertices), GL_LINES);
            }
            
            m_boundsValid = true;
        }
        
        AttrString GroupRenderer::groupString(const Model::Group* group) const {
            return group->name();
        }
        
        const Color& GroupRenderer::boundsColor(const Model::Group* group) const {
            return pref(Preferences::DefaultGroupColor);
        }
    }
}
