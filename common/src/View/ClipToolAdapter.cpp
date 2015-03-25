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

#include "ClipToolAdapter.h"

#include "Model/Brush.h"
#include "Model/BrushEdge.h"
#include "Model/BrushFace.h"
#include "Model/BrushVertex.h"
#include "Model/Hit.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/PickResult.h"
#include "Renderer/Camera.h"
#include "View/Grid.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        ClipToolAdapter2D::ClipToolAdapter2D(ClipTool* tool, const Grid& grid) :
        ClipToolAdapter(tool, grid) {}
        
        class ClipToolAdapter2D::PointSnapper : public ClipTool::PointSnapper {
        private:
            const Grid& m_grid;
        public:
            PointSnapper(const Grid& grid) : m_grid(grid) {}
        private:
            bool doSnap(const Vec3& point, Vec3& snappedPoint) const {
                snappedPoint = m_grid.snap(point);
                return true;
            }
        };

        class ClipToolAdapter2D::PointStrategy : public ClipTool::PointStrategy {
        private:
            const Vec3 m_viewDirection;
        public:
            PointStrategy(const Vec3& viewDirection) : m_viewDirection(viewDirection) {}
        private:
            bool doComputeThirdPoint(const Vec3& point1, const Vec3& point2, Vec3& point3) const {
                point3 = point2 + 128.0 * m_viewDirection;
                return !linearlyDependent(point1, point2, point3);
            }
        };

        class ClipToolAdapter2D::PointStrategyFactory : public ClipTool::PointStrategyFactory {
        private:
            const Vec3 m_viewDirection;
        public:
            PointStrategyFactory(const Vec3& viewDirection) : m_viewDirection(viewDirection) {}
        private:
            ClipTool::PointStrategy* doCreateStrategy() const {
                return new PointStrategy(m_viewDirection);
            }
        };

        bool ClipToolAdapter2D::doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            if (!startDrag(inputState))
                return false;
            
            if (!m_tool->beginDragPoint(inputState.pickResult(), initialPoint))
                return false;
            
            plane = Plane3(initialPoint, inputState.camera().direction().firstAxis());
            return true;
        }
        
        bool ClipToolAdapter2D::doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
            
            const PointSnapper snapper(m_grid);
            Vec3 snapped;
            if (m_tool->dragPoint(curPoint, snapper, snapped))
                refPoint = snapped;
            return true;
        }
        
        void ClipToolAdapter2D::doEndPlaneDrag(const InputState& inputState) {}
        void ClipToolAdapter2D::doCancelPlaneDrag() {}
        void ClipToolAdapter2D::doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {}
        
        bool ClipToolAdapter2D::doAddClipPoint(const InputState& inputState) {
            const Renderer::Camera& camera = inputState.camera();
            const Vec3 viewDir = camera.direction().firstAxis();
            
            const Ray3& pickRay = inputState.pickRay();
            const Vec3 defaultPos = m_tool->defaultClipPointPos();
            const FloatType distance = pickRay.intersectWithPlane(viewDir, defaultPos);
            if (Math::isnan(distance))
                return false;
            
            const Vec3 hitPoint = pickRay.pointAtDistance(distance);
            const PointSnapper snapper(m_grid);
            if (!m_tool->canAddPoint(hitPoint, snapper))
                return false;
            
            const PointStrategyFactory factory(viewDir);
            m_tool->addPoint(hitPoint, snapper, factory);
            return true;
        }

        bool ClipToolAdapter2D::doSetClipPlane(const InputState& inputState) {
            return false;
        }

        ClipToolAdapter3D::ClipToolAdapter3D(ClipTool* tool, const Grid& grid) :
        ClipToolAdapter(tool, grid) {}

        class ClipToolAdapter3D::PointSnapper : public ClipTool::PointSnapper {
        private:
            const Grid& m_grid;
            const Model::BrushFace* m_currentFace;
        public:
            PointSnapper(const Grid& grid, const Model::BrushFace* currentFace) :
            m_grid(grid),
            m_currentFace(currentFace) {
                assert(m_currentFace != NULL);
            }
        private:
            bool doSnap(const Vec3& point, Vec3& snappedPoint) const {
                const Vec3 snapAttempt = m_grid.snap(point, m_currentFace->boundary());
                if (!m_currentFace->containsPoint(snapAttempt))
                    return false;
                
                snappedPoint = snapAttempt;
                return true;
            }
        };
        
        bool ClipToolAdapter3D::doStartMouseDrag(const InputState& inputState) {
            if (!startDrag(inputState))
                return false;
            Vec3 initialPosition;
            return m_tool->beginDragPoint(inputState.pickResult(), initialPosition);
        }
        
        bool ClipToolAdapter3D::doMouseDrag(const InputState& inputState) {
            const Model::Hit& hit = inputState.pickResult().query().type(Model::Brush::BrushHit).occluded().first();
            if (hit.isMatch()) {
                const PointSnapper snapper(m_grid, Model::hitToFace(hit));
                Vec3 snapped;
                
                m_tool->dragPoint(hit.hitPoint(), snapper, snapped);
            }
            return true;
        }
        
        void ClipToolAdapter3D::doEndMouseDrag(const InputState& inputState) {}
        void ClipToolAdapter3D::doCancelMouseDrag() {}
        
        bool ClipToolAdapter3D::doAddClipPoint(const InputState& inputState) {
            const Model::Hit& hit = inputState.pickResult().query().pickable().type(Model::Brush::BrushHit).occluded().first();
            if (!hit.isMatch())
                return false;
            
            const Vec3& point = hit.hitPoint();
            const Model::BrushFace* face = hit.target<Model::BrushFace*>();
            
            const PointSnapper snapper(m_grid, face);
            if (!m_tool->canAddPoint(point, snapper))
                return false;
            m_tool->addPoint(point, snapper);
            return true;
        }

        bool ClipToolAdapter3D::doSetClipPlane(const InputState& inputState) {
            const Model::Hit& hit = inputState.pickResult().query().pickable().type(Model::Brush::BrushHit).occluded().first();
            if (!hit.isMatch())
                return false;
            const Model::BrushFace* face = Model::hitToFace(hit);
            m_tool->setFace(face);
            return true;
        }
    }
}
