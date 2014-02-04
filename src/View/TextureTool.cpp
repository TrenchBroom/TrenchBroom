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

#include "TextureTool.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/Brush.h"
#include "Model/BrushEdge.h"
#include "Model/BrushFace.h"
#include "Model/BrushVertex.h"
#include "Model/HitAdapter.h"
#include "Model/HitFilters.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexSpec.h"
#include "View/InputState.h"
#include "View/ControllerFacade.h"
#include "View/Grid.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        TextureTool::TextureTool(BaseTool* next, MapDocumentWPtr document, ControllerWPtr controller) :
        Tool(next, document, controller),
        m_face(NULL) {}

        bool TextureTool::initiallyActive() const {
            return false;
        }
        
        bool TextureTool::doActivate(const InputState& inputState) {
            return true;
        }
        
        bool TextureTool::doDeactivate(const InputState& inputState) {
            return true;
        }
        
        void TextureTool::doMouseMove(const InputState& inputState) {
        }

        bool TextureTool::doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            assert(m_face == NULL);
            if (!applies(inputState))
                return false;

            const Model::PickResult::FirstHit first = Model::firstHit(inputState.pickResult(), Model::Brush::BrushHit, document()->filter(), true);
            assert(first.matches);
            
            m_face = Model::hitAsFace(first.hit);
            plane = Plane3(first.hit.hitPoint(), m_face->boundary().normal);
            initialPoint = first.hit.hitPoint();
            return true;
        }
        
        bool TextureTool::doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
            assert(m_face != NULL);
            assert(m_face->selected());
            
            const Grid& grid = document()->grid();
            const Vec2 last = m_face->convertToTexCoordSystem(refPoint);
            const Vec2 cur = m_face->convertToTexCoordSystem(curPoint);
            const Vec2 delta = grid.snap(cur - last);
            if (delta.null())
                return true;
            
            const Model::BrushFaceList& faces = document()->allSelectedFaces();
            Model::BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                const Vec2 last = face->convertToTexCoordSystem(refPoint);
                const Vec2 cur = face->convertToTexCoordSystem(curPoint);
                const Vec2 delta = grid.snap(cur - last);

                const Model::BrushFaceList applyTo(1, face);
                controller()->beginUndoableGroup("Move Texture");
                if (delta.x() != 0.0)
                    controller()->setFaceXOffset(applyTo, -delta.x(), true);
                if (delta.y() != 0.0)
                    controller()->setFaceYOffset(applyTo, -delta.y(), true);
                controller()->closeGroup();
            }
            
            
            const Vec3 newRef = m_face->convertToWorldCoordSystem(last + delta);
            refPoint = newRef;
            return true;
        }
        
        void TextureTool::doEndPlaneDrag(const InputState& inputState) {
            m_face = NULL;
        }
        
        void TextureTool::doCancelPlaneDrag(const InputState& inputState) {
            m_face = NULL;
        }

        void TextureTool::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
            renderContext.clearTintSelection();
        }

        void TextureTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext) {
            if (dragging())
                return;
            
            const Model::PickResult::FirstHit first = Model::firstHit(inputState.pickResult(), Model::Brush::BrushHit, document()->filter(), true);
            if (!first.matches)
                return;

            const Model::BrushFace* face = Model::hitAsFace(first.hit);

            PreferenceManager& prefs = PreferenceManager::instance();
            Renderer::EdgeRenderer edgeRenderer = buildEdgeRenderer(face);
            
            glDisable(GL_DEPTH_TEST);
            edgeRenderer.setUseColor(true);
            edgeRenderer.setColor(prefs.get(Preferences::ResizeHandleColor));
            edgeRenderer.render(renderContext);
            glEnable(GL_DEPTH_TEST);
        }
        
        Renderer::EdgeRenderer TextureTool::buildEdgeRenderer(const Model::BrushFace* face) const {
            assert(face != NULL);
            
            const Model::BrushEdgeList& edges = face->edges();

            typedef Renderer::VertexSpecs::P3::Vertex Vertex;
            Vertex::List vertices(2 * edges.size());

            for (size_t i = 0; i < edges.size(); ++i) {
                const Model::BrushEdge* edge = edges[i];
                vertices[2 * i + 0] = Vertex(edge->start->position);
                vertices[2 * i + 1] = Vertex(edge->end->position);
            }
            
            return Renderer::EdgeRenderer(Renderer::VertexArray::swap(GL_LINES, vertices));
        }

        bool TextureTool::applies(const InputState& inputState) const {
            if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                return false;
            
            const Model::PickResult::FirstHit first = Model::firstHit(inputState.pickResult(), Model::Brush::BrushHit, document()->filter(), true);
            if (!first.matches)
                return false;
            const Model::BrushFace* face = Model::hitAsFace(first.hit);
            const Model::Brush* brush = face->parent();
            return face->selected() || brush->selected();
        }
    }
}
