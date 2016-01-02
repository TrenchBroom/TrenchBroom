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
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
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
            if (m_tool->dragPoint(curPoint, snapper, Vec3::EmptyList, snapped))
                refPoint = snapped;
            return true;
        }
        
        void ClipToolAdapter2D::doEndPlaneDrag(const InputState& inputState) {
            m_tool->endDragPoint();
        }
        
        void ClipToolAdapter2D::doCancelPlaneDrag() {
            m_tool->endDragPoint();
        }
        
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
            
            m_tool->addPoint(hitPoint, snapper, Vec3::List(1, viewDir));
            return true;
        }

        bool ClipToolAdapter2D::doSetClipPlane(const InputState& inputState) {
            return false;
        }

        void ClipToolAdapter2D::doRenderFeedback(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (dragging())
                return;
            
            const Renderer::Camera& camera = inputState.camera();
            const Vec3 viewDir = camera.direction().firstAxis();
            
            const Ray3& pickRay = inputState.pickRay();
            const Vec3 defaultPos = m_tool->defaultClipPointPos();
            const FloatType distance = pickRay.intersectWithPlane(viewDir, defaultPos);
            if (Math::isnan(distance))
                return;
            
            const Vec3 hitPoint = pickRay.pointAtDistance(distance);
            const PointSnapper snapper(m_grid);
            if (!m_tool->canAddPoint(hitPoint, snapper))
                return;
            
            m_tool->renderFeedback(renderContext, renderBatch, hitPoint, snapper);
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
                snappedPoint = m_grid.snap(point, m_currentFace->boundary());
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
                const Vec3& point = hit.hitPoint();
                Model::BrushFace* face = hit.target<Model::BrushFace*>();

                const PointSnapper snapper(m_grid, Model::hitToFace(hit));
                Vec3 snapped;

                const Vec3::List helpVectors = selectHelpVectors(face, point);
                m_tool->dragPoint(hit.hitPoint(), snapper, helpVectors, snapped);
            }
            return true;
        }
        
        void ClipToolAdapter3D::doEndMouseDrag(const InputState& inputState) {
            m_tool->endDragPoint();
        }
        
        void ClipToolAdapter3D::doCancelMouseDrag() {
            m_tool->endDragPoint();
        }
        
        bool ClipToolAdapter3D::doAddClipPoint(const InputState& inputState) {
            const Model::Hit& hit = inputState.pickResult().query().pickable().type(Model::Brush::BrushHit).occluded().first();
            if (!hit.isMatch())
                return false;
            
            const Vec3& point = hit.hitPoint();
            Model::BrushFace* face = hit.target<Model::BrushFace*>();
            
            const PointSnapper snapper(m_grid, face);
            if (!m_tool->canAddPoint(point, snapper))
                return false;
            
            const Vec3::List helpVectors = selectHelpVectors(face, point);
            m_tool->addPoint(point, snapper, helpVectors);
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

        void ClipToolAdapter3D::doRenderFeedback(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (dragging())
                return;
            const Model::Hit& hit = inputState.pickResult().query().pickable().type(Model::Brush::BrushHit).occluded().first();
            if (!hit.isMatch())
                return;
            
            const Vec3& point = hit.hitPoint();
            Model::BrushFace* face = hit.target<Model::BrushFace*>();
            
            const PointSnapper snapper(m_grid, face);
            if (!m_tool->canAddPoint(point, snapper))
                return;
            
            m_tool->renderFeedback(renderContext, renderBatch, point, snapper);
        }
        
        Vec3::List ClipToolAdapter3D::selectHelpVectors(Model::BrushFace* face, const Vec3& hitPoint) const {
            assert(face != NULL);

            Vec3::List result;
            const Model::BrushFaceList incidentFaces = selectIncidentFaces(face, hitPoint);
            Model::BrushFaceList::const_iterator it, end;
            for (it = incidentFaces.begin(), end = incidentFaces.end(); it != end; ++it) {
                const Model::BrushFace* incidentFace = *it;
                const Vec3& normal = incidentFace->boundary().normal;
                result.push_back(normal.firstAxis());
            }
            
            return result;
        }

        Model::BrushFaceList ClipToolAdapter3D::selectIncidentFaces(Model::BrushFace* face, const Vec3& hitPoint) const {
            const Model::BrushFace::VertexList vertices = face->vertices();
            Model::BrushFace::VertexList::const_iterator vIt, vEnd;
            for (vIt = vertices.begin(), vEnd = vertices.end(); vIt != vEnd; ++vIt) {
                const Model::BrushVertex* vertex = *vIt;
                if (vertex->position().equals(hitPoint)) {
                    const Model::Brush* brush = face->brush();
                    return brush->incidentFaces(vertex);
                }
            }
            
            const Model::BrushFace::EdgeList edges = face->edges();
            Model::BrushFace::EdgeList::const_iterator eIt, eEnd;
            for (eIt = edges.begin(), eEnd = edges.end(); eIt != eEnd; ++eIt) {
                Model::BrushEdge* edge = *eIt;
                if (edge->contains(hitPoint)) {
                    Model::BrushFaceList result;
                    result.push_back(edge->firstFace()->payload());
                    result.push_back(edge->secondFace()->payload());
                    return result;
                }
            }
            
            return Model::BrushFaceList(1, face);
        }
    }
}
