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
#include "Model/HitQuery.h"
#include "Model/PickResult.h"
#include "Renderer/Camera.h"
#include "View/Grid.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        ClipToolAdapter2D::ClipToolAdapter2D(ClipTool* tool, const Grid& grid) :
        ClipToolAdapter(tool, grid) {}

        bool ClipToolAdapter2D::doStartPlaneDrag(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {
            if (!canStartDrag(inputState))
                return false;
            return false;
        }
        
        bool ClipToolAdapter2D::doPlaneDrag(const InputState& inputState, const Vec3& lastPoint, const Vec3& curPoint, Vec3& refPoint) {
            return true;
        }
        
        void ClipToolAdapter2D::doEndPlaneDrag(const InputState& inputState) {}
        void ClipToolAdapter2D::doCancelPlaneDrag() {}
        void ClipToolAdapter2D::doResetPlane(const InputState& inputState, Plane3& plane, Vec3& initialPoint) {}

        class ClipToolAdapter2D::ClipPlaneStrategy : public ClipTool::ClipPlaneStrategy {
        private:
            Vec3 m_viewDir;
        public:
            ClipPlaneStrategy(const Vec3& viewDir) : m_viewDir(viewDir) {}
        private:
            Vec3 doSnapClipPoint(const Grid& grid, const Vec3& point) const {
                return grid.snap(point);
            }
            
            void doComputeThirdClipPoint(const Vec3& point1, const Vec3& point2, Vec3& point3) const {
                assert(point1 != point2);
                point3 = point1 + 128.0 * m_viewDir.firstAxis();
            }
        };
        
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
            
            
            const ClipPlaneStrategy strategy(viewDir);
            return m_tool->addClipPoint(hitPoint, strategy);
        }

        ClipToolAdapter3D::ClipToolAdapter3D(ClipTool* tool, const Grid& grid) :
        ClipToolAdapter(tool, grid) {}

        bool ClipToolAdapter3D::doStartMouseDrag(const InputState& inputState) {
            if (!canStartDrag(inputState))
                return false;
            return false;
        }
        
        bool ClipToolAdapter3D::doMouseDrag(const InputState& inputState) {
            return true;
        }
        
        void ClipToolAdapter3D::doEndMouseDrag(const InputState& inputState) {}
        void ClipToolAdapter3D::doCancelMouseDrag() {}
        
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
            
            void doComputeThirdClipPoint(const Vec3& point1, const Vec3& point2, Vec3& point3) const {
                assert(point1 != point2);
                const Vec3::List normals = getNormals(point2, m_currentFace);
                const Vec3 normal = Vec3::average(normals);
                point3 = point2 + 128.0 * normal.firstAxis();
            }

            Vec3::List getNormals(const Vec3& point, const Model::BrushFace* face) const {
                const Model::Brush* brush = face->brush();
                const Model::BrushEdgeList edges = brush->edges();
                Model::BrushEdgeList::const_iterator it, end;
                for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                    const Model::BrushEdge& edge = **it;
                    if (point.equals(edge.start->position)) {
                        return getNormals(brush->incidentFaces(edge.start));
                    } else if (point.equals(edge.end->position)) {
                        return getNormals(brush->incidentFaces(edge.end));
                    } else if (edge.contains(point)) {
                        Vec3::List normals(2);
                        normals[0] = edge.leftFace()->boundary().normal;
                        normals[1] = edge.rightFace()->boundary().normal;
                        return normals;
                    }
                }
                
                return Vec3::List(1, face->boundary().normal);
            }
            
            Vec3::List getNormals(const Model::BrushFaceList& faces) const {
                Vec3::List normals;
                normals.reserve(faces.size());
                
                Model::BrushFaceList::const_iterator it, end;
                for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                    const Model::BrushFace& face = **it;
                    normals.push_back(face.boundary().normal);
                }
                
                return normals;
            }
        };

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
