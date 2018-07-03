/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#include "ShearObjectsTool.h"

#include "Preferences.h"
#include "PreferenceManager.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/CollectMatchingBrushFacesVisitor.h"
#include "Model/FindMatchingBrushFaceVisitor.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/NodeVisitor.h"
#include "Model/PickResult.h"
#include "Renderer/Camera.h"
#include "View/Grid.h"
#include "View/MapDocument.h"

#include <algorithm>
#include <iterator>
#include <array>
#include <set>

namespace TrenchBroom {
    namespace View {
        const Model::Hit::HitType ShearObjectsTool::ShearToolFaceHit = Model::Hit::freeHitType();

        ShearObjectsTool::ShearObjectsTool(MapDocumentWPtr document) :
        Tool(false),
        m_document(document),
        m_dragStartHit(Model::Hit::NoHit),
        m_resizing(false),
        m_constrainVertical(false)
        {
            bindObservers();
        }
        
        ShearObjectsTool::~ShearObjectsTool() {
            unbindObservers();
        }
        
        bool ShearObjectsTool::applies() const {
            MapDocumentSPtr document = lock(m_document);
            return !document->selectedNodes().empty();
        }

        void ShearObjectsTool::pickBackSides(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) {
            const BBox3& myBounds = bounds();

            // select back faces. Used for both 2D and 3D.
            if (pickResult.empty()) {

                FloatType closestDistToRay = std::numeric_limits<FloatType>::max();
                FloatType bestDistAlongRay = std::numeric_limits<FloatType>::max();
                Vec3 bestNormal;

                // idea is: find the closest point on an edge of the cube, belonging
                // to a face that's facing away from the pick ray.
                auto visitor = [&](const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& p3, const Vec3& n){
                    const FloatType cosAngle = n.dot(pickRay.direction);
                    if (cosAngle >= 0.0 && cosAngle < 1.0) {
                        // the face is pointing away from the camera (or exactly perpendicular)
                        // but not equal to the camera direction (important for 2D views)

                        const std::array<Vec3, 4> points{p0, p1, p2, p3};
                        for (size_t i = 0; i < 4; i++) {
                            const Ray3::LineDistance result = pickRay.distanceToSegment(points[i], points[(i + 1) % 4]);
                            if (!Math::isnan(result.distance) && result.distance < closestDistToRay) {
                                closestDistToRay = result.distance;
                                bestNormal = n;
                                bestDistAlongRay = result.rayDistance;
                            }
                        }
                    }
                };
                eachBBoxFace(myBounds, visitor);

                // The hit point is the closest point on the pick ray to one of the edges of the face.
                // For face dragging, we'll project the pick ray onto the line through this point and having the face normal.
                assert(bestNormal != Vec3::Null);
                pickResult.addHit(Model::Hit(ShearToolFaceHit, bestDistAlongRay, pickRay.pointAtDistance(bestDistAlongRay), BBoxSide{bestNormal}));

                //std::cout << "closest: " << pickRay.pointAtDistance(bestDistAlongRay) << "\n";
            }
        }

        void ShearObjectsTool::pick2D(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) {
            const BBox3& myBounds = bounds();

            // origin in bbox
            if (myBounds.contains(pickRay.origin))
                return;

            Model::PickResult localPickResult;

            pickBackSides(pickRay, camera, localPickResult);

            auto hit = localPickResult.query().first();

#if 0
            if (hit.type() == ScaleToolFaceHit)
                std::cout << "hit face " << normalForBBoxSide(hit.target<BBoxSide>()) << "\n";
            else if (hit.type() == ScaleToolEdgeHit)
                printf("hit edge\n");
            else if (hit.type() == ScaleToolCornerHit)
                printf("hit corner\n");
            else
                printf("no hit\n");
#endif

            if (hit.isMatch()) {
                pickResult.addHit(hit);
            }
        }

        void ShearObjectsTool::pick3D(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) {
            const BBox3& myBounds = bounds();

            // origin in bbox
            if (myBounds.contains(pickRay.origin))
                return;

            Model::PickResult localPickResult;

            // these handles only work in 3D.
            assert(camera.perspectiveProjection());

            // faces
            for (const BBoxSide& side : AllSides()) {
                const auto poly = polygonForBBoxSide(myBounds, side);

                const FloatType dist = intersectPolygonWithRay(pickRay, poly.begin(), poly.end());
                if (!Math::isnan(dist)) {
                    localPickResult.addHit(Model::Hit(ShearToolFaceHit, dist, pickRay.pointAtDistance(dist), side));
                }
            }

            pickBackSides(pickRay, camera, localPickResult);

            auto hit = localPickResult.query().first();

#if 0
            if (hit.type() == ScaleToolFaceHit)
                std::cout << "hit face " << normalForBBoxSide(hit.target<BBoxSide>()) << "\n";
            else if (hit.type() == ScaleToolEdgeHit)
                printf("hit edge\n");
            else if (hit.type() == ScaleToolCornerHit)
                printf("hit corner\n");
            else
                printf("no hit\n");
#endif

            if (hit.isMatch()) {
                pickResult.addHit(hit);
            }
        }


        BBox3 ShearObjectsTool::bounds() const {
            MapDocumentSPtr document = lock(m_document);
            return document->selectionBounds();
        }
        
        // used for rendering
        
        static std::vector<Polygon3f> polysForSides(const BBox3& box,
                                                    const std::vector<BBoxSide>& sides) {
            std::vector<Polygon3f> result;
            for (const auto& side : sides) {
                result.push_back(Polygon3f(polygonForBBoxSide(box, side)));
            }
            return result;
        }

        std::vector<Polygon3f> ShearObjectsTool::polygonsHighlightedByDrag() const {
            std::vector<BBoxSide> sides;

            if (m_dragStartHit.type() == ShearToolFaceHit) {
                const auto side = m_dragStartHit.target<BBoxSide>();
                sides = {side};
            } else {
                // ???
            }

            return polysForSides(bounds(), sides);
        }
        
        bool ShearObjectsTool::hasDragPolygon() const {
            return dragPolygon().vertexCount() > 0;
        }

        Polygon3f ShearObjectsTool::dragPolygon() const {
            if (m_dragStartHit.type() == ShearToolFaceHit) {
                const auto side = m_dragStartHit.target<BBoxSide>();
                return Polygon3f(polygonForBBoxSide(bounds(), side));
            }
                                                            
            return Polygon3f();
        }

        // for rendering sheared bbox
        BBox3 ShearObjectsTool::bboxAtDragStart() const {
            if (m_resizing) {
                return m_bboxAtDragStart;
            } else {
                return bounds();
            }
        }
        Mat4x4 ShearObjectsTool::bboxShearMatrix() const {
            if (!m_resizing) {
                return Mat4x4::Identity;
            }

            // happens if you cmd+drag on an edge or corner
            if (m_dragStartHit.type() != ShearToolFaceHit) {
                return Mat4x4::Identity;
            }
            
            const BBoxSide side = m_dragStartHit.target<BBoxSide>();
            
            return shearBBoxMatrix(m_bboxAtDragStart,
                                   side.normal,
                                   m_totalDelta);
        }
        Polygon3f ShearObjectsTool::shearHandle() const {
            // happens if you cmd+drag on an edge or corner
            if (m_dragStartHit.type() != ShearToolFaceHit) {
                return Polygon3f();
            }
            
            const BBoxSide side = m_dragStartHit.target<BBoxSide>();
            // use the bboxAtDragStart() function so we get bounds() if we're not currently inside a drag.
            const Polygon3 polyAtDragStart = polygonForBBoxSide(bboxAtDragStart(), side);
            
            const Polygon3 handle = polyAtDragStart.transformed(bboxShearMatrix());
            return Polygon3f(handle);
        }

        void ShearObjectsTool::updateDragFaces(const Model::PickResult& pickResult) {
            const Model::Hit& hit = pickResult.query().type(ShearToolFaceHit).occluded().first();

            // hack for highlighting on mouseover
            m_dragStartHit = hit;

            // TODO: extract the highlighted handle from the hit here, and only refresh views if it changed
            // (see ResizeBrushesTool::updateDragFaces)
            refreshViews();
        }
        
        bool ShearObjectsTool::beginResize(const Model::PickResult& pickResult) {
            const Model::Hit& hit = pickResult.query().type(ShearToolFaceHit).occluded().first();
            if (!hit.isMatch())
                return false;
            
            m_dragStartHit = hit;
            m_bboxAtDragStart = bounds();
            m_dragOrigin = hit.hitPoint();
            m_totalDelta = Vec3::Null;
            
            if (hit.type() == ShearToolFaceHit)
                printf("start face\n");
            else
                assert(0);

            MapDocumentSPtr document = lock(m_document);
            document->beginTransaction("Resize Brushes");
            m_resizing = true;

            return true;
        }
        
        void ShearObjectsTool::commitResize() {
            MapDocumentSPtr document = lock(m_document);
            if (m_totalDelta.null()) {
                document->cancelTransaction();
            } else {
                document->commitTransaction();
            }
            m_resizing = false;
        }
        
        void ShearObjectsTool::cancelResize() {
            MapDocumentSPtr document = lock(m_document);
            document->cancelTransaction();
            m_resizing = false;
        }
        
        void ShearObjectsTool::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->nodesWereAddedNotifier.addObserver(this, &ShearObjectsTool::nodesDidChange);
            document->nodesWillChangeNotifier.addObserver(this, &ShearObjectsTool::nodesDidChange);
            document->nodesWillBeRemovedNotifier.addObserver(this, &ShearObjectsTool::nodesDidChange);
            document->selectionDidChangeNotifier.addObserver(this, &ShearObjectsTool::selectionDidChange);
        }
        
        void ShearObjectsTool::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->nodesWereAddedNotifier.removeObserver(this, &ShearObjectsTool::nodesDidChange);
                document->nodesWillChangeNotifier.removeObserver(this, &ShearObjectsTool::nodesDidChange);
                document->nodesWillBeRemovedNotifier.removeObserver(this, &ShearObjectsTool::nodesDidChange);
                document->selectionDidChangeNotifier.removeObserver(this, &ShearObjectsTool::selectionDidChange);
            }
        }
        
        void ShearObjectsTool::nodesDidChange(const Model::NodeList& nodes) {
        }

        void ShearObjectsTool::selectionDidChange(const Selection& selection) {
        }

        bool ShearObjectsTool::constrainVertical() const {
            return m_constrainVertical;
        }

        void ShearObjectsTool::setConstrainVertical(const bool constrainVertical) {
            m_constrainVertical = constrainVertical;
        }
    }
}
