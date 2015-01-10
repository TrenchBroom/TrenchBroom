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
        
        class ClipToolAdapter2D::ClipPlaneStrategy : public ClipTool::ClipPlaneStrategy {
        public:
            ClipPlaneStrategy() {}
        private:
            Vec3 doSnapClipPoint(const Grid& grid, const Vec3& point) const {
                return grid.snap(point);
            }
        };

        bool ClipToolAdapter2D::doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            if (!startDrag(inputState))
                return false;
            
            initialPoint = m_tool->draggedPointPosition();
            plane = Plane3(initialPoint, inputState.camera().direction().firstAxis());
            return true;
        }
        
        bool ClipToolAdapter2D::doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
            
            const ClipPlaneStrategy strategy;
            if (m_tool->dragClipPoint(curPoint, strategy))
                refPoint = m_tool->draggedPointPosition();
            return true;
        }
        
        void ClipToolAdapter2D::doEndPlaneDrag(const InputState& inputState) {}
        void ClipToolAdapter2D::doCancelPlaneDrag() {}
        void ClipToolAdapter2D::doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {}
        
        bool ClipToolAdapter2D::doAddClipPoint(const InputState& inputState) {
            const Renderer::Camera& camera = inputState.camera();
            const Vec3 viewDir = camera.direction().firstAxis();
            
            Vec3 hitPoint;
            const Model::Hit& hit = inputState.pickResult().query().pickable().type(Model::Brush::BrushHit).occluded().first();
            if (hit.isMatch()) {
                hitPoint = hit.hitPoint();
            } else {
                const Ray3& pickRay = inputState.pickRay();
                const Vec3 defaultPos = m_tool->defaultClipPointPos();
                const FloatType distance = pickRay.intersectWithPlane(viewDir, defaultPos);
                if (Math::isnan(distance))
                    return false;
                
                hitPoint = pickRay.pointAtDistance(distance);
            }
            
            
            const ClipPlaneStrategy strategy;
            return m_tool->addClipPoint(hitPoint, strategy);
        }

        ClipToolAdapter3D::ClipToolAdapter3D(ClipTool* tool, const Grid& grid) :
        ClipToolAdapter(tool, grid) {}

        class ClipToolAdapter3D::ClipPlaneStrategy : public ClipTool::ClipPlaneStrategy {
            const Model::BrushFace* m_currentFace;
        public:
            ClipPlaneStrategy(const Model::BrushFace* currentFace) :
            m_currentFace(currentFace) {
                assert(m_currentFace != NULL);
            }
        private:
            Vec3 doSnapClipPoint(const Grid& grid, const Vec3& point) const {
                return grid.snap(point, m_currentFace->boundary());
            }
        };
        
        bool ClipToolAdapter3D::doStartMouseDrag(const InputState& inputState) {
            return startDrag(inputState);
        }
        
        bool ClipToolAdapter3D::doMouseDrag(const InputState& inputState) {
            const Model::Hit& hit = inputState.pickResult().query().type(Model::Brush::BrushHit).occluded().first();
            if (hit.isMatch()) {
                const ClipPlaneStrategy strategy(Model::hitToFace(hit));
                m_tool->dragClipPoint(hit.hitPoint(), strategy);
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
            
            const ClipPlaneStrategy strategy(face);
            return m_tool->addClipPoint(point, strategy);
        }
    }
}
