/*
 Copyright (C) 2010-2016 Kristian Duske

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
            CreateComplexBrushTool* m_tool;
            Polyhedron3 m_oldPolyhedron;
        protected:
            Part(CreateComplexBrushTool* tool) :
            m_tool(tool),
            m_oldPolyhedron() {
                ensure(m_tool != NULL, "tool is null");
            }
        public:
            virtual ~Part() {}
        };
        
        class CreateComplexBrushToolController3D::DrawFacePart : public Part, public ToolControllerBase<NoPickingPolicy, NoKeyPolicy, NoMousePolicy, RestrictedDragPolicy, NoRenderPolicy, NoDropPolicy> {
        private:
            Plane3 m_plane;
            Vec3 m_initialPoint;
        public:
            DrawFacePart(CreateComplexBrushTool* tool) :
            Part(tool) {}
        private:
            Tool* doGetTool() { return m_tool; }
            
            DragInfo doStartDrag(const InputState& inputState) {
                if (inputState.modifierKeysDown(ModifierKeys::MKShift))
                    return DragInfo();
                
                const Model::PickResult& pickResult = inputState.pickResult();
                const Model::Hit& hit = pickResult.query().pickable().type(Model::Brush::BrushHit).occluded().first();
                if (!hit.isMatch())
                    return DragInfo();

                m_oldPolyhedron = m_tool->polyhedron();
                
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
            
            DragResult doDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint) {
                updatePolyhedron(curPoint);
                return DR_Continue;
            }
            
            void doEndDrag(const InputState& inputState) {
            }
            
            void doCancelDrag() {
                m_tool->update(m_oldPolyhedron);
            }
            
            bool doCancel() { return false; }
        private:
            void updatePolyhedron(const Vec3& current) {
                const Grid& grid = m_tool->grid();
                
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
                
                Polyhedron3 polyhedron = m_oldPolyhedron;
                polyhedron.addPoint(topLeft3);
                polyhedron.addPoint(bottomLeft3);
                polyhedron.addPoint(bottomRight3);
                polyhedron.addPoint(topRight3);
                m_tool->update(polyhedron);
            }
        };
        
        class CreateComplexBrushToolController3D::DuplicateFacePart : public Part, public ToolControllerBase<NoPickingPolicy, NoKeyPolicy, NoMousePolicy, RestrictedDragPolicy, NoRenderPolicy, NoDropPolicy> {
        private:
            Vec3 m_dragDir;
        public:
            DuplicateFacePart(CreateComplexBrushTool* tool) :
            Part(tool) {}
        private:
            Tool* doGetTool() { return m_tool; }

            DragInfo doStartDrag(const InputState& inputState) {
                if (!inputState.modifierKeysDown(ModifierKeys::MKShift))
                    return DragInfo();
                
                if (!m_tool->polyhedron().polygon())
                    return DragInfo();
                
                m_oldPolyhedron = m_tool->polyhedron();

                const Polyhedron3::FaceHit hit = m_oldPolyhedron.pickFace(inputState.pickRay());
                const Vec3 origin    = inputState.pickRay().pointAtDistance(hit.distance);
                const Vec3 direction = hit.face->normal();
                
                const Line3 line(origin, direction);
                m_dragDir = line.direction;
                
                return DragInfo(new LineDragRestricter(line), new NoDragSnapper(), origin);
            }
            
            DragResult doDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint) {
                Polyhedron3 polyhedron = m_oldPolyhedron;
                assert(polyhedron.polygon());
                
                const Grid& grid = m_tool->grid();
                
                const Vec3 rayDelta             = curPoint - dragOrigin();
                const Vec3 rayAxis              = m_dragDir.firstAxis();
                const FloatType axisDistance    = rayDelta.dot(rayAxis);
                const FloatType snappedDistance = grid.snap(axisDistance);
                const FloatType snappedRayDist  = rayAxis.inverseDot(snappedDistance, m_dragDir);
                const Vec3 snappedRayDelta      = snappedRayDist * m_dragDir;
                
                const Polyhedron3::Face* face = m_oldPolyhedron.faces().front();
                const Vec3::Array points = face->vertexPositions() + snappedRayDelta;
                
                polyhedron.addPoints(points);
                m_tool->update(polyhedron);
                
                return DR_Continue;
            }
            
            void doEndDrag(const InputState& inputState) {
            }
            
            void doCancelDrag() {
                m_tool->update(m_oldPolyhedron);
            }
            
            bool doCancel() { return false; }
        };
        
        CreateComplexBrushToolController3D::CreateComplexBrushToolController3D(CreateComplexBrushTool* tool) :
        m_tool(tool) {
            ensure(m_tool != NULL, "tool is null");
            addController(new DrawFacePart(m_tool));
            addController(new DuplicateFacePart(m_tool));
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
            
            const Grid& grid = m_tool->grid();
            
            const Model::BrushFace* face = Model::hitToFace(hit);
            const Vec3 snapped = grid.snap(hit.hitPoint(), face->boundary());
            
            Polyhedron3 polyhedron = m_tool->polyhedron();
            polyhedron.addPoint(snapped);
            m_tool->update(polyhedron);
            
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
            
            Polyhedron3 polyhedron = m_tool->polyhedron();
            const Model::BrushFace* face = Model::hitToFace(hit);
            
            for (const Model::BrushVertex* vertex : face->vertices())
                polyhedron.addPoint(vertex->position());
            m_tool->update(polyhedron);
            
            return true;
        }

        bool CreateComplexBrushToolController3D::doShouldHandleMouseDrag(const InputState& inputState) const {
            if (!inputState.mouseButtonsDown(MouseButtons::MBLeft))
                return false;
            if (!inputState.checkModifierKeys(MK_No, MK_No, MK_DontCare))
                return false;
            return true;
        }

        void CreateComplexBrushToolController3D::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_tool->render(renderContext, renderBatch);
            
            const Polyhedron3& polyhedron = m_tool->polyhedron();
            if (!polyhedron.empty()) {
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(pref(Preferences::HandleColor));
                renderService.setLineWidth(2.0f);
                
                for (const Polyhedron3::Edge* edge : polyhedron.edges())
                    renderService.renderLine(edge->firstVertex()->position(), edge->secondVertex()->position());
                
                for (const Polyhedron3::Vertex* vertex : polyhedron.vertices())
                    renderService.renderPointHandle(vertex->position());
                
                if (polyhedron.polygon() && inputState.modifierKeysDown(ModifierKeys::MKShift)) {
                    const Polyhedron3::Face* face = polyhedron.faces().front();
                    const Vec3::Array pos3 = face->vertexPositions();
                    Vec3f::Array pos3f(pos3.size());
                    for (size_t i = 0; i < pos3.size(); ++i)
                        pos3f[i] = Vec3f(pos3[i]);
                    
                    renderService.setForegroundColor(Color(pref(Preferences::HandleColor), 0.5f));
                    renderService.renderFilledPolygon(pos3f);

                    std::reverse(std::begin(pos3f), std::end(pos3f));
                    renderService.renderFilledPolygon(pos3f);
                }
            }
        }

        bool CreateComplexBrushToolController3D::doCancel() {
            const Polyhedron3& polyhedron = m_tool->polyhedron();
            if (polyhedron.empty())
                return false;
            
            m_tool->update(Polyhedron3());
            return true;
        }
    }
}
