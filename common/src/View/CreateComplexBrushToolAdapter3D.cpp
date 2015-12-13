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
#include <algorithm>

namespace TrenchBroom {
    namespace View {
        class CreateComplexBrushToolAdapter3D::DragDelegate {
        protected:
            const Grid& m_grid;
            Polyhedron3& m_currentPolyhedron;
            const Polyhedron3 m_initialPolyhedron;
        protected:
            DragDelegate(const Grid& grid, Polyhedron3& polyhedron) :
            m_grid(grid),
            m_currentPolyhedron(polyhedron),
            m_initialPolyhedron(polyhedron) {}
        public:
            virtual ~DragDelegate() {}
        };
        
        class CreateComplexBrushToolAdapter3D::DrawFaceDelegate : public DragDelegate, public PlaneDragPolicy {
        private:
            Plane3 m_plane;
            Vec3 m_initialPoint;
        public:
            DrawFaceDelegate(const Grid& grid, Polyhedron3& polyhedron) :
            DragDelegate(grid, polyhedron) {}
        private:
            bool doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
                const Model::PickResult& pickResult = inputState.pickResult();
                const Model::Hit& hit = pickResult.query().pickable().type(Model::Brush::BrushHit).occluded().first();
                if (!hit.isMatch())
                    return false;
                
                const Model::BrushFace* face = Model::hitToFace(hit);
                m_initialPoint = initialPoint = hit.hitPoint();
                m_plane = plane = face->boundary();
                
                updatePolyhedron(m_initialPoint);
                
                return true;
            }
            
            bool doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
                updatePolyhedron(curPoint);
                refPoint = curPoint;
                return true;
            }

            void doEndPlaneDrag(const InputState& inputState) {}
            
            void doCancelPlaneDrag() {
                m_currentPolyhedron = m_initialPolyhedron;
            }
            
            void doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {}
        private:
            void updatePolyhedron(const Vec3& current) {
                const Math::Axis::Type axis = m_plane.normal.firstComponent();
                const Plane3 swizzledPlane(swizzle(m_plane.anchor(), axis), swizzle(m_plane.normal, axis));
                const Vec3 theMin = swizzle(m_grid.snapDown(min(m_initialPoint, current)), axis);
                const Vec3 theMax = swizzle(m_grid.snapUp  (max(m_initialPoint, current)), axis);
                
                const Vec2     topLeft2(theMin.x(), theMin.y());
                const Vec2    topRight2(theMax.x(), theMin.y());
                const Vec2  bottomLeft2(theMin.x(), theMax.y());
                const Vec2 bottomRight2(theMax.x(), theMax.y());
                
                const Vec3     topLeft3 = unswizzle(Vec3(topLeft2,     swizzledPlane.zAt(topLeft2)),     axis);
                const Vec3    topRight3 = unswizzle(Vec3(topRight2,    swizzledPlane.zAt(topRight2)),    axis);
                const Vec3  bottomLeft3 = unswizzle(Vec3(bottomLeft2,  swizzledPlane.zAt(bottomLeft2)),  axis);
                const Vec3 bottomRight3 = unswizzle(Vec3(bottomRight2, swizzledPlane.zAt(bottomRight2)), axis);
                
                m_currentPolyhedron = m_initialPolyhedron;
                m_currentPolyhedron.addPoint(topLeft3);
                m_currentPolyhedron.addPoint(bottomLeft3);
                m_currentPolyhedron.addPoint(bottomRight3);
                m_currentPolyhedron.addPoint(topRight3);
            }
        };
        
        class CreateComplexBrushToolAdapter3D::DuplicateFaceDelegate : public DragDelegate, public LineDragPolicy {
        private:
            Vec3 m_dragDir;
        public:
            DuplicateFaceDelegate(const Grid& grid, Polyhedron3& polyhedron) :
            DragDelegate(grid, polyhedron) {}
        private:
            bool doStartLineDrag(const InputState& inputState, Line3& line, FloatType& initialDist) {
                if (!m_currentPolyhedron.polygon())
                    return false;
                
                const Polyhedron3::FaceHit hit = m_currentPolyhedron.pickFace(inputState.pickRay());
                const Vec3 origin    = inputState.pickRay().pointAtDistance(hit.distance);
                const Vec3 direction = hit.face->normal();
                
                line = Line3(origin, direction);
                initialDist = 0.0;
                m_dragDir = line.direction;
                
                return true;
            }
            
            bool doLineDrag(const InputState& inputState, FloatType lastDist, FloatType curDist, FloatType& refDist) {
                assert(m_initialPolyhedron.polygon());
                
                const Vec3 rayDelta             = curDist * m_dragDir;
                const Vec3 rayAxis              = m_dragDir.firstAxis();
                const FloatType axisDistance    = rayDelta.dot(rayAxis);
                const FloatType snappedDistance = m_grid.snap(axisDistance);
                const FloatType snappedRayDist  = rayAxis.inverseDot(snappedDistance, m_dragDir);
                const Vec3 snappedRayDelta      = snappedRayDist * m_dragDir;
                
                const Polyhedron3::Face* face = m_initialPolyhedron.faces().front();
                const Vec3::List points = face->vertexPositions() + snappedRayDelta;
                
                m_currentPolyhedron = m_initialPolyhedron;
                m_currentPolyhedron.addPoints(points);
                
                refDist = curDist;
                return true;
            }
            
            void doEndLineDrag(const InputState& inputState) {}
            
            void doCancelLineDrag() {
                m_currentPolyhedron = m_initialPolyhedron;
            }
        };
        
        CreateComplexBrushToolAdapter3D::CreateComplexBrushToolAdapter3D(CreateComplexBrushTool* tool, MapDocumentWPtr document) :
        m_tool(tool),
        m_document(document) {
            assert(m_tool != NULL);
        }

        void CreateComplexBrushToolAdapter3D::performCreateBrush() {
            m_tool->createBrush();
            m_polyhedron = Polyhedron3();
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
            
            m_polyhedron.addPoint(snapped);
            m_tool->update(m_polyhedron);
            
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
                m_polyhedron.addPoint((*it)->position());
            m_tool->update(m_polyhedron);
            
            return true;
        }

        MouseDragPolicy* CreateComplexBrushToolAdapter3D::doCreateDelegate(const InputState& inputState) {
            if (!inputState.mouseButtonsDown(MouseButtons::MBLeft))
                return NULL;
            if (!inputState.checkModifierKeys(MK_No, MK_No, MK_DontCare))
                return NULL;

            const Grid& grid = lock(m_document)->grid();
            if (inputState.modifierKeysDown(ModifierKeys::MKShift))
                return new DuplicateFaceDelegate(grid, m_polyhedron);
            return new DrawFaceDelegate(grid, m_polyhedron);
        }
        
        void CreateComplexBrushToolAdapter3D::doDeleteDelegate(MouseDragPolicy* delegate) {
            delete delegate;
        }

        void CreateComplexBrushToolAdapter3D::doMouseDragStarted() {
            m_tool->update(m_polyhedron);
        }
        
        void CreateComplexBrushToolAdapter3D::doMouseDragged() {
            m_tool->update(m_polyhedron);
        }
        
        void CreateComplexBrushToolAdapter3D::doMouseDragCancelled() {
            m_tool->update(m_polyhedron);
        }

        void CreateComplexBrushToolAdapter3D::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_tool->render(renderContext, renderBatch);
            
            if (!m_polyhedron.empty()) {
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(pref(Preferences::HandleColor));
                renderService.setLineWidth(2.0f);
                
                const Polyhedron3::EdgeList& edges = m_polyhedron.edges();
                Polyhedron3::EdgeList::const_iterator eIt, eEnd;
                for (eIt = edges.begin(), eEnd = edges.end(); eIt != eEnd; ++eIt) {
                    const Polyhedron3::Edge* edge = *eIt;
                    renderService.renderLine(edge->firstVertex()->position(), edge->secondVertex()->position());
                }
                
                const Polyhedron3::VertexList& vertices = m_polyhedron.vertices();
                Polyhedron3::VertexList::const_iterator vIt, vEnd;
                for (vIt = vertices.begin(), vEnd = vertices.end(); vIt != vEnd; ++vIt) {
                    const Polyhedron3::Vertex* vertex = *vIt;
                    renderService.renderPointHandle(vertex->position());
                }
                
                if (m_polyhedron.polygon() && inputState.modifierKeysDown(ModifierKeys::MKShift)) {
                    const Polyhedron3::Face* face = m_polyhedron.faces().front();
                    const Vec3::List pos3 = face->vertexPositions();
                    Vec3f::List pos3f(pos3.size());
                    for (size_t i = 0; i < pos3.size(); ++i)
                        pos3f[i] = Vec3f(pos3[i]);
                    
                    renderService.setForegroundColor(Color(pref(Preferences::HandleColor), 0.5f));
                    renderService.renderFilledPolygon(pos3f);

                    std::reverse(pos3f.begin(), pos3f.end());
                    renderService.renderFilledPolygon(pos3f);
                }
            }
        }

        bool CreateComplexBrushToolAdapter3D::doCancel() {
            if (m_polyhedron.empty())
                return false;
            
            m_polyhedron = Polyhedron3();
            m_tool->update(m_polyhedron);
            return true;
        }
    }
}
