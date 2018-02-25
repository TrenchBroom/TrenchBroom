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

namespace TrenchBroom {
    namespace View {
        const Model::Hit::HitType ScaleObjectsTool::ScaleHit2D = Model::Hit::freeHitType();
        const Model::Hit::HitType ScaleObjectsTool::ScaleHit3D = Model::Hit::freeHitType();
        
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
            
            // gather polygons
            std::vector<Polygon3> polys;
            auto visitor = [&](const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& p3, const Vec3& n){
                const Polygon3 poly {p0, p1, p2, p3};
                polys.push_back(poly);
            };
            eachBBoxFace(myBounds, visitor);
            
            printf("testing polys:\n");
            for (const auto& poly : polys) {
                const FloatType dist = intersectPolygonWithRay(pickRay, poly.begin(), poly.end());
                printf("    dist %f\n", dist);
            }
            printf("\n");
            
            //return Model::Hit(ScaleHit3D, distance, pickRay.pointAtDistance(distance), nullptr);
            
            return Model::Hit::NoHit;
        }
        
        BBox3 ScaleObjectsTool::bounds() const {
            MapDocumentSPtr document = lock(m_document);
            return document->selectionBounds();
        }
        
        FloatType ScaleObjectsTool::intersectWithRay(const Ray3& ray) const {

            
            return 0;
        }
        
        bool ScaleObjectsTool::hasDragFaces() const {
            return false;
        }
        
        const Model::BrushFaceList& ScaleObjectsTool::dragFaces() const {
            return {};
        }
        
        void ScaleObjectsTool::updateDragFaces(const Model::PickResult& pickResult) {
//            const Model::Hit& hit = pickResult.query().type(ResizeHit2D | ResizeHit3D).occluded().first();
//            Model::BrushFaceList newDragFaces = getDragFaces(hit);
//            if (newDragFaces != m_dragFaces)
//                refreshViews();
//
//            using std::swap;
//            swap(m_dragFaces, newDragFaces);
        }
        
        Model::BrushFaceList ScaleObjectsTool::getDragFaces(const Model::Hit& hit) const {
            return !hit.isMatch() ? Model::EmptyBrushFaceList : collectDragFaces(hit);
        }
        
        class ScaleObjectsTool::MatchFaceBoundary {
        private:
            const Model::BrushFace* m_reference;
        public:
            MatchFaceBoundary(const Model::BrushFace* reference) :
            m_reference(reference) {
                ensure(m_reference != nullptr, "reference is null");
            }
            
            bool operator()(Model::BrushFace* face) const {
                return face != m_reference && face->boundary().equals(m_reference->boundary());
            }
        };
        
        Model::BrushFaceList ScaleObjectsTool::collectDragFaces(const Model::Hit& hit) const {
            assert(hit.isMatch());
//            assert(hit.type() == ResizeHit2D || hit.type() == ResizeHit3D);
            
            Model::BrushFaceList result;
//            if (hit.type() == ResizeHit2D) {
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
            Model::CollectMatchingBrushFacesVisitor<MatchFaceBoundary> visitor((MatchFaceBoundary(face)));
            
            MapDocumentSPtr document = lock(m_document);
            const Model::NodeList& nodes = document->selectedNodes().nodes();
            Model::Node::accept(std::begin(nodes), std::end(nodes), visitor);
            return visitor.faces();
        }
        
        bool ScaleObjectsTool::beginResize(const Model::PickResult& pickResult, const bool split) {
//            const Model::Hit& hit = pickResult.query().type(ResizeHit2D | ResizeHit3D).occluded().first();
//            if (!hit.isMatch())
                return false;
            
            m_dragOrigin = Vec3::Null;//hit.hitPoint();
            m_totalDelta = Vec3::Null;
         //   m_splitBrushes = split;
            
            MapDocumentSPtr document = lock(m_document);
            document->beginTransaction("Resize Brushes");
            m_resizing = true;
            return true;
        }
        
        bool ScaleObjectsTool::resize(const Ray3& pickRay, const Renderer::Camera& camera) {
//            assert(!m_dragFaces.empty());
//
//            Model::BrushFace* dragFace = m_dragFaces.front();
//            const Vec3& faceNormal = dragFace->boundary().normal;
//
//            const Ray3::LineDistance distance = pickRay.distanceToLine(m_dragOrigin, faceNormal);
//            if (distance.parallel)
//                return true;
//
//            const FloatType dragDist = distance.lineDistance;
//
//            MapDocumentSPtr document = lock(m_document);
//            const View::Grid& grid = document->grid();
//            const Vec3 relativeFaceDelta = grid.snap(dragDist) * faceNormal;
//            const Vec3 absoluteFaceDelta = grid.moveDelta(dragFace, faceNormal * dragDist);
//
//            const Vec3 faceDelta = selectDelta(relativeFaceDelta, absoluteFaceDelta, dragDist);
//            if (faceDelta.null())
//                return true;
            
//            if (m_splitBrushes) {
//                if (splitBrushes(faceDelta)) {
//                    m_totalDelta += faceDelta;
//                    m_dragOrigin += faceDelta;
//                    m_splitBrushes = false;
//                }
//            } else {
//                if (document->resizeBrushes(dragFaceDescriptors(), faceDelta)) {
//                    m_totalDelta += faceDelta;
//                    m_dragOrigin += faceDelta;
//                }
//            }
            
            return false;
        }
        
        Vec3 ScaleObjectsTool::selectDelta(const Vec3& relativeDelta, const Vec3& absoluteDelta, const FloatType mouseDistance) const {
            // select the delta that is closest to the actual delta indicated by the mouse cursor
            const FloatType mouseDistance2 = mouseDistance * mouseDistance;
            return (std::abs(relativeDelta.squaredLength() - mouseDistance2) <
                    std::abs(absoluteDelta.squaredLength() - mouseDistance2) ?
                    relativeDelta :
                    absoluteDelta);
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
        
        bool ScaleObjectsTool::splitBrushes(const Vec3& delta) {
            MapDocumentSPtr document = lock(m_document);
            const BBox3& worldBounds = document->worldBounds();
            const bool lockTextures = pref(Preferences::TextureLock);
            
            // First ensure that the drag can be applied at all. For this, check whether each drag faces is moved
            // "up" along its normal.
//            if (!std::all_of(std::begin(m_dragFaces), std::end(m_dragFaces),
//                             [&delta](const Model::BrushFace* face) { return Math::pos(face->boundary().normal.dot(delta)); }))
//                return false;
            
            Model::BrushList newBrushes;
            Model::BrushFaceList newDragFaces;
            Model::ParentChildrenMap newNodes;
            
//            for (Model::BrushFace* dragFace : m_dragFaces) {
//                Model::Brush* brush = dragFace->brush();
//
//                Model::Brush* newBrush = brush->clone(worldBounds);
//                Model::BrushFace* newDragFace = findMatchingFace(newBrush, dragFace);
//
//                newBrushes.push_back(newBrush);
//                newDragFaces.push_back(newDragFace);
//
//                if (!newBrush->canMoveBoundary(worldBounds, newDragFace, delta)) {
//                    // There is a brush for which the move is not applicable. Abort.
//                    VectorUtils::deleteAll(newBrushes);
//                    return false;
//                } else {
//                    Model::BrushFace* clipFace = newDragFace->clone();
//                    clipFace->invert();
//
//                    newBrush->moveBoundary(worldBounds, newDragFace, delta, lockTextures);
//
//                    // This should never happen, but let's be on the safe side.
//                    if (!newBrush->clip(worldBounds, clipFace)) {
//                        delete clipFace;
//                        VectorUtils::deleteAll(newBrushes);
//                        return false;
//                    }
//
//                    newNodes[brush->parent()].push_back(newBrush);
//                }
//            }
            
            document->deselectAll();
            const Model::NodeList addedNodes = document->addNodes(newNodes);
            document->select(addedNodes);
//            m_dragFaces = newDragFaces;
            
            return true;
        }
        
        Model::BrushFace* ScaleObjectsTool::findMatchingFace(Model::Brush* brush, const Model::BrushFace* reference) const {
            Model::FindMatchingBrushFaceVisitor<MatchFaceBoundary> visitor((MatchFaceBoundary(reference)));
            visitor.visit(brush);
            if (!visitor.hasResult())
                return nullptr;
            return visitor.result();
        }
        
        Polygon3::List ScaleObjectsTool::dragFaceDescriptors() const {
            Polygon3::List result;
//            result.reserve(m_dragFaces.size());
//            std::transform(std::begin(m_dragFaces), std::end(m_dragFaces), std::back_inserter(result), [](const Model::BrushFace* face) { return face->polygon(); });
            return result;
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
