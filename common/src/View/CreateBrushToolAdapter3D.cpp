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
#include "Renderer/Camera.h"
#include "Renderer/RenderService.h"
#include "View/CreateBrushTool.h"
#include "View/Grid.h"
#include "View/InputState.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        class CreateBrushToolAdapter3D::CreateBrushToolHelper : public PlaneDragHelper {
        protected:
            CreateBrushTool* m_tool;
            const Grid& m_grid;
        public:
            CreateBrushToolHelper(PlaneDragPolicy* policy, CreateBrushTool* tool, const Grid& grid) :
            PlaneDragHelper(policy),
            m_tool(tool),
            m_grid(grid) {
                assert(m_tool != NULL);
            }
            
            virtual ~CreateBrushToolHelper() {}

            void performCreateBrush() {
                doPerformCreateBrush();
            }
            
            void modifierKeyChange(const InputState& inputState) {
                doModifierKeyChange(inputState);
            }

            bool mouseClick(const InputState& inputState) {
                return doMouseClick(inputState);
            }

            bool mouseDoubleClick(const InputState& inputState) {
                return doMouseDoubleClick(inputState);
            }

            virtual void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
                m_tool->render(renderContext, renderBatch);
            }

            bool cancel() {
                return doCancel();
            }
        private:
            virtual void doPerformCreateBrush() = 0;
            virtual void doModifierKeyChange(const InputState& inputState) = 0;
            virtual bool doMouseClick(const InputState& inputState) = 0;
            virtual bool doMouseDoubleClick(const InputState& inputState) = 0;
            virtual bool doCancel() = 0;
        };
        
        class CreateBrushToolAdapter3D::CreateBrushHelper : public CreateBrushToolAdapter3D::CreateBrushToolHelper {
        private:
            Vec3 m_initialPoint;
        public:
            CreateBrushHelper(PlaneDragPolicy* policy, CreateBrushTool* tool, const Grid& grid) :
            CreateBrushToolHelper(policy, tool, grid) {}
        private:
            void doPerformCreateBrush() {} // happens automatically when drag ends

            void doModifierKeyChange(const InputState& inputState) {
                if (dragging())
                    resetPlane(inputState);
            }
            
            bool doMouseClick(const InputState& inputState) {
                return false;
            }

            bool doMouseDoubleClick(const InputState& inputState) {
                return false;
            }

            bool doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
                if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft))
                    return false;
                if (!inputState.modifierKeysPressed(ModifierKeys::MKNone))
                    return false;
                
                const Model::PickResult& pickResult = inputState.pickResult();
                const Model::Hit& hit = pickResult.query().pickable().type(Model::Brush::BrushHit).occluded().first();
                if (hit.isMatch())
                    m_initialPoint = initialPoint = initialPoint = hit.hitPoint();
                else
                    m_initialPoint = initialPoint = inputState.defaultPointUnderMouse();
                
                plane = Plane3(initialPoint, Vec3::PosZ);
                
                updateBounds(m_initialPoint);
                return true;
            }
            
            bool doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
                updateBounds(curPoint);
                return true;
            }
            
            void doEndPlaneDrag(const InputState& inputState) {
                m_tool->createBrush();
            }
            
            void doCancelPlaneDrag() {
                m_tool->cancel();
            }
            
            void doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
                const FloatType distance = plane.intersectWithRay(inputState.pickRay());
                if (Math::isnan(distance))
                    return;
                initialPoint = inputState.pickRay().pointAtDistance(distance);
                
                if (inputState.modifierKeys() == ModifierKeys::MKAlt) {
                    Vec3 planeNorm = inputState.pickRay().direction;
                    planeNorm[2] = 0.0;
                    planeNorm.normalize();
                    plane = Plane3(initialPoint, planeNorm);
                } else {
                    plane = horizontalDragPlane(initialPoint);
                }
            }
            
            bool doCancel() {
                return false;
            }
        private:
            void updateBounds(const Vec3& point) {
                BBox3 bounds;
                for (size_t i = 0; i < 3; i++) {
                    bounds.min[i] = std::min(m_initialPoint[i], point[i]);
                    bounds.max[i] = std::max(m_initialPoint[i], point[i]);
                }
                
                bounds.min = m_grid.snapDown(bounds.min);
                bounds.max = m_grid.snapUp(bounds.max);
                
                for (size_t i = 0; i < 3; i++)
                    if (bounds.max[i] <= bounds.min[i])
                        bounds.max[i] = bounds.min[i] + m_grid.actualSize();
                
                m_tool->updateBrush(bounds);
            }
        };
        
        class CreateBrushToolAdapter3D::CreatePolyhedronHelper : public CreateBrushToolAdapter3D::CreateBrushToolHelper {
        private:
            Vec3 m_initialPoint;
            Plane3 m_plane;
            
            Polyhedron3 m_polyhedron;
            
            // for dragging quads
            Polyhedron3 m_oldPolyhedron;
        public:
            CreatePolyhedronHelper(PlaneDragPolicy* policy, CreateBrushTool* tool, const Grid& grid) :
            CreateBrushToolHelper(policy, tool, grid) {}
        private:
            void doPerformCreateBrush() {
                m_tool->createBrush();
                m_polyhedron = Polyhedron3();
            }

            void doModifierKeyChange(const InputState& inputState) {}
            
            bool doMouseClick(const InputState& inputState) {
                if (!inputState.mouseButtonsDown(MouseButtons::MBLeft))
                    return false;
                if (!inputState.checkModifierKeys(MK_No, MK_No, MK_DontCare))
                    return false;
                
                const Model::PickResult& pickResult = inputState.pickResult();
                const Model::Hit& hit = pickResult.query().pickable().type(Model::Brush::BrushHit).occluded().first();
                if (!hit.isMatch())
                    return false;
                
                const Model::BrushFace* face = Model::hitToFace(hit);
                const Vec3 snapped = m_grid.snap(hit.hitPoint(), face->boundary());
                
                m_polyhedron.addPoint(snapped);
                m_tool->updateBrush(m_polyhedron);
                
                return true;
            }
            
            bool doMouseDoubleClick(const InputState& inputState) {
                if (!inputState.mouseButtonsDown(MouseButtons::MBLeft))
                    return false;
                if (!inputState.checkModifierKeys(MK_No, MK_No, MK_DontCare))
                    return false;
                
                const Model::PickResult& pickResult = inputState.pickResult();
                const Model::Hit& hit = pickResult.query().pickable().type(Model::Brush::BrushHit).occluded().first();
                if (!hit.isMatch())
                    return false;

                const Model::BrushFace* face = Model::hitToFace(hit);
                
                const Model::BrushFace::VertexList vertices = face->vertices();
                Model::BrushFace::VertexList::const_iterator it, end;
                for (it = vertices.begin(), end = vertices.end(); it != end; ++it)
                    m_polyhedron.addPoint((*it)->position);
                m_tool->updateBrush(m_polyhedron);
                
                return true;
            }
            
            bool doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
                if (!inputState.mouseButtonsDown(MouseButtons::MBLeft))
                    return false;
                if (!inputState.checkModifierKeys(MK_No, MK_No, MK_DontCare))
                    return false;
                
                const Model::PickResult& pickResult = inputState.pickResult();
                const Model::Hit& hit = pickResult.query().pickable().type(Model::Brush::BrushHit).occluded().first();
                if (!hit.isMatch())
                    return false;
                
                const Model::BrushFace* face = Model::hitToFace(hit);
                m_initialPoint = initialPoint = hit.hitPoint();
                m_plane = plane = face->boundary();
                m_oldPolyhedron = m_polyhedron;
                
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
                m_polyhedron = m_oldPolyhedron;
                m_tool->updateBrush(m_polyhedron);
            }
            
            void doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {}
            
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
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
                }
            }
            
            bool doCancel() {
                if (m_polyhedron.empty())
                    return false;
                
                m_polyhedron = Polyhedron3();
                m_tool->updateBrush(m_polyhedron);
                return true;
            }
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

                m_polyhedron = m_oldPolyhedron;
                m_polyhedron.addPoint(topLeft3);
                m_polyhedron.addPoint(bottomLeft3);
                m_polyhedron.addPoint(bottomRight3);
                m_polyhedron.addPoint(topRight3);

                m_tool->updateBrush(m_polyhedron);
            }
        };
        
        CreateBrushToolAdapter3D::CreateBrushToolAdapter3D(CreateBrushTool* tool, MapDocumentWPtr document) :
        m_tool(tool),
        m_document(document),
        m_createBrushHelper(new CreateBrushHelper(this, m_tool, lock(m_document)->grid())),
        m_createPolyhedronHelper(new CreatePolyhedronHelper(this, m_tool, lock(m_document)->grid())),
        m_currentHelper(m_createBrushHelper) {
            assert(tool != NULL);
        }

        CreateBrushToolAdapter3D::~CreateBrushToolAdapter3D() {
            delete m_createBrushHelper;
            delete m_createPolyhedronHelper;
        }

        void CreateBrushToolAdapter3D::performCreateBrush() {
            m_currentHelper->performCreateBrush();
            setCreateBrushMode();
        }

        Tool* CreateBrushToolAdapter3D::doGetTool() {
            return m_tool;
        }
        
        void CreateBrushToolAdapter3D::doModifierKeyChange(const InputState& inputState) {
            m_currentHelper->modifierKeyChange(inputState);
        }

        bool CreateBrushToolAdapter3D::doMouseClick(const InputState& inputState) {
            if (inputState.modifierKeysPressed(ModifierKeys::MKShift))
                setCreatePolyhedronMode();
            return m_currentHelper->mouseClick(inputState);
        }

        bool CreateBrushToolAdapter3D::doMouseDoubleClick(const InputState& inputState) {
            if (inputState.modifierKeysPressed(ModifierKeys::MKShift))
                setCreatePolyhedronMode();
            return m_currentHelper->mouseDoubleClick(inputState);
        }

        bool CreateBrushToolAdapter3D::doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            if (inputState.modifierKeysPressed(ModifierKeys::MKShift))
                setCreatePolyhedronMode();
            return m_currentHelper->startPlaneDrag(inputState, plane, initialPoint);
        }
        
        bool CreateBrushToolAdapter3D::doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
            return m_currentHelper->planeDrag(inputState, lastPoint, curPoint, refPoint);
        }
        
        void CreateBrushToolAdapter3D::doEndPlaneDrag(const InputState& inputState) {
            m_currentHelper->endPlaneDrag(inputState);
        }
        
        void CreateBrushToolAdapter3D::doCancelPlaneDrag() {
            m_currentHelper->cancelPlaneDrag();
        }
        
        void CreateBrushToolAdapter3D::doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            m_currentHelper->resetPlane(inputState, plane, initialPoint);
        }

        void CreateBrushToolAdapter3D::doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const {}
        
        void CreateBrushToolAdapter3D::doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            m_currentHelper->render(inputState, renderContext, renderBatch);
        }

        bool CreateBrushToolAdapter3D::doCancel() {
            if (m_currentHelper->cancel())
                return true;
            if (m_currentHelper != m_createBrushHelper) {
                setCreateBrushMode();
                return true;
            }
            return false;
        }

        void CreateBrushToolAdapter3D::setCreateBrushMode() {
            setCurrentHelper(m_createBrushHelper);
        }
        
        void CreateBrushToolAdapter3D::setCreatePolyhedronMode() {
            setCurrentHelper(m_createPolyhedronHelper);
        }
        
        void CreateBrushToolAdapter3D::setCurrentHelper(CreateBrushToolHelper* helper) {
            if (helper != m_currentHelper)
                m_currentHelper->cancel();
            m_currentHelper = helper;
        }

        void CreateBrushToolAdapter3D::resetMode() {
            setCreateBrushMode();
        }
    }
}
