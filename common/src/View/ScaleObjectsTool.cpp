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

#include "ScaleObjectsTool.h"

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

namespace TrenchBroom {
    namespace View {
        const Model::Hit::HitType ScaleObjectsTool::ScaleToolFaceHit = Model::Hit::freeHitType();
        const Model::Hit::HitType ScaleObjectsTool::ScaleToolEdgeHit = Model::Hit::freeHitType();
        const Model::Hit::HitType ScaleObjectsTool::ScaleToolCornerHit = Model::Hit::freeHitType();
        
        static const std::vector<BBoxSide> AllSides() {
            std::vector<BBoxSide> result;
            result.reserve(6);
            
            const BBox3 box{{-1, -1, -1}, {1, 1, 1}};
            auto op = [&](const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& p3, const Vec3& normal) {
                result.push_back(BBoxSide(normal));
            };
            eachBBoxFace(box, op);
            
            assert(result.size() == 6);
            return result;
        };
        
        static Vec3 normalForBBoxSide(const BBoxSide side) {
            return side.normal;
        }
                
        static std::vector<BBoxEdge> AllEdges() {
            std::vector<BBoxEdge> result;
            result.reserve(12);
            
            const BBox3 box{{-1, -1, -1}, {1, 1, 1}};
            auto op = [&](const Vec3& p0, const Vec3& p1) {
                result.push_back(BBoxEdge(p0, p1));
            };
            eachBBoxEdge(box, op);
            
            assert(result.size() == 12);
            return result;
        }
        
        static std::vector<BBoxCorner> AllCorners() {
            std::vector<BBoxCorner> result;
            result.reserve(8);
            
            const BBox3 box{{-1, -1, -1}, {1, 1, 1}};
            auto op = [&](const Vec3& point) {
                result.push_back(BBoxCorner(point));
            };
            eachBBoxVertex(box, op);
            
            assert(result.size() == 8);
            return result;
        }
        
        static Vec3 pointForBBoxCorner(const BBox3& box, const BBoxCorner corner) {
            Vec3 res;
            for (size_t i = 0; i < 3; ++i) {
                assert(corner.corner[i] == 1.0 || corner.corner[i] == -1.0);
                
                res[i] = (corner.corner[i] == 1.0) ? box.max[i] : box.min[i];
            }
            return res;
        }
        
        static Vec3 normalForBBoxCorner(const BBoxCorner corner) {
            // HACK: Due to the representation of corners, all we need to do is normalize it
            return corner.corner.normalized();
        }
        
        static BBoxCorner oppositeCorner(const BBoxCorner corner) {
            return BBoxCorner(Vec3(-corner.corner.x(),
                                   -corner.corner.y(),
                                   -corner.corner.z()));
        }
        
        static BBoxEdge oppositeEdge(const BBoxEdge edge) {
            return BBoxEdge(oppositeCorner(BBoxCorner(edge.point0)).corner,
                            oppositeCorner(BBoxCorner(edge.point1)).corner);
        }
        
        static Edge3 pointsForBBoxEdge(const BBox3& box, const BBoxEdge edge) {
            return Edge3(pointForBBoxCorner(box, BBoxCorner(edge.point0)),
                         pointForBBoxCorner(box, BBoxCorner(edge.point1)));
        }
        
        static Vec3 normalForBBoxEdge(const BBoxEdge edge) {
            const Vec3 corner0Normal = edge.point0;
            const Vec3 corner1Normal = edge.point1;
            return (corner0Normal + corner1Normal).normalized();
        }

        static Polygon3 polygonForBBoxSide(const BBox3& box, const BBoxSide side) {
            const Vec3 wantedNormal = normalForBBoxSide(side);
            
            Polygon3 res;
            auto visitor = [&](const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& p3, const Vec3& n){
                if (n == wantedNormal) {
                    const Polygon3 poly {p0, p1, p2, p3};
                    res = poly;
                }
            };
            eachBBoxFace(box, visitor);
            
            assert(res.vertexCount() == 4);
            return res;
        }
        
        static BBox3 moveBBoxFace(const BBox3& in, const BBoxSide side, const Vec3& delta) {
            const Vec3 n = side.normal;
            
            if (n == Vec3::PosX) return BBox3(in.min                        , in.max + Vec3(delta.x(), 0, 0));
            if (n == Vec3::PosY) return BBox3(in.min                        , in.max + Vec3(0, delta.y(), 0));
            if (n == Vec3::PosZ) return BBox3(in.min                        , in.max + Vec3(0, 0, delta.z()));
                
            if (n == Vec3::NegX) return BBox3(in.min + Vec3(delta.x(), 0, 0), in.max                        );
            if (n == Vec3::NegY) return BBox3(in.min + Vec3(0, delta.y(), 0), in.max                        );
            if (n == Vec3::NegZ) return BBox3(in.min + Vec3(0, 0, delta.z()), in.max                        );
        
            assert(0);
            return {};
        }
        
        static BBox3 moveBBoxCorner(const BBox3& in, const BBoxCorner corner, const Vec3& delta) {
            const BBoxCorner opposite = oppositeCorner(corner);
            
            const Vec3::List newVerts {
                pointForBBoxCorner(in, opposite),
                pointForBBoxCorner(in, corner) + delta,
            };
            
            return BBox3(newVerts);
        }
        
        static BBox3 moveBBoxEdge(const BBox3& in, const BBoxEdge edge, const Vec3& delta) {
            const BBoxEdge opposite = oppositeEdge(edge);
            
            const Edge3 oppositePoints = pointsForBBoxEdge(in, opposite);
            const Edge3 edgePoints = pointsForBBoxEdge(in, edge);
            
            const Vec3::List newVerts {
                oppositePoints.start(),
                oppositePoints.end(),
                edgePoints.start() + delta,
                edgePoints.end() + delta,
            };
            
            return BBox3(newVerts);
        }
        
        ScaleObjectsTool::ScaleObjectsTool(MapDocumentWPtr document) :
        Tool(false),
        m_document(document),
        m_toolPage(nullptr),
        m_dragStartHit(Model::Hit::NoHit),
        m_resizing(false)
        {
            bindObservers();
        }
        
        ScaleObjectsTool::~ScaleObjectsTool() {
            unbindObservers();
        }
        
        bool ScaleObjectsTool::applies() const {
            MapDocumentSPtr document = lock(m_document);
            return !document->selectedNodes().empty();
        }
        
        Model::Hit ScaleObjectsTool::pick2D(const Ray3& pickRay, const Renderer::Camera& camera, const Model::PickResult& pickResult) {
            return Model::Hit::NoHit;
        }
        
        Model::Hit ScaleObjectsTool::pick3D(const Ray3& pickRay, const Renderer::Camera& camera, const Model::PickResult& pickResult) {
            const BBox3& myBounds = bounds();
            
            // origin in bbox
            if (myBounds.contains(pickRay.origin))
                return Model::Hit::NoHit;

            Model::PickResult localPickResult;

            // corners
            for (const BBoxCorner& corner : AllCorners()) {
                const Vec3 point = pointForBBoxCorner(myBounds, corner);
                
                // make the spheres for the corner handles slightly larger than the
                // cylinders of the edge handles, so they take priority where they overlap.
                const FloatType cornerRadius = pref(Preferences::HandleRadius) + 0.1;
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
            for (const BBoxSide side : AllSides()) {
                const auto poly = polygonForBBoxSide(myBounds, side);
                
                const FloatType dist = intersectPolygonWithRay(pickRay, poly.begin(), poly.end());
                if (!Math::isnan(dist)) {
                    localPickResult.addHit(Model::Hit(ScaleToolFaceHit, dist, pickRay.pointAtDistance(dist), side));
                }
            }

            // select back faces
            if (localPickResult.empty()) {
                
                FloatType closestDistToRay = std::numeric_limits<FloatType>::max();
                FloatType bestDistAlongRay = std::numeric_limits<FloatType>::max();
                Vec3 bestNormal;
                
                // idea is: find the closest point on an edge of the cube, belonging
                // to a face that's facing away from the pick ray.
                auto visitor = [&](const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& p3, const Vec3& n){
                    if (n.dot(pickRay.direction) > 0.0) {
                        // the face is pointing away from the camera
                        
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
                
                assert(bestNormal != Vec3::Null);
                localPickResult.addHit(Model::Hit(ScaleToolFaceHit, bestDistAlongRay, pickRay.pointAtDistance(bestDistAlongRay), BBoxSide{bestNormal}));
                
                //std::cout << "closest: " << pickRay.pointAtDistance(bestDistAlongRay) << "\n";
            }
            
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
            
            return hit;
        }
        
        BBox3 ScaleObjectsTool::bounds() const {
            MapDocumentSPtr document = lock(m_document);
            return document->selectionBounds();
        }
        
        // used for rendering
        
        bool ScaleObjectsTool::hasDragPolygon() const {
            return dragPolygon().vertexCount() > 0;
        }

        Polygon3 ScaleObjectsTool::dragPolygon() const {
            if (m_dragStartHit.type() == ScaleToolFaceHit) {
                const auto side = m_dragStartHit.target<BBoxSide>();
                return polygonForBBoxSide(bounds(), side);
            }
                                                            
            return Polygon3();
        }
        
        bool ScaleObjectsTool::hasDragEdge() const {
            return m_dragStartHit.type() == ScaleToolEdgeHit;
        }
        
        Edge3 ScaleObjectsTool::dragEdge() const {
            assert(hasDragEdge());
            auto whichEdge = m_dragStartHit.target<BBoxEdge>();
            return pointsForBBoxEdge(bounds(), whichEdge);
        }
        
        bool ScaleObjectsTool::hasDragCorner() const {
            return m_dragStartHit.type() == ScaleToolCornerHit;
        }
        
        Vec3 ScaleObjectsTool::dragCorner() const {
            assert(hasDragCorner());
            auto whichCorner = m_dragStartHit.target<BBoxCorner>();
            return pointForBBoxCorner(bounds(), whichCorner);
        }

        // for rendering sheared bbox
        BBox3 ScaleObjectsTool::bboxAtDragStart() const {
            return m_bboxAtDragStart;
        }
        Mat4x4 ScaleObjectsTool::bboxShearMatrix() const {
            const BBoxSide side = m_dragStartHit.target<BBoxSide>();
            
            return shearBBoxMatrix(m_bboxAtDragStart,
                                   side.normal,
                                   m_totalDelta);
        }
        
        bool ScaleObjectsTool::isShearing() const {
            return m_isShearing;
        }

        Vec3::List ScaleObjectsTool::cornerHandles() const {
            Vec3::List result;
            result.reserve(8);
            auto op = [&](const Vec3& point) {
                result.push_back(point);
            };
            eachBBoxVertex(bounds(), op);
            return result;
        }
        
//        Vec3 ScaleObjectsTool::dragPolygonNormal() const {
//            Plane3 plane;
//            if (!getPlane(m_dragPolygon.begin(), m_dragPolygon.end(), plane))
//                return Vec3(0,0,0);
//
//            return plane.normal;
//        }
        
      void ScaleObjectsTool::updateDragFaces(const Model::PickResult& pickResult) {          
            const Model::Hit& hit = pickResult.query().type(ScaleToolFaceHit | ScaleToolEdgeHit | ScaleToolCornerHit).occluded().first();
//
//
//            auto newDragFaces = getDragPolygon(hit);
//            //if (newDragFaces != m_dragPolygon)
//                refreshViews();
//
//            //m_dragPolygon = newDragFaces;
          
      }
        
//        BBoxSide ScaleObjectsTool::getDragPolygon(const Model::Hit& hit) const {
//            if (!hit.isMatch()) return Polygon3();
//
//            const BBoxSide index = hit.target<BBoxSide>();
//            printf("hit out: %d\n", static_cast<int>(index));
//
//            return polygonForBBoxSide(bounds(), index);
//        }
        
//        class ScaleObjectsTool::MatchFaceBoundary {
//        private:
//            const Model::BrushFace* m_reference;
//        public:
//            MatchFaceBoundary(const Model::BrushFace* reference) :
//            m_reference(reference) {
//                ensure(m_reference != nullptr, "reference is null");
//            }
//
//            bool operator()(Model::BrushFace* face) const {
//                return face != m_reference && face->boundary().equals(m_reference->boundary());
//            }
//        };
        
        Model::BrushFaceList ScaleObjectsTool::collectDragFaces(const Model::Hit& hit) const {
            assert(hit.isMatch());
//            assert(hit.type() == ScaleHit2D || hit.type() == ScaleHit3D);

            Model::BrushFaceList result;
//            if (hit.type() == ScaleHit2D) {
//                const Model::BrushFaceList& faces = hit.target<Model::BrushFaceList>();
//                assert(!faces.empty());
//                VectorUtils::append(result, faces);
//                VectorUtils::append(result, collectDragFaces(faces[0]));
//                if (faces.size() > 1)
//                    VectorUtils::append(result, collectDragFaces(faces[1]));
//            } else {
//                Model::BrushFace* face = hit.target<Model::BrushFace*>();
//                result.push_back(face);
//                VectorUtils::append(result, collectDragFaces(face));
//            }

            return result;
        }
        
        Model::BrushFaceList ScaleObjectsTool::collectDragFaces(Model::BrushFace* face) const {
            return {};
//            Model::CollectMatchingBrushFacesVisitor<MatchFaceBoundary> visitor((MatchFaceBoundary(face)));
//
//            MapDocumentSPtr document = lock(m_document);
//            const Model::NodeList& nodes = document->selectedNodes().nodes();
//            Model::Node::accept(std::begin(nodes), std::end(nodes), visitor);
//            return visitor.faces();
        }
        
        bool ScaleObjectsTool::beginResize(const Model::PickResult& pickResult, const bool split) {
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
            
//            m_dragSide = hit.target<BBoxSide>();
//            std::cout << "initial hitpoint: " << hit.hitPoint() << " drag side: " << m_dragSide.normal.asString() << "\n";
            
         //   m_splitBrushes = split;
            
            MapDocumentSPtr document = lock(m_document);
            document->beginTransaction("Resize Brushes");
            m_resizing = true;
            return true;
        }
        
        Vec3 makeNormalOnGrid(Vec3 normal) {
            Vec3 res = normal;
            for (size_t i = 0; i < 3; ++i) {
                if (res[i] < 0)
                    res[i] = -1;
                if (res[i] > 0)
                    res[i] = 1;
            }
            return res;
        }
        
        bool ScaleObjectsTool::resize(const Ray3& pickRay, const Renderer::Camera& camera, const bool proportional, const bool vertical, const bool shear) {
//            assert(!m_dragFaces.empty());
//            assert(hasDragPolygon());
//
//            Model::BrushFace* dragFace = m_dragFaces.front();
            
            printf("proportional %d vertical %d shear %d\n",
                   (int)proportional, (int)vertical, (int)shear);
            
            Vec3 dragObjNormal;
            if (m_dragStartHit.type() == ScaleToolFaceHit) {
                dragObjNormal = normalForBBoxSide(m_dragStartHit.target<BBoxSide>());
                std::cout << "ScaleObjectsTool::resize with face normal " << dragObjNormal.asString() << "\n";
            } else if (m_dragStartHit.type() == ScaleToolEdgeHit) {
                dragObjNormal = normalForBBoxEdge(m_dragStartHit.target<BBoxEdge>());
                std::cout << "ScaleObjectsTool::resize with edge normal " << dragObjNormal.asString() << "\n";
            } else if (m_dragStartHit.type() == ScaleToolCornerHit) {
                dragObjNormal = normalForBBoxCorner(m_dragStartHit.target<BBoxCorner>());
                std::cout << "ScaleObjectsTool::resize with corner normal " << dragObjNormal.asString() << "\n";
            } else
                assert(0);
            
            std::cout << "ScaleObjectsTool::resize with start bbox: "
                        << m_bboxAtDragStart.min.asString() << "->"
                        << m_bboxAtDragStart.max.asString() << "\n";
//                        << " side: " << m_dragSide.normal.asString() << "\n";
            
            const Ray3::LineDistance distance = pickRay.distanceToLine(m_dragOrigin, dragObjNormal);
            if (distance.parallel)
                return true;

            const FloatType dragDist = distance.lineDistance;

            MapDocumentSPtr document = lock(m_document);
            const View::Grid& grid = document->grid();
            
            // FIXME: Do makeNormalOnGrid in a cleaner way
            const Vec3 relativeFaceDelta = grid.snap(dragDist) * makeNormalOnGrid(dragObjNormal);
            //const Vec3 absoluteFaceDelta = grid.moveDelta(dragFace, faceNormal * dragDist);

            const Vec3 faceDelta = relativeFaceDelta;//selectDelta(relativeFaceDelta, absoluteFaceDelta, dragDist);
            
            
            m_isShearing = shear;
            
            if (!shear) {
                BBox3 newBbox;
                if (m_dragStartHit.type() == ScaleToolFaceHit) {
                    const auto side = m_dragStartHit.target<BBoxSide>();
                    newBbox = moveBBoxFace(m_bboxAtDragStart, side, faceDelta);
                } else if (m_dragStartHit.type() == ScaleToolEdgeHit) {
                    const auto edge = m_dragStartHit.target<BBoxEdge>();
                    newBbox = moveBBoxEdge(m_bboxAtDragStart, edge, faceDelta);
                } else if (m_dragStartHit.type() == ScaleToolCornerHit) {
                    const auto corner = m_dragStartHit.target<BBoxCorner>();
                    newBbox = moveBBoxCorner(m_bboxAtDragStart, corner, faceDelta);
                } else
                    assert(0);

                std::cout << "ScaleObjectsTool new bbox: "
                << newBbox.min.asString() << "->"
                << newBbox.max.asString() << "\n";
                
                std::cout << "make resize with delta: " << faceDelta << "\n";
                if (newBbox.empty()) {
                    std::cout << "skipping because empty\n";
                } else {
                    if (document->scaleObjectsBBox(bounds(), newBbox)) {
                        m_totalDelta += faceDelta;
                        //m_dragOrigin += faceDelta;
                    }
                }
                
            } else {
                // shear
                if (m_dragStartHit.type() == ScaleToolFaceHit) {
                    const BBoxSide side = m_dragStartHit.target<BBoxSide>();
                    
                    const auto poly = polygonForBBoxSide(bounds(), side);
                    const Vec3 planeAnchor = poly.vertices().front();
                    
                    // get the point where the pick ray intersects the plane being dragged.
                    const Vec3 rayHit = pickRay.pointAtDistance(pickRay.intersectWithPlane(side.normal, planeAnchor));
                    
                    std::cout << "make shear with rayHit: " << rayHit << "\n";

                    //m_dragOrigin = rayHit;
                    
                    Vec3 delta = rayHit - m_dragOrigin;
                    delta = grid.snap(delta);
                    if (vertical) {
                        delta[0] = 0;
                        delta[1] = 0;
                    } else {
                        delta[2] = 0;
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
        
        void ScaleObjectsTool::commitResize() {
            MapDocumentSPtr document = lock(m_document);
            if (m_totalDelta.null())
                document->cancelTransaction();
            else
                document->commitTransaction();
//            m_dragFaces.clear();
            m_resizing = false;
        }
        
        void ScaleObjectsTool::cancelResize() {
            MapDocumentSPtr document = lock(m_document);
            document->cancelTransaction();
//            m_dragFaces.clear();
            m_resizing = false;
        }
        
        void ScaleObjectsTool::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->nodesWereAddedNotifier.addObserver(this, &ScaleObjectsTool::nodesDidChange);
            document->nodesWillChangeNotifier.addObserver(this, &ScaleObjectsTool::nodesDidChange);
            document->nodesWillBeRemovedNotifier.addObserver(this, &ScaleObjectsTool::nodesDidChange);
            document->selectionDidChangeNotifier.addObserver(this, &ScaleObjectsTool::selectionDidChange);
        }
        
        void ScaleObjectsTool::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->nodesWereAddedNotifier.removeObserver(this, &ScaleObjectsTool::nodesDidChange);
                document->nodesWillChangeNotifier.removeObserver(this, &ScaleObjectsTool::nodesDidChange);
                document->nodesWillBeRemovedNotifier.removeObserver(this, &ScaleObjectsTool::nodesDidChange);
                document->selectionDidChangeNotifier.removeObserver(this, &ScaleObjectsTool::selectionDidChange);
            }
        }
        
        void ScaleObjectsTool::nodesDidChange(const Model::NodeList& nodes) {
//            if (!m_resizing)
//                m_dragFaces.clear();
        }
        
        void ScaleObjectsTool::selectionDidChange(const Selection& selection) {
//            if (!m_resizing)
//                m_dragFaces.clear();
        }

        wxWindow* ScaleObjectsTool::doCreatePage(wxWindow* parent) {
            assert(m_toolPage == nullptr);
            m_toolPage = new ScaleObjectsToolPage(parent, m_document);
            return m_toolPage;
        }
    }
}
