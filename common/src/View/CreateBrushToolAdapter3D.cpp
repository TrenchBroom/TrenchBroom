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

#include "CreateBrushToolAdapter3D.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushVertex.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/PickResult.h"
#include "Renderer/RenderService.h"
#include "View/CreateBrushTool.h"
#include "View/Grid.h"
#include "View/InputState.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        bool CreateBrushToolAdapter3D::PointHistory::empty() const {
            return m_points.empty();
        }

        const Vec3::List& CreateBrushToolAdapter3D::PointHistory::points() const {
            return m_points;
        }
        
        void CreateBrushToolAdapter3D::PointHistory::push(const Vec3& point) {
            m_indices.push_back(m_points.size());
            m_points.push_back(point);
        }
        
        void CreateBrushToolAdapter3D::PointHistory::push(const Vec3::List& points) {
            if (!points.empty()) {
                m_indices.push_back(m_points.size());
                VectorUtils::append(m_points, points);
            }
        }
        
        void CreateBrushToolAdapter3D::PointHistory::pop() {
            if (!m_points.empty()) {
                assert(!m_indices.empty());
                
                const size_t index = m_indices.back(); m_indices.pop_back();
                Vec3::List::iterator it = m_points.begin();
                std::advance(it, index);
                m_points.erase(it);
            }
        }

        void CreateBrushToolAdapter3D::PointHistory::clear() {
            m_points.clear();
            m_indices.clear();
        }
        
        class CreateBrushToolAdapter3D::DragHelper : public PlaneDragHelper {
        public:
            virtual ~DragHelper() {}

            const Vec3::List& points() const { return doGetPoints(); }
        private:
            virtual const Vec3::List& doGetPoints() const = 0;
        };
        
        class CreateBrushToolAdapter3D::CreatePolygonDragHelper : public DragHelper {
        private:
            CreateBrushTool* m_tool;
            const Grid& m_grid;
            Vec3 m_initialPoint;
            Plane3 m_plane;
            Vec3::List m_points;
        public:
            CreatePolygonDragHelper(CreateBrushTool* tool, const Grid& grid) :
            m_tool(tool),
            m_grid(grid) {
                assert(m_tool != NULL);
            }
        public:
            bool startPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
                const Model::PickResult& pickResult = inputState.pickResult();
                const Model::Hit& hit = pickResult.query().pickable().type(Model::Brush::BrushHit).occluded().first();
                if (!hit.isMatch())
                    return false;
                
                const Model::BrushFace* face = Model::hitToFace(hit);
                m_initialPoint = initialPoint = hit.hitPoint();
                m_plane = plane = face->boundary();
                
                update(m_initialPoint);
                return true;
            }
            
            bool planeDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
                update(curPoint);
                refPoint = curPoint;
                return true;
            }
            
            void endPlaneDrag(const InputState& inputState) {
                m_points.clear();
            }
            
            void cancelPlaneDrag() {
                m_points.clear();
            }
            
            void resetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            }
            
            void render(const InputState& inputState, const bool dragging, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
                if (dragging) {
                    Renderer::RenderService renderService(renderContext, renderBatch);
                    renderService.setForegroundColor(pref(Preferences::HandleColor));
                    renderService.setLineWidth(1.0f);
                    
                    for (size_t i = 0; i < m_points.size(); ++i) {
                        const Vec3& point = m_points[i];
                        const Vec3& next = m_points[Math::succ(i, m_points.size())];
                        renderService.renderLine(point, next);
                    }
                }
            }
        private:
            void update(const Vec3& current) {
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
                
                Vec3::List points;
                points.push_back(topLeft3);
                points.push_back(bottomLeft3);
                points.push_back(bottomRight3);
                points.push_back(topRight3);
                
                using std::swap;
                swap(m_points, points);
            }
        private:
            const Vec3::List& doGetPoints() const {
                return m_points;
            }
        };
        
        CreateBrushToolAdapter3D::CreateBrushToolAdapter3D(CreateBrushTool* tool, const Grid& grid) :
        m_tool(tool),
        m_grid(grid),
        m_createPolygonDragHelper(new CreatePolygonDragHelper(m_tool, m_grid)),
        m_dragHelper(NULL) {
            assert(tool != NULL);
        }

        CreateBrushToolAdapter3D::~CreateBrushToolAdapter3D() {
            delete m_createPolygonDragHelper;
        }

        Tool* CreateBrushToolAdapter3D::doGetTool() {
            return m_tool;
        }
        
        void CreateBrushToolAdapter3D::doModifierKeyChange(const InputState& inputState) {
        }
        
        bool CreateBrushToolAdapter3D::doMouseClick(const InputState& inputState) {
            if (!inputState.mouseButtonsDown(MouseButtons::MBLeft))
                return false;
            
            const Model::PickResult& pickResult = inputState.pickResult();
            const Model::Hit& hit = pickResult.query().pickable().type(Model::Brush::BrushHit).occluded().first();
            if (!hit.isMatch())
                return false;
            
            if (inputState.modifierKeysPressed(ModifierKeys::MKShift)) {
                const Model::BrushFace* face = Model::hitToFace(hit);
                
                Vec3::List positions;
                const Model::BrushVertexList& vertices = face->vertices();
                for (size_t i = 0; i < vertices.size(); ++i)
                    positions.push_back(vertices[i]->position);
                m_history.push(positions);
            } else {
                const Model::BrushFace* face = Model::hitToFace(hit);
                const Vec3 snapped = m_grid.snap(hit.hitPoint(), face->boundary());
                m_history.push(snapped);
            }
            
            updateTool();
            return true;
        }
        
        bool CreateBrushToolAdapter3D::doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            if (!inputState.mouseButtonsDown(MouseButtons::MBLeft))
                return false;
            if (!inputState.modifierKeysPressed(ModifierKeys::MKNone))
                return false;
            
            assert(m_dragHelper == NULL);
            m_dragHelper = m_createPolygonDragHelper;
            
            return m_dragHelper->startPlaneDrag(inputState, plane, initialPoint);
        }
        
        bool CreateBrushToolAdapter3D::doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
            assert(m_dragHelper != NULL);
            return m_dragHelper->planeDrag(inputState, lastPoint, curPoint, refPoint);
        }
        
        void CreateBrushToolAdapter3D::doEndPlaneDrag(const InputState& inputState) {
            assert(m_dragHelper != NULL);
            m_history.push(m_dragHelper->points());
            m_dragHelper->endPlaneDrag(inputState);
            m_dragHelper = NULL;

            updateTool();
        }
        
        void CreateBrushToolAdapter3D::doCancelPlaneDrag() {
            assert(m_dragHelper != NULL);
            m_dragHelper->cancelPlaneDrag();
            m_dragHelper = NULL;

            updateTool();
        }
        
        void CreateBrushToolAdapter3D::doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            assert(m_dragHelper != NULL);
            m_dragHelper->resetPlane(inputState, plane, initialPoint);
        }

        void CreateBrushToolAdapter3D::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {
        }
        
        void CreateBrushToolAdapter3D::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (m_dragHelper != NULL)
                m_dragHelper->render(inputState, dragging(), renderContext, renderBatch);
            m_tool->render(renderContext, renderBatch);
        }

        bool CreateBrushToolAdapter3D::doCancel() {
            if (!m_history.empty()) {
                m_history.clear();
                m_tool->clear();
                return true;
            }
            return false;
        }

        void CreateBrushToolAdapter3D::updateTool() {
            m_tool->update(m_history.points());
        }
    }
}
