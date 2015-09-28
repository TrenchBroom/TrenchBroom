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

#include "CreateComplexBrushToolAdapter3D.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/PickResult.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderService.h"
#include "View/CreateComplexBrushTool.h"
#include "View/Grid.h"
#include "View/InputState.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        CreateComplexBrushToolAdapter3D::CreateComplexBrushToolAdapter3D(CreateComplexBrushTool* tool, MapDocumentWPtr document) :
        m_tool(tool),
        m_document(document) {
            assert(tool != NULL);
        }

        void CreateComplexBrushToolAdapter3D::performCreateBrush() {
            m_tool->createBrush();
            m_currentPolyhedron = Polyhedron3();
        }

        Tool* CreateComplexBrushToolAdapter3D::doGetTool() {
            return m_tool;
        }

        bool CreateComplexBrushToolAdapter3D::doMouseClick(const InputState& inputState) {
            if (!inputState.mouseButtonsDown(MouseButtons::MBLeft))
                return false;
            if (!inputState.checkModifierKeys(MK_No, MK_No, MK_No))
                return false;
            
            const Model::PickResult& pickResult = inputState.pickResult();
            const Model::Hit& hit = pickResult.query().pickable().type(Model::Brush::BrushHit).occluded().first();
            if (!hit.isMatch())
                return false;
            
            MapDocumentSPtr document = lock(m_document);
            const Grid& grid = document->grid();
            
            const Model::BrushFace* face = Model::hitToFace(hit);
            const Vec3 snapped = grid.snap(hit.hitPoint(), face->boundary());
            
            m_currentPolyhedron.addPoint(snapped);
            m_tool->update(m_currentPolyhedron);
            
            return true;
        }

        bool CreateComplexBrushToolAdapter3D::doMouseDoubleClick(const InputState& inputState) {
            if (!inputState.mouseButtonsDown(MouseButtons::MBLeft))
                return false;
            if (!inputState.checkModifierKeys(MK_No, MK_No, MK_No))
                return false;
            
            const Model::PickResult& pickResult = inputState.pickResult();
            const Model::Hit& hit = pickResult.query().pickable().type(Model::Brush::BrushHit).occluded().first();
            if (!hit.isMatch())
                return false;
            
            const Model::BrushFace* face = Model::hitToFace(hit);
            
            const Model::BrushFace::VertexList vertices = face->vertices();
            Model::BrushFace::VertexList::const_iterator it, end;
            for (it = vertices.begin(), end = vertices.end(); it != end; ++it)
                m_currentPolyhedron.addPoint((*it)->position());
            m_tool->update(m_currentPolyhedron);
            
            return true;
        }

        bool CreateComplexBrushToolAdapter3D::doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            if (!inputState.mouseButtonsDown(MouseButtons::MBLeft))
                return false;
            if (!inputState.checkModifierKeys(MK_No, MK_No, MK_No))
                return false;
            
            const Model::PickResult& pickResult = inputState.pickResult();
            const Model::Hit& hit = pickResult.query().pickable().type(Model::Brush::BrushHit).occluded().first();
            if (!hit.isMatch())
                return false;
            
            const Model::BrushFace* face = Model::hitToFace(hit);
            m_initialPoint = initialPoint = hit.hitPoint();
            m_plane = plane = face->boundary();
            m_lastPolyhedron = m_currentPolyhedron;
            
            updatePolyhedron(m_initialPoint);
            return true;
        }

        bool CreateComplexBrushToolAdapter3D::doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
            updatePolyhedron(curPoint);
            refPoint = curPoint;
            return true;
        }

        void CreateComplexBrushToolAdapter3D::doEndPlaneDrag(const InputState& inputState) {}

        void CreateComplexBrushToolAdapter3D::doCancelPlaneDrag() {
            m_currentPolyhedron = m_lastPolyhedron;
            m_tool->update(m_currentPolyhedron);
        }

        void CreateComplexBrushToolAdapter3D::doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {}

        void CreateComplexBrushToolAdapter3D::updatePolyhedron(const Vec3& current) {
            MapDocumentSPtr document = lock(m_document);
            const Grid& grid = document->grid();

            const Math::Axis::Type axis = m_plane.normal.firstComponent();
            const Plane3 swizzledPlane(swizzle(m_plane.anchor(), axis), swizzle(m_plane.normal, axis));
            const Vec3 theMin = swizzle(grid.snapDown(min(m_initialPoint, current)), axis);
            const Vec3 theMax = swizzle(grid.snapUp  (max(m_initialPoint, current)), axis);
            
            const Vec2     topLeft2(theMin.x(), theMin.y());
            const Vec2    topRight2(theMax.x(), theMin.y());
            const Vec2  bottomLeft2(theMin.x(), theMax.y());
            const Vec2 bottomRight2(theMax.x(), theMax.y());
            
            const Vec3     topLeft3 = unswizzle(Vec3(topLeft2,     swizzledPlane.zAt(topLeft2)),     axis);
            const Vec3    topRight3 = unswizzle(Vec3(topRight2,    swizzledPlane.zAt(topRight2)),    axis);
            const Vec3  bottomLeft3 = unswizzle(Vec3(bottomLeft2,  swizzledPlane.zAt(bottomLeft2)),  axis);
            const Vec3 bottomRight3 = unswizzle(Vec3(bottomRight2, swizzledPlane.zAt(bottomRight2)), axis);
            
            m_currentPolyhedron = m_lastPolyhedron;
            m_currentPolyhedron.addPoint(topLeft3);
            m_currentPolyhedron.addPoint(bottomLeft3);
            m_currentPolyhedron.addPoint(bottomRight3);
            m_currentPolyhedron.addPoint(topRight3);
            
            m_tool->update(m_currentPolyhedron);
        }

        void CreateComplexBrushToolAdapter3D::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {}

        void CreateComplexBrushToolAdapter3D::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_tool->render(renderContext, renderBatch);
            
            if (!m_currentPolyhedron.empty()) {
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(pref(Preferences::HandleColor));
                renderService.setLineWidth(2.0f);
                
                const Polyhedron3::EdgeList& edges = m_currentPolyhedron.edges();
                Polyhedron3::EdgeList::const_iterator eIt, eEnd;
                for (eIt = edges.begin(), eEnd = edges.end(); eIt != eEnd; ++eIt) {
                    const Polyhedron3::Edge* edge = *eIt;
                    renderService.renderLine(edge->firstVertex()->position(), edge->secondVertex()->position());
                }
                
                const Polyhedron3::VertexList& vertices = m_currentPolyhedron.vertices();
                Polyhedron3::VertexList::const_iterator vIt, vEnd;
                for (vIt = vertices.begin(), vEnd = vertices.end(); vIt != vEnd; ++vIt) {
                    const Polyhedron3::Vertex* vertex = *vIt;
                    renderService.renderPointHandle(vertex->position());
                }
            }
        }

        bool CreateComplexBrushToolAdapter3D::doCancel() {
            if (m_currentPolyhedron.empty())
                return false;
            
            m_currentPolyhedron = Polyhedron3();
            m_tool->update(m_currentPolyhedron);
            return true;
        }
    }
}
