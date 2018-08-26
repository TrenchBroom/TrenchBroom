/*
 Copyright (C) 2010-2017 Kristian Duske
 Copyright (C) 2018 Eric Wasylishen
 
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

#include "ScaleObjectsTool.h"

#include "Preferences.h"
#include "PreferenceManager.h"
#include "Model/HitQuery.h"
#include "Model/PickResult.h"
#include "Renderer/Camera.h"
#include "View/Grid.h"
#include "View/MapDocument.h"

#include <algorithm>
#include <iterator>
#include <set>

namespace TrenchBroom {
    namespace View {
        const Model::Hit::HitType ScaleObjectsTool::ScaleToolSideHit = Model::Hit::freeHitType();
        const Model::Hit::HitType ScaleObjectsTool::ScaleToolEdgeHit = Model::Hit::freeHitType();
        const Model::Hit::HitType ScaleObjectsTool::ScaleToolCornerHit = Model::Hit::freeHitType();

        // Scale tool helper functions

        bool BBoxSide::validSideNormal(const vec3& n) {
            for (size_t i = 0; i < 3; ++i) {
                vec3 expected = vec3::zero;
                expected[i] = 1.0;
                if (n == expected || n == -expected) {
                    return true;
                }
            }
            return false;
        }

        BBoxSide::BBoxSide(const vec3& n)
                : normal(n) {
            if (!validSideNormal(n)) {
                throw std::invalid_argument("BBoxSide created with invalid normal " + StringUtils::toString(n));
            }
        }

        bool BBoxSide::operator<(const BBoxSide& other) const {
            return normal < other.normal;
        }

        bool BBoxSide::operator==(const BBoxSide& other) const {
            return normal == other.normal;
        }

        // BBoxCorner

        bool BBoxCorner::validCorner(const vec3& c) {
            // all components must be either +1 or -1
            for (size_t i = 0; i < 3; ++i) {
                if (!(c[i] == -1.0 || c[i] == 1.0)) {
                    return false;
                }
            }
            return true;
        }

        BBoxCorner::BBoxCorner(const vec3& c) : corner(c) {
            if (!validCorner(c)) {
                throw std::invalid_argument("BBoxCorner created with invalid corner " + StringUtils::toString(c));
            }
        }

        bool BBoxCorner::operator==(const BBoxCorner& other) const {
            return corner == other.corner;
        }

        // BBoxEdge

        BBoxEdge::BBoxEdge(const vec3 &p0, const vec3& p1) : point0(p0), point1(p1) {
            if (!BBoxCorner::validCorner(p0)) {
                throw std::invalid_argument("BBoxEdge created with invalid corner " + StringUtils::toString(p0));
            }
            if (!BBoxCorner::validCorner(p1)) {
                throw std::invalid_argument("BBoxEdge created with invalid corner " + StringUtils::toString(p1));
            }
        }

        bool BBoxEdge::operator==(const BBoxEdge& other) const {
            return point0 == other.point0
                && point1 == other.point1;
        }

        // ProportionalAxes

        ProportionalAxes::ProportionalAxes(const bool xProportional, const bool yProportional, const bool zProportional) {
            m_bits.set(0, xProportional);
            m_bits.set(1, yProportional);
            m_bits.set(2, zProportional);
        }

        ProportionalAxes ProportionalAxes::All() {
            return ProportionalAxes(true, true, true);
        }

        ProportionalAxes ProportionalAxes::None() {
            return ProportionalAxes(false, false, false);
        }

        void ProportionalAxes::setAxisProportional(const size_t axis, const bool proportional) {
            m_bits.set(axis, proportional);
        }

        bool ProportionalAxes::isAxisProportional(const size_t axis) const {
            return m_bits.test(axis);
        }

        bool ProportionalAxes::allAxesProportional() const {
            return m_bits.all();
        }

        bool ProportionalAxes::operator==(const ProportionalAxes& other) const {
            return m_bits == other.m_bits;
        }

        bool ProportionalAxes::operator!=(const ProportionalAxes& other) const {
            return m_bits != other.m_bits;
        }

        // Helper functions

        std::vector<BBoxSide> allSides() {
            std::vector<BBoxSide> result;
            result.reserve(6);
            
            const BBox3 box{{-1, -1, -1}, {1, 1, 1}};
            auto op = [&](const vec3& p0, const vec3& p1, const vec3& p2, const vec3& p3, const vec3& normal) {
                result.push_back(BBoxSide(normal));
            };
            eachBBoxFace(box, op);
            
            assert(result.size() == 6);
            return result;
        }

        std::vector<BBoxEdge> allEdges() {
            std::vector<BBoxEdge> result;
            result.reserve(12);
            
            const BBox3 box{{-1, -1, -1}, {1, 1, 1}};
            auto op = [&](const vec3& p0, const vec3& p1) {
                result.push_back(BBoxEdge(p0, p1));
            };
            eachBBoxEdge(box, op);
            
            assert(result.size() == 12);
            return result;
        }
        
        std::vector<BBoxCorner> allCorners() {
            std::vector<BBoxCorner> result;
            result.reserve(8);
            
            const BBox3 box{{-1, -1, -1}, {1, 1, 1}};
            auto op = [&](const vec3& point) {
                result.push_back(BBoxCorner(point));
            };
            eachBBoxVertex(box, op);
            
            assert(result.size() == 8);
            return result;
        }
        
        vec3 pointForBBoxCorner(const BBox3& box, const BBoxCorner& corner) {
            vec3 res;
            for (size_t i = 0; i < 3; ++i) {
                assert(corner.corner[i] == 1.0 || corner.corner[i] == -1.0);
                
                res[i] = (corner.corner[i] == 1.0) ? box.max[i] : box.min[i];
            }
            return res;
        }
        
        BBoxSide oppositeSide(const BBoxSide& side) {
            return BBoxSide(side.normal * -1.0);
        }
        
        BBoxCorner oppositeCorner(const BBoxCorner& corner) {
            return BBoxCorner(vec3(-corner.corner.x(),
                                   -corner.corner.y(),
                                   -corner.corner.z()));
        }
        
        BBoxEdge oppositeEdge(const BBoxEdge& edge) {
            return BBoxEdge(oppositeCorner(BBoxCorner(edge.point0)).corner,
                            oppositeCorner(BBoxCorner(edge.point1)).corner);
        }
        
        Edge3 pointsForBBoxEdge(const BBox3& box, const BBoxEdge& edge) {
            return Edge3(pointForBBoxCorner(box, BBoxCorner(edge.point0)),
                         pointForBBoxCorner(box, BBoxCorner(edge.point1)));
        }

        Polygon3 polygonForBBoxSide(const BBox3& box, const BBoxSide& side) {
            const auto wantedNormal = side.normal;
            
            Polygon3 res;
            auto visitor = [&](const vec3& p0, const vec3& p1, const vec3& p2, const vec3& p3, const vec3& n){
                if (n == wantedNormal) {
                    const Polygon3 poly {p0, p1, p2, p3};
                    res = poly;
                }
            };
            eachBBoxFace(box, visitor);
            
            assert(res.vertexCount() == 4);
            return res;
        }
        
        vec3 centerForBBoxSide(const BBox3& box, const BBoxSide& side) {
            const auto wantedNormal = side.normal;
            
            vec3 result;
            bool setResult = false;
            
            auto visitor = [&](const vec3& p0, const vec3& p1, const vec3& p2, const vec3& p3, const vec3& n){
                if (n == wantedNormal) {
                    result = (p0 + p1 + p2 + p3) / 4.0;
                    setResult = true;
                }
            };
            eachBBoxFace(box, visitor);
            assert(setResult);
            return result;
        }

        // manipulating bboxes

        BBox3 moveBBoxSide(const BBox3 &in,
                           const BBoxSide &side,
                           const vec3 &delta,
                           ProportionalAxes proportional,
                           AnchorPos anchorType) {
            FloatType sideLengthDelta = dot(side.normal, delta);

            // when using a center anchor, we're stretching both sides
            // at once, so multiply the delta by 2.
            if (anchorType == AnchorPos::Center) {
                sideLengthDelta *= 2.0;
            }

            const size_t axis = side.normal.firstComponent();
            const FloatType inSideLenth = in.max[axis] - in.min[axis];
            const FloatType sideLength = inSideLenth + sideLengthDelta;

            if (sideLength <= 0) {
                return BBox3();
            }

            const vec3 n = side.normal;
            const size_t axis1 = n.firstComponent();
            const size_t axis2 = n.secondComponent();
            const size_t axis3 = n.thirdComponent();

            vec3 newSize = in.size();

            newSize[axis1] = sideLength;

            // optionally apply proportional scaling to axis2/axis3
            const FloatType ratio = sideLength / in.size()[axis1];
            if (proportional.isAxisProportional(axis2)) {
                newSize[axis2] *= ratio;
            }
            if (proportional.isAxisProportional(axis3)) {
                newSize[axis3] *= ratio;
            }

            const vec3 anchor = (anchorType == AnchorPos::Center)
                ? in.center()
                : centerForBBoxSide(in, oppositeSide(side));

            const auto matrix = scaleBBoxMatrixWithAnchor(in, newSize, anchor);

            return BBox3(matrix * in.min, matrix * in.max);
        }


        BBox3 moveBBoxCorner(const BBox3& in,
                             const BBoxCorner& corner,
                             const vec3& delta,
                             const AnchorPos anchorType) {

            const BBoxCorner opposite = oppositeCorner(corner);
            const vec3 oppositePoint = pointForBBoxCorner(in, opposite);
            const vec3 anchor = (anchorType == AnchorPos::Center)
                                ? in.center()
                                : oppositePoint;
            const vec3 oldCorner = pointForBBoxCorner(in, corner);
            const vec3 newCorner = oldCorner + delta;

            // check for inverting the box
            for (size_t i = 0; i < 3; ++i) {
                if (newCorner[i] == anchor[i]) {
                    return BBox3();
                }
                const bool oldPositive = oldCorner[i] > anchor[i];
                const bool newPositive = newCorner[i] > anchor[i];
                if (oldPositive != newPositive) {
                    return BBox3();
                }
            }

            if (anchorType == AnchorPos::Center) {
                return BBox3(vec3::List{anchor - (newCorner - anchor), newCorner});
            } else {
                return BBox3(vec3::List{oppositePoint, newCorner});
            }
        }

        BBox3 moveBBoxEdge(const BBox3& in,
                           const BBoxEdge& edge,
                           const vec3& delta,
                           const ProportionalAxes proportional,
                           const AnchorPos anchorType) {

            const BBoxEdge opposite = oppositeEdge(edge);
            const vec3 edgeMid = pointsForBBoxEdge(in, edge).center();
            const vec3 oppositeEdgeMid = pointsForBBoxEdge(in, opposite).center();

            const vec3 anchor = (anchorType == AnchorPos::Center)
                                ? in.center()
                                : oppositeEdgeMid;

            const vec3 oldAnchorDist = edgeMid - anchor;
            const vec3 newAnchorDist = oldAnchorDist + delta;

            // check for crossing over the anchor
            for (size_t i = 0; i < 3; ++i) {
                if ((oldAnchorDist[i] > 0) && (newAnchorDist[i] < 0)) {
                    return BBox3();
                }
                if ((oldAnchorDist[i] < 0) && (newAnchorDist[i] > 0)) {
                    return BBox3();
                }
            }

            const size_t nonMovingAxis = oldAnchorDist.thirdComponent();

            const vec3 corner1 = (anchorType == AnchorPos::Center)
                                 ? anchor - newAnchorDist
                                 : anchor;
            const vec3 corner2 = anchor + newAnchorDist;


            BBox3 result(corner1, corner2);
            result.repair();

            // the only type of proportional scaling we support is optionally
            // scaling the nonMovingAxis.
            if (proportional.isAxisProportional(nonMovingAxis)) {
                const size_t axis1 = oldAnchorDist.firstComponent();
                const FloatType ratio = result.size()[axis1] / in.size()[axis1];

                result.min[nonMovingAxis] = anchor[nonMovingAxis] - (in.size()[nonMovingAxis] * ratio * 0.5);
                result.max[nonMovingAxis] = anchor[nonMovingAxis] + (in.size()[nonMovingAxis] * ratio * 0.5);
            } else {
                result.min[nonMovingAxis] = in.min[nonMovingAxis];
                result.max[nonMovingAxis] = in.max[nonMovingAxis];
            }

            result.repair();

            // check for zero size
            for (size_t i = 0; i < 3; ++i) {
                if (Math::zero(result.size()[i], Math::Constants<FloatType>::almostZero())) {
                    return BBox3();
                }
            }

            return result;
        }

        Line3 handleLineForHit(const BBox3& bboxAtDragStart, const Model::Hit& hit) {
            Line3 handleLine;

            // NOTE: We don't need to check for the Alt modifier (moves the drag anchor to the center of the bbox)
            // because all of these lines go through the center of the box anyway, so the resulting line would be the
            // same.

            if (hit.type() == ScaleObjectsTool::ScaleToolSideHit) {
                const auto draggingSide = hit.target<BBoxSide>();

                // We could use centerForBBoxSide(draggingSide) here, but this gives better dragging behaviour.
                // See pickBackSideOfBox() for how hit.hitPoint() is determined.
                handleLine = Line3(hit.hitPoint(), draggingSide.normal);
            } else if (hit.type() == ScaleObjectsTool::ScaleToolEdgeHit) {
                const auto endEdge = hit.target<BBoxEdge>();
                const auto startEdge = oppositeEdge(endEdge);

                const Edge3 endEdgeActual = pointsForBBoxEdge(bboxAtDragStart, endEdge);
                const Edge3 startEdgeActual = pointsForBBoxEdge(bboxAtDragStart, startEdge);

                const vec3 handleLineStart = startEdgeActual.center();
                const vec3 handleLineEnd = endEdgeActual.center();

                handleLine = Line3(handleLineStart, normalize(handleLineEnd - handleLineStart));
            } else if (hit.type() == ScaleObjectsTool::ScaleToolCornerHit) {
                const auto endCorner = hit.target<BBoxCorner>();
                const auto startCorner = oppositeCorner(endCorner);

                const vec3 handleLineStart = pointForBBoxCorner(bboxAtDragStart, startCorner);
                const vec3 handleLineEnd = pointForBBoxCorner(bboxAtDragStart, endCorner);

                handleLine = Line3(handleLineStart, normalize(handleLineEnd - handleLineStart));
            } else {
                assert(0);
            }

            return handleLine;
        }

        BBox3 moveBBoxForHit(const BBox3& bboxAtDragStart,
                             const Model::Hit& dragStartHit,
                             const vec3& delta,
                             const ProportionalAxes proportional,
                             const AnchorPos anchor) {
            if (dragStartHit.type() == ScaleObjectsTool::ScaleToolSideHit) {
                const auto endSide = dragStartHit.target<BBoxSide>();

                return moveBBoxSide(bboxAtDragStart, endSide, delta, proportional, anchor);
            } else if (dragStartHit.type() == ScaleObjectsTool::ScaleToolEdgeHit) {
                const auto endEdge = dragStartHit.target<BBoxEdge>();

                return moveBBoxEdge(bboxAtDragStart, endEdge, delta, proportional, anchor);
            } else if (dragStartHit.type() == ScaleObjectsTool::ScaleToolCornerHit) {
                const auto endCorner = dragStartHit.target<BBoxCorner>();

                return moveBBoxCorner(bboxAtDragStart, endCorner, delta, anchor);
            } else {
                assert(0);
                return BBox3();
            }
        }

        // ScaleObjectsTool

        ScaleObjectsTool::ScaleObjectsTool(MapDocumentWPtr document) :
        Tool(false),
        m_document(document),
        m_toolPage(nullptr),
        m_resizing(false),
        m_anchorPos(AnchorPos::Opposite),
        m_bboxAtDragStart(),
        m_dragStartHit(Model::Hit::NoHit),
        m_dragCumulativeDelta(vec3::zero),
        m_proportionalAxes(ProportionalAxes::None()) {}
        
        ScaleObjectsTool::~ScaleObjectsTool() = default;

        const Model::Hit& ScaleObjectsTool::dragStartHit() const {
            return m_dragStartHit;
        }
        
        bool ScaleObjectsTool::applies() const {
            MapDocumentSPtr document = lock(m_document);
            return !document->selectedNodes().empty();
        }

        BackSide pickBackSideOfBox(const Ray3& pickRay, const Renderer::Camera& camera, const BBox3& box) {
            FloatType closestDistToRay = std::numeric_limits<FloatType>::max();
            FloatType bestDistAlongRay = std::numeric_limits<FloatType>::max();
            vec3 bestNormal;

            // idea is: find the closest point on an edge of the cube, belonging
            // to a face that's facing away from the pick ray.
            auto visitor = [&](const vec3& p0, const vec3& p1, const vec3& p2, const vec3& p3, const vec3& n){
                const FloatType cosAngle = dot(n, pickRay.direction);
                if (cosAngle >= 0.0 && cosAngle < 1.0) {
                    // the face is pointing away from the camera (or exactly perpendicular)
                    // but not equal to the camera direction (important for 2D views)

                    const vec3 points[] = {p0, p1, p2, p3};
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
            eachBBoxFace(box, visitor);

            // The hit point is the closest point on the pick ray to one of the edges of the face.
            // For face dragging, we'll project the pick ray onto the line through this point and having the face normal.
            assert(bestNormal != vec3::zero);

            BackSide result;
            result.distAlongRay = bestDistAlongRay;
            result.pickedSideNormal = bestNormal;
            return result;
        }

        void ScaleObjectsTool::pickBackSides(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) {
            // select back sides. Used for both 2D and 3D.
            if (pickResult.empty()) {
                const auto result = pickBackSideOfBox(pickRay, camera, bounds());

                // The hit point is the closest point on the pick ray to one of the edges of the face.
                // For face dragging, we'll project the pick ray onto the line through this point and having the face normal.
                assert(result.pickedSideNormal != vec3::zero);
                pickResult.addHit(Model::Hit(ScaleToolSideHit, result.distAlongRay, pickRay.pointAtDistance(result.distAlongRay), BBoxSide{result.pickedSideNormal}));
            }
        }

        void ScaleObjectsTool::pick2D(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) {
            const BBox3& myBounds = bounds();

            // origin in bbox
            if (myBounds.contains(pickRay.origin)) {
                return;
            }

            Model::PickResult localPickResult;

            // bbox corners in 2d views
            assert(camera.orthographicProjection());
            for (const BBoxEdge& edge : allEdges()) {
                const Edge3 points = pointsForBBoxEdge(myBounds, edge);

                // in 2d views, only use edges that are parallel to the camera
                if (parallel(points.direction(), vec3(camera.direction()))) {
                    // could figure out which endpoint is closer to camera, or just test both.
                    for (const vec3& point : vec3::List{points.start(), points.end()}) {
                        const FloatType dist = camera.pickPointHandle(pickRay, point, pref(Preferences::HandleRadius));
                        if (!Math::isnan(dist)) {
                            localPickResult.addHit(Model::Hit(ScaleToolEdgeHit, dist, pickRay.pointAtDistance(dist), edge));
                        }
                    }
                }
            }

            pickBackSides(pickRay, camera, localPickResult);

            auto hit = localPickResult.query().first();

            if (hit.isMatch()) {
                pickResult.addHit(hit);
            }
        }

        void ScaleObjectsTool::pick3D(const Ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) {
            const BBox3& myBounds = bounds();

            // origin in bbox
            if (myBounds.contains(pickRay.origin)) {
                return;
            }

            Model::PickResult localPickResult;

            // these handles only work in 3D.
            assert(camera.perspectiveProjection());

            // corners
            for (const BBoxCorner& corner : allCorners()) {
                const vec3 point = pointForBBoxCorner(myBounds, corner);

                // make the spheres for the corner handles slightly larger than the
                // cylinders of the edge handles, so they take priority where they overlap.
                const FloatType cornerRadius = pref(Preferences::HandleRadius) * 2.0;
                const FloatType dist = camera.pickPointHandle(pickRay, point, cornerRadius);
                if (!Math::isnan(dist)) {
                    localPickResult.addHit(Model::Hit(ScaleToolCornerHit, dist, pickRay.pointAtDistance(dist), corner));
                }
            }

            // edges
            for (const BBoxEdge& edge : allEdges()) {
                const Edge3 points = pointsForBBoxEdge(myBounds, edge);

                const FloatType dist = camera.pickLineSegmentHandle(pickRay, points, pref(Preferences::HandleRadius));
                if (!Math::isnan(dist)) {
                    localPickResult.addHit(Model::Hit(ScaleToolEdgeHit, dist, pickRay.pointAtDistance(dist), edge));
                }
            }

            // sides
            for (const BBoxSide& side : allSides()) {
                const auto poly = polygonForBBoxSide(myBounds, side);

                const FloatType dist = intersectPolygonWithRay(pickRay, poly.begin(), poly.end());
                if (!Math::isnan(dist)) {
                    localPickResult.addHit(Model::Hit(ScaleToolSideHit, dist, pickRay.pointAtDistance(dist), side));
                }
            }

            pickBackSides(pickRay, camera, localPickResult);

            auto hit = localPickResult.query().first();

            if (hit.isMatch()) {
                pickResult.addHit(hit);
            }
        }


        BBox3 ScaleObjectsTool::bounds() const {
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
                vec3 sideNormal = vec3::zero;
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
            
            auto visitor = [&](const vec3& p0, const vec3& p1, const vec3& p2, const vec3& p3, const vec3& n){
                const vec3 verts[4] = {p0, p1, p2, p3};
                
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

        static std::vector<BBoxSide> sidesWithOppositeSides(const std::vector<BBoxSide>& sides) {
            std::set<BBoxSide> result;
            for (const auto& side : sides) {
                result.insert(side);
                result.insert(oppositeSide(side));
            }

            return std::vector<BBoxSide>(result.begin(), result.end());
        }

        std::vector<Polygon3f> ScaleObjectsTool::polygonsHighlightedByDrag() const {
            std::vector<BBoxSide> sides;

            if (m_dragStartHit.type() == ScaleToolSideHit) {
                const auto side = m_dragStartHit.target<BBoxSide>();
                sides = {side};

                // Add additional highlights when Shift is pressed, to indicate the other axes that are being scaled
                // proportionally.
                for (size_t i = 0; i < 3; ++i) {
                    // Don't highlight `side` or its opposite
                    if (i == side.normal.firstComponent()) {
                        continue;
                    }

                    if (m_proportionalAxes.isAxisProportional(i)) {
                        // Highlight the + and - sides on this axis
                        vec3 side1;
                        side1[i] = 1.0;
                        sides.emplace_back(side1);

                        vec3 side2;
                        side2[i] = -1.0;
                        sides.emplace_back(side2);
                    }
                }
            } else if (m_dragStartHit.type() == ScaleToolEdgeHit) {
                const auto edge = m_dragStartHit.target<BBoxEdge>();
                sides = sidesForEdgeSelection(edge);
            } else if (m_dragStartHit.type() == ScaleToolCornerHit) {
                const auto corner = m_dragStartHit.target<BBoxCorner>();
                sides = sidesForCornerSelection(corner);
            } else {
                return {};
            }

            // When the anchor point is the center, highlight the opposite sides also.
            if (m_anchorPos == AnchorPos::Center) {
                sides = sidesWithOppositeSides(sides);
            }

            return polysForSides(bounds(), sides);
        }
        
        bool ScaleObjectsTool::hasDragSide() const {
            return dragSide().vertexCount() > 0;
        }

        Polygon3f ScaleObjectsTool::dragSide() const {
            if (m_dragStartHit.type() == ScaleToolSideHit) {
                const auto side = m_dragStartHit.target<BBoxSide>();
                return Polygon3f(polygonForBBoxSide(bounds(), side));
            }
                                                            
            return Polygon3f();
        }
        
        bool ScaleObjectsTool::hasDragEdge() const {
            return m_dragStartHit.type() == ScaleToolEdgeHit;
        }
        
        Edge3f ScaleObjectsTool::dragEdge() const {
            assert(hasDragEdge());
            auto whichEdge = m_dragStartHit.target<BBoxEdge>();
            return Edge3f(pointsForBBoxEdge(bounds(), whichEdge));
        }
        
        bool ScaleObjectsTool::hasDragCorner() const {
            return m_dragStartHit.type() == ScaleToolCornerHit;
        }
        
        vec3f ScaleObjectsTool::dragCorner() const {
            assert(hasDragCorner());
            auto whichCorner = m_dragStartHit.target<BBoxCorner>();
            return vec3f(pointForBBoxCorner(bounds(), whichCorner));
        }

        bool ScaleObjectsTool::hasDragAnchor() const {
            if (bounds().empty()) {
                return false;
            }

            const auto type = m_dragStartHit.type();
            return type == ScaleToolEdgeHit
                   || type == ScaleToolCornerHit
                   || type == ScaleToolSideHit;
        }

        vec3f ScaleObjectsTool::dragAnchor() const {
            if (m_anchorPos == AnchorPos::Center) {
                return vec3f(bounds().center());
            }

            if (m_dragStartHit.type() == ScaleToolSideHit) {
                const auto endSide = m_dragStartHit.target<BBoxSide>();
                const auto startSide = oppositeSide(endSide);

                return vec3f(centerForBBoxSide(bounds(), startSide));
            } else if (m_dragStartHit.type() == ScaleToolEdgeHit) {
                const auto endEdge = m_dragStartHit.target<BBoxEdge>();
                const auto startEdge = oppositeEdge(endEdge);

                const Edge3 startEdgeActual = pointsForBBoxEdge(bounds(), startEdge);

                return vec3f(startEdgeActual.center());
            } else if (m_dragStartHit.type() == ScaleToolCornerHit) {
                const auto endCorner = m_dragStartHit.target<BBoxCorner>();
                const auto startCorner = oppositeCorner(endCorner);

                const auto startCornerActual = pointForBBoxCorner(bounds(), startCorner);
                return vec3f(startCornerActual);
            }

            assert(0);
            return vec3f::zero;
        }

        BBox3 ScaleObjectsTool::bboxAtDragStart() const {
            ensure(m_resizing, "bboxAtDragStart() can only be called while resizing");
            return m_bboxAtDragStart;
        }

        vec3::List ScaleObjectsTool::cornerHandles() const {
            if (bounds().empty()) {
                return {};
            }

            vec3::List result;
            result.reserve(8);
            auto op = [&](const vec3& point) {
                result.push_back(point);
            };
            eachBBoxVertex(bounds(), op);
            return result;
        }

        void ScaleObjectsTool::updatePickedHandle(const Model::PickResult &pickResult) {
            const Model::Hit& hit = pickResult.query().type(ScaleToolSideHit | ScaleToolEdgeHit | ScaleToolCornerHit).occluded().first();

            // extract the highlighted handle from the hit here, and only refresh views if it changed
            if (hit.type() == ScaleToolSideHit && m_dragStartHit.type() == ScaleToolSideHit) {
                if (hit.target<BBoxSide>() == m_dragStartHit.target<BBoxSide>()) {
                    return;
                }
            } else if (hit.type() == ScaleToolEdgeHit && m_dragStartHit.type() == ScaleToolEdgeHit) {
                if (hit.target<BBoxEdge>() == m_dragStartHit.target<BBoxEdge>()) {
                    return;
                }
            } else if (hit.type() == ScaleToolCornerHit && m_dragStartHit.type() == ScaleToolCornerHit) {
                if (hit.target<BBoxCorner>() == m_dragStartHit.target<BBoxCorner>()) {
                    return;
                }
            }

            // hack for highlighting on mouseover
            m_dragStartHit = hit;

            refreshViews();
        }

        void ScaleObjectsTool::setAnchorPos(const AnchorPos pos) {
            m_anchorPos = pos;
        }

        AnchorPos ScaleObjectsTool::anchorPos() const {
            return m_anchorPos;
        }

        void ScaleObjectsTool::setProportionalAxes(ProportionalAxes proportionalAxes) {
            m_proportionalAxes = proportionalAxes;
        }

        ProportionalAxes ScaleObjectsTool::proportionalAxes() const {
            return m_proportionalAxes;
        }

        void ScaleObjectsTool::startScaleWithHit(const Model::Hit& hit) {
            ensure(hit.isMatch(), "must start with matching hit");
            ensure(hit.type() == ScaleToolCornerHit
                   || hit.type() == ScaleToolEdgeHit
                   || hit.type() == ScaleToolSideHit, "wrong hit type");
            ensure(!m_resizing, "must not be resizing already");

            m_bboxAtDragStart = bounds();
            m_dragStartHit = hit;
            m_dragCumulativeDelta = vec3::zero;

            MapDocumentSPtr document = lock(m_document);
            document->beginTransaction("Scale Objects");
            m_resizing = true;
        }

        void ScaleObjectsTool::scaleByDelta(const vec3 &delta) {
            ensure(m_resizing, "must be resizing already");

            m_dragCumulativeDelta += delta;

            MapDocumentSPtr document = lock(m_document);

            const auto newBox = moveBBoxForHit(m_bboxAtDragStart, m_dragStartHit, m_dragCumulativeDelta,
                                               m_proportionalAxes, m_anchorPos);

            if (!newBox.empty()) {
                document->scaleObjects(bounds(), newBox);
            }
        }

        void ScaleObjectsTool::commitScale() {
            MapDocumentSPtr document = lock(m_document);
            if (isNull(m_dragCumulativeDelta)) {
                document->cancelTransaction();
            } else {
                document->commitTransaction();
            }
            m_resizing = false;
        }

        void ScaleObjectsTool::cancelScale() {
            MapDocumentSPtr document = lock(m_document);
            document->cancelTransaction();
            m_resizing = false;
        }

        wxWindow* ScaleObjectsTool::doCreatePage(wxWindow* parent) {
            assert(m_toolPage == nullptr);
            m_toolPage = new ScaleObjectsToolPage(parent, m_document);
            return m_toolPage;
        }
    }
}
