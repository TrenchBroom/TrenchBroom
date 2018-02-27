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
        const Model::Hit::HitType ScaleObjectsTool::ScaleHit2D = Model::Hit::freeHitType();
        const Model::Hit::HitType ScaleObjectsTool::ScaleHit3D = Model::Hit::freeHitType();
        
        static const std::array<BBoxSide, 6> AllSides {
            BBoxSide::PosX,
            BBoxSide::NegX,
            BBoxSide::PosY,
            BBoxSide::NegY,
            BBoxSide::PosZ,
            BBoxSide::NegZ
        };
        
        static Vec3 normalForBBoxSide(const BBoxSide side) {
            switch (side) {
                case BBoxSide::PosX: return Vec3::PosX;
                case BBoxSide::NegX: return Vec3::NegX;
                case BBoxSide::PosY: return Vec3::PosY;
                case BBoxSide::NegY: return Vec3::NegY;
                case BBoxSide::PosZ: return Vec3::PosZ;
                case BBoxSide::NegZ: return Vec3::NegZ;
            }
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
            switch (side) {
                case BBoxSide::PosX: return BBox3(in.min                        , in.max + Vec3(delta.x(), 0, 0));
                case BBoxSide::PosY: return BBox3(in.min                        , in.max + Vec3(0, delta.y(), 0));
                case BBoxSide::PosZ: return BBox3(in.min                        , in.max + Vec3(0, 0, delta.z()));
                    
                case BBoxSide::NegX: return BBox3(in.min + Vec3(delta.x(), 0, 0), in.max                        );
                case BBoxSide::NegY: return BBox3(in.min + Vec3(0, delta.y(), 0), in.max                        );
                case BBoxSide::NegZ: return BBox3(in.min + Vec3(0, 0, delta.z()), in.max                        );
            }
        }
        
        ScaleObjectsTool::ScaleObjectsTool(MapDocumentWPtr document) :
        Tool(false),
        m_document(document),
        m_toolPage(nullptr),
        m_resizing(false) {
            bindObservers();
        }
        
        ScaleObjectsTool::~ScaleObjectsTool() {
            unbindObservers();
        }
        
        bool ScaleObjectsTool::applies() const {
            MapDocumentSPtr document = lock(m_document);
            return !document->selectedNodes().empty();
        }
        
        Model::Hit ScaleObjectsTool::pick2D(const Ray3& pickRay, const Model::PickResult& pickResult) {
            return Model::Hit::NoHit;
        }
        
        Model::Hit ScaleObjectsTool::pick3D(const Ray3& pickRay, const Model::PickResult& pickResult) {
            const BBox3& myBounds = bounds();
            
            // origin in bbox
            if (myBounds.contains(pickRay.origin))
                return Model::Hit::NoHit;

            FloatType bestDist = Math::nan<FloatType>();
            BBoxSide bestIndex;
            
            //printf("testing polys:\n");
            for (const BBoxSide side : AllSides) {
                const auto poly = polygonForBBoxSide(myBounds, side);
                
                const FloatType dist = intersectPolygonWithRay(pickRay, poly.begin(), poly.end());
                //printf("    dist %f\n", dist);
                
                if (!isnan(dist)
                    && (isnan(bestDist) || dist < bestDist)) {
                    
                    //printf("    hit in %d\n", static_cast<int>(side));
                    bestIndex = side;
                    bestDist = dist;
                }
            }
            printf("\n");
            
            if (!isnan(bestDist)) {
                return Model::Hit(ScaleHit3D, bestDist, pickRay.pointAtDistance(bestDist), bestIndex);
            }
            
            return Model::Hit::NoHit;
        }
        
        BBox3 ScaleObjectsTool::bounds() const {
            MapDocumentSPtr document = lock(m_document);
            return document->selectionBounds();
        }

        bool ScaleObjectsTool::hasDragPolygon() const {
            return m_resizing;
        }

        Polygon3 ScaleObjectsTool::dragPolygon() const {
            assert(m_resizing);
            return polygonForBBoxSide(m_bboxAtDragStart, m_dragSide);
        }

//        Vec3 ScaleObjectsTool::dragPolygonNormal() const {
//            Plane3 plane;
//            if (!getPlane(m_dragPolygon.begin(), m_dragPolygon.end(), plane))
//                return Vec3(0,0,0);
//
//            return plane.normal;
//        }
        
      void ScaleObjectsTool::updateDragFaces(const Model::PickResult& pickResult) {          
//            const Model::Hit& hit = pickResult.query().type(ScaleHit2D | ScaleHit3D).occluded().first();
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
            const Model::Hit& hit = pickResult.query().type(ScaleHit2D | ScaleHit3D).occluded().first();
            if (!hit.isMatch())
                return false;
            
            m_dragSide = hit.target<BBoxSide>();
            std::cout << "initial hitpoint: " << hit.hitPoint() << " drag side: " << static_cast<int>(m_dragSide) << "\n";
            
            m_bboxAtDragStart = bounds();
            
            m_dragOrigin = hit.hitPoint();
            m_totalDelta = Vec3::Null;
         //   m_splitBrushes = split;
            
            MapDocumentSPtr document = lock(m_document);
            document->beginTransaction("Resize Brushes");
            m_resizing = true;
            return true;
        }
        
        bool ScaleObjectsTool::resize(const Ray3& pickRay, const Renderer::Camera& camera) {
//            assert(!m_dragFaces.empty());
            assert(hasDragPolygon());
//
//            Model::BrushFace* dragFace = m_dragFaces.front();
            const Vec3 faceNormal = normalForBBoxSide(m_dragSide);
//
            std::cout << "ScaleObjectsTool::resize with start bbox: "
                        << m_bboxAtDragStart.min.asString() << "->"
                        << m_bboxAtDragStart.max.asString()
                        << " side: " << static_cast<int>(m_dragSide) << "\n";
            
            const Ray3::LineDistance distance = pickRay.distanceToLine(m_dragOrigin, faceNormal);
            if (distance.parallel)
                return true;

            const FloatType dragDist = distance.lineDistance;

            MapDocumentSPtr document = lock(m_document);
            const View::Grid& grid = document->grid();
            const Vec3 relativeFaceDelta = grid.snap(dragDist) * faceNormal;
            //const Vec3 absoluteFaceDelta = grid.moveDelta(dragFace, faceNormal * dragDist);

            const Vec3 faceDelta = relativeFaceDelta;//selectDelta(relativeFaceDelta, absoluteFaceDelta, dragDist);            
            const BBox3 newBbox = moveBBoxFace(m_bboxAtDragStart, m_dragSide, faceDelta);
            
            std::cout << "ScaleObjectsTool new bbox: "
                << newBbox.min.asString() << "->"
                << newBbox.max.asString() << "\n";
            
            std::cout << "make resize with delta: " << faceDelta << "\n";
            if (document->scaleObjectsBBox(bounds(), newBbox)) {
                m_totalDelta += faceDelta;
                //m_dragOrigin += faceDelta;
            }
//                if (document->resizeBrushes(dragFaceDescriptors(), faceDelta)) {
//                    m_totalDelta += faceDelta;
//                    m_dragOrigin += faceDelta;
//                }
            
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
