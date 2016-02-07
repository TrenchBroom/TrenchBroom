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

#include "CreateComplexBrushToolController3D.h"

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
        class CreateComplexBrushToolController3D::Part {
        protected:
            const Grid& m_grid;
            Polyhedron3& m_currentPolyhedron;
            const Polyhedron3 m_initialPolyhedron;
        protected:
            Part(const Grid& grid, Polyhedron3& polyhedron) :
            m_grid(grid),
            m_currentPolyhedron(polyhedron),
            m_initialPolyhedron(polyhedron) {}
        public:
            virtual ~Part() {}
        };
        
        class CreateComplexBrushToolController3D::DrawFacePart : public Part, public ToolControllerBase<NoPickingPolicy, NoKeyPolicy, NoMousePolicy, RestrictedDragPolicy, NoRenderPolicy, NoDropPolicy> {
        private:
            Plane3 m_plane;
            Vec3 m_initialPoint;
        public:
            DrawFacePart(const Grid& grid, Polyhedron3& polyhedron) :
            Part(grid, polyhedron) {}
        private:
            Tool* doGetTool() { return NULL; }
            
            DragInfo doStartDrag(const InputState& inputState) {
                if (inputState.modifierKeysDown(ModifierKeys::MKShift))
                    return DragInfo();
                
                const Model::PickResult& pickResult = inputState.pickResult();
                const Model::Hit& hit = pickResult.query().pickable().type(Model::Brush::BrushHit).occluded().first();
                if (!hit.isMatch())
                    return DragInfo();

                const Model::BrushFace* face = Model::hitToFace(hit);
                m_plane = face->boundary();
                m_initialPoint = hit.hitPoint();
                updatePolyhedron(m_initialPoint);
                
                SurfaceDragRestricter* restricter = new SurfaceDragRestricter();
                restricter->setPickable(true);
                restricter->setType(Model::Brush::BrushHit);
                restricter->setOccluded(true);
                return DragInfo(restricter, new NoDragSnapper(), m_initialPoint);
            }
            
            bool doDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint) {
                updatePolyhedron(curPoint);
                return true;
            }
            
            void doEndDrag(const InputState& inputState) {}
            
            void doCancelDrag() {
                m_currentPolyhedron = m_initialPolyhedron;
            }
            
            bool doCancel() { return false; }
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
        
        class CreateComplexBrushToolController3D::DuplicateFacePart : public Part, public ToolControllerBase<NoPickingPolicy, NoKeyPolicy, NoMousePolicy, RestrictedDragPolicy, NoRenderPolicy, NoDropPolicy> {
        private:
            Vec3 m_dragDir;
        public:
            DuplicateFacePart(const Grid& grid, Polyhedron3& polyhedron) :
            Part(grid, polyhedron) {}
        private:
            Tool* doGetTool() { return NULL; }

            DragInfo doStartDrag(const InputState& inputState) {
                if (!inputState.modifierKeysDown(ModifierKeys::MKShift))
                    return DragInfo();
                
                if (!m_currentPolyhedron.polygon())
                    return DragInfo();
                
                const Polyhedron3::FaceHit hit = m_currentPolyhedron.pickFace(inputState.pickRay());
                const Vec3 origin    = inputState.pickRay().pointAtDistance(hit.distance);
                const Vec3 direction = hit.face->normal();
                
                const Line3 line(origin, direction);
                m_dragDir = line.direction;
                
                return DragInfo(new LineDragRestricter(line), new NoDragSnapper(), origin);
            }
            
            bool doDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint) {
                assert(m_initialPolyhedron.polygon());
                
                const FloatType curDist         = (curPoint - lastPoint).length();
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
                
                return true;
            }
            
            void doEndDrag(const InputState& inputState) {}
            
            void doCancelDrag() {
                m_currentPolyhedron = m_initialPolyhedron;
            }
            
            bool doCancel() { return false; }
        };
        
        CreateComplexBrushToolController3D::CreateComplexBrushToolController3D(CreateComplexBrushTool* tool, MapDocumentWPtr document) :
        m_tool(tool),
        m_document(document) {
            assert(m_tool != NULL);
            const Grid& grid = lock(m_document)->grid();
            addController(new DrawFacePart(grid, m_polyhedron));
            addController(new DuplicateFacePart(grid, m_polyhedron));
        }

        void CreateComplexBrushToolController3D::performCreateBrush() {
            m_tool->createBrush();
            m_polyhedron = Polyhedron3();
        }

        Tool* CreateComplexBrushToolController3D::doGetTool() {
            return m_tool;
        }

        bool CreateComplexBrushToolController3D::doMouseClick(const InputState& inputState) {
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

        bool CreateComplexBrushToolController3D::doMouseDoubleClick(const InputState& inputState) {
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

        bool CreateComplexBrushToolController3D::doShouldHandleMouseDrag(const InputState& inputState) const {
            if (!inputState.mouseButtonsDown(MouseButtons::MBLeft))
                return false;
            if (!inputState.checkModifierKeys(MK_No, MK_No, MK_DontCare))
                return false;
            return true;
        }

        void CreateComplexBrushToolController3D::doMouseDragStarted(const InputState& inputState) {
            m_tool->update(m_polyhedron);
        }
        
        void CreateComplexBrushToolController3D::doMouseDragged(const InputState& inputState) {
            m_tool->update(m_polyhedron);
        }
        
        void CreateComplexBrushToolController3D::doMouseDragEnded(const InputState& inputState) {
            m_tool->update(m_polyhedron);
        }
        
        void CreateComplexBrushToolController3D::doMouseDragCancelled() {
            m_tool->update(m_polyhedron);
        }

        void CreateComplexBrushToolController3D::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
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

        bool CreateComplexBrushToolController3D::doCancel() {
            if (m_polyhedron.empty())
                return false;
            
            m_polyhedron = Polyhedron3();
            m_tool->update(m_polyhedron);
            return true;
        }
    }
}
