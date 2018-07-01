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
        const Model::Hit::HitType ShearObjectsTool::ScaleToolFaceHit = Model::Hit::freeHitType();
        const Model::Hit::HitType ShearObjectsTool::ScaleToolEdgeHit = Model::Hit::freeHitType();
        const Model::Hit::HitType ShearObjectsTool::ScaleToolCornerHit = Model::Hit::freeHitType();

        ShearObjectsTool::ShearObjectsTool(MapDocumentWPtr document) :
        Tool(false),
        m_document(document),
        m_toolPage(nullptr),
        m_dragStartHit(Model::Hit::NoHit),
        m_resizing(false),
        m_anchorPos(AnchorPos::Opposite),
        m_scaleAllAxes(false)
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
                pickResult.addHit(Model::Hit(ScaleToolFaceHit, bestDistAlongRay, pickRay.pointAtDistance(bestDistAlongRay), BBoxSide{bestNormal}));

                //std::cout << "closest: " << pickRay.pointAtDistance(bestDistAlongRay) << "\n";
            }
        }

        void ShearObjectsTool::pick2D(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) {
            const BBox3& myBounds = bounds();

            // origin in bbox
            if (myBounds.contains(pickRay.origin))
                return;

            Model::PickResult localPickResult;

            // bbox corners in 2d views
            assert(camera.orthographicProjection());
            for (const BBoxEdge& edge : AllEdges()) {
                const Edge3 points = pointsForBBoxEdge(myBounds, edge);

                // in 2d views, only use edges that are parallel to the camera
                if (points.direction().parallelTo(camera.direction())) {
                    // could figure out which endpoint is closer to camera, or just test both.
                    for (const Vec3& point : Vec3::List{points.start(), points.end()}) {
                        const FloatType dist = camera.pickPointHandle(pickRay, point, pref(Preferences::HandleRadius));
                        if (!Math::isnan(dist)) {
                            localPickResult.addHit(Model::Hit(ScaleToolEdgeHit, dist, pickRay.pointAtDistance(dist), edge));
                        }
                    }
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

        void ShearObjectsTool::pick3D(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) {
            const BBox3& myBounds = bounds();

            // origin in bbox
            if (myBounds.contains(pickRay.origin))
                return;

            Model::PickResult localPickResult;

            // these handles only work in 3D.
            assert(camera.perspectiveProjection());

            // corners
            for (const BBoxCorner& corner : AllCorners()) {
                const Vec3 point = pointForBBoxCorner(myBounds, corner);

                // make the spheres for the corner handles slightly larger than the
                // cylinders of the edge handles, so they take priority where they overlap.
                const FloatType cornerRadius = pref(Preferences::HandleRadius) * 2.0;
                const FloatType dist = camera.pickPointHandle(pickRay, point, cornerRadius);
                if (!Math::isnan(dist)) {
                    localPickResult.addHit(Model::Hit(ScaleToolCornerHit, dist, pickRay.pointAtDistance(dist), corner));
                }
            }

            // edges
            for (const BBoxEdge& edge : AllEdges()) {
                const Edge3 points = pointsForBBoxEdge(myBounds, edge);

                const FloatType dist = camera.pickLineSegmentHandle(pickRay, points, pref(Preferences::HandleRadius));
                if (!Math::isnan(dist)) {
                    localPickResult.addHit(Model::Hit(ScaleToolEdgeHit, dist, pickRay.pointAtDistance(dist), edge));
                }
            }

            // faces
            for (const BBoxSide& side : AllSides()) {
                const auto poly = polygonForBBoxSide(myBounds, side);

                const FloatType dist = intersectPolygonWithRay(pickRay, poly.begin(), poly.end());
                if (!Math::isnan(dist)) {
                    localPickResult.addHit(Model::Hit(ScaleToolFaceHit, dist, pickRay.pointAtDistance(dist), side));
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
        
        /**
         * For dragging a corner retursn the 3 sides that touch that corner
         */
        static std::vector<BBoxSide> sidesForCornerSelection(const BBoxCorner corner) {
            std::vector<BBoxSide> result;
            for (size_t i = 0; i < 3; ++i) {
                Vec3 sideNormal = Vec3::Null;
                sideNormal[i] = corner.corner[i];
                
                result.push_back(BBoxSide(sideNormal));
            }
            assert(result.size() == 3);
            return result;
        }
        
        /**
         * For dragging an edge, returns the 2 bbox sides that contain that edge
         */
        static std::vector<BBoxSide> sidesForEdgeSelection(const BBoxEdge edge) {
            std::vector<BBoxSide> result;
            
            const BBox3 box{{-1, -1, -1}, {1, 1, 1}};
            
            auto visitor = [&](const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& p3, const Vec3& n){
                const Vec3 verts[4] = {p0, p1, p2, p3};
                
                // look for the edge
                for (size_t i = 0; i < 4; ++i) {
                    if ((verts[i] == edge.point0 && verts[(i+1)%4] == edge.point1)
                        || (verts[i] == edge.point1 && verts[(i+1)%4] == edge.point0)) {
                        result.push_back(BBoxSide(n));
                    }
                }
                
            };
            eachBBoxFace(box, visitor);
            assert(result.size() == 2);
            
            return result;
        }
        
        static std::vector<Polygon3f> polysForSides(const BBox3& box,
                                                    const std::vector<BBoxSide>& sides) {
            std::vector<Polygon3f> result;
            for (const auto& side : sides) {
                result.push_back(Polygon3f(polygonForBBoxSide(box, side)));
            }
            return result;
        }

        static std::vector<BBoxSide> oppositeSides(const std::vector<BBoxSide>& sides) {
            std::vector<BBoxSide> result;
            for (const auto& side : sides) {
                result.push_back(oppositeSide(side));
            }
            return result;
        }

        static std::vector<BBoxSide> sidesWithOppositeSides(const std::vector<BBoxSide>& sides) {
            std::set<BBoxSide> result;
            for (const auto& side : sides) {
                result.insert(side);
                result.insert(oppositeSide(side));
            }

            return std::vector<BBoxSide>(result.begin(), result.end());
        }

        static std::vector<BBoxSide> allSidesExcept(const std::vector<BBoxSide>& sides) {
            std::set<BBoxSide> result = SetUtils::makeSet(AllSides());
            for (const auto& side : sides) {
                result.erase(side);
            }
            return std::vector<BBoxSide>(result.begin(), result.end());
        }

        std::vector<Polygon3f> ShearObjectsTool::polygonsHighlightedByDrag() const {
            std::vector<BBoxSide> sides;

            if (m_dragStartHit.type() == ScaleToolFaceHit) {
                const auto side = m_dragStartHit.target<BBoxSide>();
                sides = {side};
            } else if (m_dragStartHit.type() == ScaleToolEdgeHit) {
                const auto edge = m_dragStartHit.target<BBoxEdge>();
                sides = sidesForEdgeSelection(edge);
            } else if (m_dragStartHit.type() == ScaleToolCornerHit) {
                const auto corner = m_dragStartHit.target<BBoxCorner>();
                sides = sidesForCornerSelection(corner);
            } else {
                // ???
            }

            // When dragging all axes, change the highlighted sides to "all except the opposites"
            if (m_scaleAllAxes) {
                sides = allSidesExcept(oppositeSides(sides));
            }

            // When the anchor point is the center, highlight the opposite sides also.
            if (m_anchorPos == AnchorPos::Center) {
                sides = sidesWithOppositeSides(sides);
            }

            return polysForSides(bounds(), sides);
        }
        
        bool ShearObjectsTool::hasDragPolygon() const {
            return dragPolygon().vertexCount() > 0;
        }

        Polygon3f ShearObjectsTool::dragPolygon() const {
            if (m_dragStartHit.type() == ScaleToolFaceHit) {
                const auto side = m_dragStartHit.target<BBoxSide>();
                return Polygon3f(polygonForBBoxSide(bounds(), side));
            }
                                                            
            return Polygon3f();
        }
        
        bool ShearObjectsTool::hasDragEdge() const {
            return m_dragStartHit.type() == ScaleToolEdgeHit;
        }
        
        Edge3f ShearObjectsTool::dragEdge() const {
            assert(hasDragEdge());
            auto whichEdge = m_dragStartHit.target<BBoxEdge>();
            return Edge3f(pointsForBBoxEdge(bounds(), whichEdge));
        }
        
        bool ShearObjectsTool::hasDragCorner() const {
            return m_dragStartHit.type() == ScaleToolCornerHit;
        }
        
        Vec3f ShearObjectsTool::dragCorner() const {
            assert(hasDragCorner());
            auto whichCorner = m_dragStartHit.target<BBoxCorner>();
            return Vec3f(pointForBBoxCorner(bounds(), whichCorner));
        }

        bool ShearObjectsTool::hasDragAnchor() const {
            if (bounds().empty()) {
                return false;
            }

            const auto type = m_dragStartHit.type();
            return type == ScaleToolEdgeHit
                   || type == ScaleToolCornerHit
                   || type == ScaleToolFaceHit;
        }

        Vec3f ShearObjectsTool::dragAnchor() const {
            if (m_anchorPos == AnchorPos::Center) {
                return Vec3f(bounds().center());
            }

            if (m_dragStartHit.type() == ScaleToolFaceHit) {
                const auto endSide = m_dragStartHit.target<BBoxSide>();
                const auto startSide = oppositeSide(endSide);

                return Vec3f(centerForBBoxSide(bounds(), startSide));
            } else if (m_dragStartHit.type() == ScaleToolEdgeHit) {
                const auto endEdge = m_dragStartHit.target<BBoxEdge>();
                const auto startEdge = oppositeEdge(endEdge);

                const Edge3 startEdgeActual = pointsForBBoxEdge(bounds(), startEdge);

                return Vec3f(startEdgeActual.center());
            } else if (m_dragStartHit.type() == ScaleToolCornerHit) {
                const auto endCorner = m_dragStartHit.target<BBoxCorner>();
                const auto startCorner = oppositeCorner(endCorner);

                const auto startCornerActual = pointForBBoxCorner(bounds(), startCorner);
                return Vec3f(startCornerActual);
            }

            assert(0);
            return Vec3f::Null;
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
            assert(m_isShearing);

            if (!m_resizing) {
                return Mat4x4::Identity;
            }

            // happens if you cmd+drag on an edge or corner
            if (m_dragStartHit.type() != ScaleToolFaceHit) {
                return Mat4x4::Identity;
            }
            
            const BBoxSide side = m_dragStartHit.target<BBoxSide>();
            
            return shearBBoxMatrix(m_bboxAtDragStart,
                                   side.normal,
                                   m_totalDelta);
        }
        Polygon3f ShearObjectsTool::shearHandle() const {
            assert(m_isShearing);
            
            // happens if you cmd+drag on an edge or corner
            if (m_dragStartHit.type() != ScaleToolFaceHit) {
                return Polygon3f();
            }
            
            const BBoxSide side = m_dragStartHit.target<BBoxSide>();
            // use the bboxAtDragStart() function so we get bounds() if we're not currently inside a drag.
            const Polygon3 polyAtDragStart = polygonForBBoxSide(bboxAtDragStart(), side);
            
            const Polygon3 handle = polyAtDragStart.transformed(bboxShearMatrix());
            return Polygon3f(handle);
        }

        void ShearObjectsTool::setShearing(bool shearing) {
            // FIXME: Ensure we are not dragging
            m_isShearing = shearing;
        }
        bool ShearObjectsTool::isShearing() const {
            return m_isShearing;
        }

        Vec3::List ShearObjectsTool::cornerHandles() const {
            if (bounds().empty()) {
                return {};
            }

            Vec3::List result;
            result.reserve(8);
            auto op = [&](const Vec3& point) {
                result.push_back(point);
            };
            eachBBoxVertex(bounds(), op);
            return result;
        }

        void ShearObjectsTool::updateDragFaces(const Model::PickResult& pickResult) {
            const Model::Hit& hit = pickResult.query().type(ScaleToolFaceHit | ScaleToolEdgeHit | ScaleToolCornerHit).occluded().first();

            // hack for highlighting on mouseover
            m_dragStartHit = hit;

            // TODO: extract the highlighted handle from the hit here, and only refresh views if it changed
            // (see ResizeBrushesTool::updateDragFaces)
            refreshViews();
        }

        void ShearObjectsTool::setAnchorPos(const AnchorPos pos) {
            m_anchorPos = pos;
        }

        AnchorPos ShearObjectsTool::anchorPos() const {
            return m_anchorPos;
        }

        void ShearObjectsTool::setScaleAllAxes(bool allAxes) {
            m_scaleAllAxes = allAxes;
        }

        bool ShearObjectsTool::scaleAllAxes() const {
            return m_scaleAllAxes;
        }
        
        bool ShearObjectsTool::beginResize(const Model::PickResult& pickResult) {
            const Model::Hit& hit = pickResult.query().type(ScaleToolFaceHit | ScaleToolEdgeHit | ScaleToolCornerHit).occluded().first();
            if (!hit.isMatch())
                return false;
            
            m_dragStartHit = hit;
            m_bboxAtDragStart = bounds();
            m_dragOrigin = hit.hitPoint();
            m_totalDelta = Vec3::Null;
            
            if (hit.type() == ScaleToolFaceHit)
                printf("start face\n");
            else if (hit.type() == ScaleToolEdgeHit)
                printf("start edge\n");
            else if (hit.type() == ScaleToolCornerHit)
                printf("start corner\n");
            else
                assert(0);

            MapDocumentSPtr document = lock(m_document);
            document->beginTransaction("Resize Brushes");
            m_resizing = true;

            return true;
        }

        bool ShearObjectsTool::resize(const Ray3& pickRay, const Renderer::Camera& camera, const bool vertical) {
            MapDocumentSPtr document = lock(m_document);
            const View::Grid& grid = document->grid();
           
            if (!m_isShearing) {
                const auto anchorPos = m_anchorPos;

                // side dragging
                if (m_dragStartHit.type() == ScaleToolFaceHit) {
                    const auto endSide = m_dragStartHit.target<BBoxSide>();

                    // This is the line that our invisible handle will be dragged along.
                    // It doesn't necessarily intersect the bbox.
                    const Line3 handleLine(m_dragOrigin, normalForBBoxSide(endSide));

                    // project pickRay onto handleLine
                    const Ray3::LineDistance distance = pickRay.distanceToLine(handleLine.point, handleLine.direction);
                    if (distance.parallel) {
                        return true;
                    }
                    const Vec3 handlePos = handleLine.pointAtDistance(distance.lineDistance);


                    // grid snapping
                    const Vec3 dragOriginHandlePosSnapped = grid.snap(m_dragOrigin, handleLine);
                    const Vec3 handlePosSnapped = grid.snap(handlePos, handleLine);

                    const Vec3 delta = handlePosSnapped - dragOriginHandlePosSnapped;

                    // debug
                    std::cout << "new delta: " << delta << "\n";
                    m_handlePos = handlePosSnapped;

                    // do the resize
                    const BBox3 newBbox= moveBBoxFace(m_bboxAtDragStart, endSide, delta, m_scaleAllAxes, anchorPos);

                    if (newBbox.empty()) {
                        std::cout << "skipping because empty\n";
                    } else if (newBbox == bounds()) {
                        std::cout << "skipping because no change\n";
                    } else {
                        if (document->scaleObjects(bounds(), newBbox)) {
                            m_totalDelta += Vec3(1,0,0); // FIXME:
                            //m_dragOrigin += faceDelta;
                        }
                    }

                    return true;
                }

                // edge, corner
                Vec3 handleLineStart, handleLineEnd;
                if (m_dragStartHit.type() == ScaleToolEdgeHit) {
                    const auto endEdge = m_dragStartHit.target<BBoxEdge>();
                    const auto startEdge = oppositeEdge(endEdge);

                    const Edge3 endEdgeActual = pointsForBBoxEdge(m_bboxAtDragStart, endEdge);
                    const Edge3 startEdgeActual = pointsForBBoxEdge(m_bboxAtDragStart, startEdge);

                    handleLineStart = startEdgeActual.center();
                    handleLineEnd = endEdgeActual.center();

                    std::cout << "ShearObjectsTool::resize from edge " << handleLineStart << " to " << handleLineEnd << "\n";
                } else if (m_dragStartHit.type() == ScaleToolCornerHit) {
                    const auto endCorner = m_dragStartHit.target<BBoxCorner>();
                    const auto startCorner = oppositeCorner(endCorner);

                    handleLineStart = pointForBBoxCorner(m_bboxAtDragStart, startCorner);
                    handleLineEnd = pointForBBoxCorner(m_bboxAtDragStart, endCorner);

                    std::cout << "ShearObjectsTool::resize from corner " << handleLineStart << " to " << handleLineEnd << "\n";
                } else {
                    assert(0);
                }

                // This is a dir from:
                //  - edge dragging: midpoint of diagonally opposite edge to midpoint of edge being dragged
                //  - corner dragging: diagonally opposite corner to corner being dragged
                const Vec3 handleLineDir = (handleLineEnd - handleLineStart).normalized();
                const Line3 handleLine(handleLineStart, handleLineDir);

                // project m_dragOrigin and pickRay onto handleLine

                const Vec3 dragOriginHandlePos = handleLine.pointOnLineClosestToPoint(m_dragOrigin);
                // project pickRay onto handleLine
                const Ray3::LineDistance distance = pickRay.distanceToLine(handleLine.point, handleLine.direction);
                if (distance.parallel) {
                    return true;
                }
                const Vec3 handlePos = handleLine.pointAtDistance(distance.lineDistance);

                // grid snapping
                const Vec3 dragOriginHandlePosSnapped = grid.snap(dragOriginHandlePos, handleLine);
                const Vec3 handlePosSnapped = grid.snap(handlePos, handleLine);

                const Vec3 delta = handlePosSnapped - dragOriginHandlePosSnapped;

                // debug
                std::cout << "new delta: " << delta << "\n";
                m_handlePos = handlePosSnapped;

                // do the resize
                BBox3 newBbox;
                if (m_dragStartHit.type() == ScaleToolEdgeHit) {
                    const auto edge = m_dragStartHit.target<BBoxEdge>();
                    newBbox = moveBBoxEdge(m_bboxAtDragStart, edge, delta, m_scaleAllAxes, anchorPos);
                } else if (m_dragStartHit.type() == ScaleToolCornerHit) {
                    const auto corner = m_dragStartHit.target<BBoxCorner>();
                    newBbox = moveBBoxCorner(m_bboxAtDragStart, corner, delta, anchorPos);
                } else {
                    assert(0);
                }

                if (newBbox.empty()) {
                    std::cout << "skipping because empty\n";
                } else if (newBbox == bounds()) {
                    std::cout << "skipping because no change\n";
                } else {
                    if (document->scaleObjects(bounds(), newBbox)) {
                        m_totalDelta += Vec3(1,0,0); // FIXME:
                        //m_dragOrigin += faceDelta;
                    }
                }
                return true;
            } else {
                // shear
                if (m_dragStartHit.type() == ScaleToolFaceHit) {
                    const BBoxSide side = m_dragStartHit.target<BBoxSide>();
                    
                    const auto poly = polygonForBBoxSide(bounds(), side);
                    const Vec3 planeAnchor = poly.vertices().front();
                    
                    // get the point where the pick ray intersects the plane being dragged.
                    Vec3 rayHit = pickRay.pointAtDistance(pickRay.intersectWithPlane(side.normal, planeAnchor));
                    if (rayHit.nan()) {
                        // in 2D views the pick ray will be perpendicular to the face normal.
                        // in that case, use a plane with a normal opposite the pickRay.
                        rayHit = pickRay.pointAtDistance(pickRay.intersectWithPlane(pickRay.direction * -1.0, planeAnchor));
                    }
                    assert(!rayHit.nan());
                    
                    std::cout << "make shear with rayHit: " << rayHit << "\n";

                    //m_dragOrigin = rayHit;
                    
                    Vec3 delta = rayHit - m_dragOrigin;
                    delta = grid.snap(delta);
                    
                    if (camera.perspectiveProjection()) {
                        if (vertical) {
                            delta[0] = 0;
                            delta[1] = 0;
                        } else {
                            delta[2] = 0;
                        }
                    } else if (camera.orthographicProjection()) {
                        const Plane3 cameraPlane(0.0, camera.direction());
                        delta = cameraPlane.projectVector(delta);
                    } else {
                        assert(0);
                    }

                    if (!delta.null()) {
                        std::cout << "make shear with m_dragOrigin: " << m_dragOrigin << "\n";

                        std::cout << "make shear with delta: " << delta << "on side" << side.normal << "\n";
                        if (document->shearObjects(bounds(), side.normal, delta)) {
                            // only used to tell whether to commit the shear
                            m_totalDelta += delta;
                            
                            // update the ref point for the next iteration
                            m_dragOrigin = rayHit;
                        }
                    }
                }
                
            }
            
            
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

//        wxWindow* ShearObjectsTool::doCreatePage(wxWindow* parent) {
//            assert(m_toolPage == nullptr);
//            m_toolPage = new ShearObjectsToolPage(parent, m_document);
//            return m_toolPage;
//        }
    }
}
