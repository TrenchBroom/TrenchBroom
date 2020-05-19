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

#ifndef TrenchBroom_Polyhedron_Misc_h
#define TrenchBroom_Polyhedron_Misc_h

#include "Polyhedron.h"

#include <vecmath/vec.h>
#include <vecmath/ray.h>
#include <vecmath/plane.h>
#include <vecmath/bbox.h>
#include <vecmath/scalar.h>
#include <vecmath/util.h>

#include <unordered_map>
#include <unordered_set>

namespace TrenchBroom {
    namespace Model {
        template <typename T, typename FP, typename VP>
        const vm::vec<T,3>& Polyhedron<T,FP,VP>::GetVertexPosition::operator()(const Vertex* vertex) const {
            return vertex->position();
        }

        template <typename T, typename FP, typename VP>
        const vm::vec<T,3>& Polyhedron<T,FP,VP>::GetVertexPosition::operator()(const HalfEdge* halfEdge) const {
            return halfEdge->origin()->position();
        }

        template <typename T, typename FP, typename VP>
        Polyhedron<T,FP,VP>::Callback::~Callback() = default;

        template <typename T, typename FP, typename VP>
        void Polyhedron<T,FP,VP>::Callback::vertexWasCreated(Vertex* /* vertex */) {}

        template <typename T, typename FP, typename VP>
        void Polyhedron<T,FP,VP>::Callback::vertexWillBeDeleted(Vertex* /* vertex */) {}

        template <typename T, typename FP, typename VP>
        void Polyhedron<T,FP,VP>::Callback::vertexWasAdded(Vertex* /* vertex */) {}

        template <typename T, typename FP, typename VP>
        void Polyhedron<T,FP,VP>::Callback::vertexWillBeRemoved(Vertex* /* vertex */) {}

        template <typename T, typename FP, typename VP>
        vm::plane<T,3> Polyhedron<T,FP,VP>::Callback::getPlane(const Face* face) const {
            const auto& boundary = face->boundary();
            assert(boundary.size() >= 3u);

            for (const HalfEdge* curEdge : boundary) {
                const auto* e1 = curEdge;
                const auto* e2 = e1->next();
                const auto* e3 = e2->next();

                const auto& p1 = e1->origin()->position();
                const auto& p2 = e2->origin()->position();
                const auto& p3 = e3->origin()->position();

                const auto [valid, result] = vm::from_points(p2, p1, p3);
                if (valid) {
                    return result;
                }
            }

            // TODO: We should really throw an exception here.
            assert(false);
            return ::vm::plane<T,3>(); // Ooops!
        }

        template <typename T, typename FP, typename VP>
        void Polyhedron<T,FP,VP>::Callback::faceWasCreated(Face* /* face */) {}

        template <typename T, typename FP, typename VP>
        void Polyhedron<T,FP,VP>::Callback::faceWillBeDeleted(Face* /* face */) {}

        template <typename T, typename FP, typename VP>
        void Polyhedron<T,FP,VP>::Callback::faceDidChange(Face* /* face */) {}

        template <typename T, typename FP, typename VP>
        void Polyhedron<T,FP,VP>::Callback::faceWasFlipped(Face* /* face */) {}

        template <typename T, typename FP, typename VP>
        void Polyhedron<T,FP,VP>::Callback::faceWasSplit(Face* /* original */, Face* /* clone */) {}

        template <typename T, typename FP, typename VP>
        void Polyhedron<T,FP,VP>::Callback::facesWillBeMerged(Face* /* remaining */, Face* /* toDelete */) {}

        template <typename T, typename FP, typename VP>
        Polyhedron<T,FP,VP>::CopyCallback::~CopyCallback() = default;

        template <typename T, typename FP, typename VP>
        void Polyhedron<T,FP,VP>::CopyCallback::vertexWasCopied(const Vertex* /* original */, Vertex* /* copy */) {}

        template <typename T, typename FP, typename VP>
        void Polyhedron<T,FP,VP>::CopyCallback::faceWasCopied(const Face* /* original */, Face* /* copy */) {}

        template <typename T, typename FP, typename VP>
        Polyhedron<T,FP,VP>::Polyhedron() {
            updateBounds();
        }

        template <typename T, typename FP, typename VP>
        Polyhedron<T,FP,VP>::Polyhedron(std::initializer_list<vm::vec<T,3>> positions) {
            addPoints(std::begin(positions), std::end(positions));
        }

        template <typename T, typename FP, typename VP>
        Polyhedron<T,FP,VP>::Polyhedron(const vm::bbox<T,3>& bounds) :
            m_bounds(bounds) {
            if (m_bounds.min == m_bounds.max) {
                addPoint(m_bounds.min);
                return;
            }

            // Explicitly create the polyhedron for better performance when building brushes.

            const vm::vec<T,3> p1(m_bounds.min.x(), m_bounds.min.y(), m_bounds.min.z());
            const vm::vec<T,3> p2(m_bounds.min.x(), m_bounds.min.y(), m_bounds.max.z());
            const vm::vec<T,3> p3(m_bounds.min.x(), m_bounds.max.y(), m_bounds.min.z());
            const vm::vec<T,3> p4(m_bounds.min.x(), m_bounds.max.y(), m_bounds.max.z());
            const vm::vec<T,3> p5(m_bounds.max.x(), m_bounds.min.y(), m_bounds.min.z());
            const vm::vec<T,3> p6(m_bounds.max.x(), m_bounds.min.y(), m_bounds.max.z());
            const vm::vec<T,3> p7(m_bounds.max.x(), m_bounds.max.y(), m_bounds.min.z());
            const vm::vec<T,3> p8(m_bounds.max.x(), m_bounds.max.y(), m_bounds.max.z());

            Vertex* v1 = new Vertex(p1);
            Vertex* v2 = new Vertex(p2);
            Vertex* v3 = new Vertex(p3);
            Vertex* v4 = new Vertex(p4);
            Vertex* v5 = new Vertex(p5);
            Vertex* v6 = new Vertex(p6);
            Vertex* v7 = new Vertex(p7);
            Vertex* v8 = new Vertex(p8);

            m_vertices.push_back(v1);
            m_vertices.push_back(v2);
            m_vertices.push_back(v3);
            m_vertices.push_back(v4);
            m_vertices.push_back(v5);
            m_vertices.push_back(v6);
            m_vertices.push_back(v7);
            m_vertices.push_back(v8);

            // Front face
            HalfEdge* f1h1 = new HalfEdge(v1);
            HalfEdge* f1h2 = new HalfEdge(v5);
            HalfEdge* f1h3 = new HalfEdge(v6);
            HalfEdge* f1h4 = new HalfEdge(v2);
            HalfEdgeList f1b;
            f1b.push_back(f1h1);
            f1b.push_back(f1h2);
            f1b.push_back(f1h3);
            f1b.push_back(f1h4);
            m_faces.push_back(new Face(std::move(f1b)));

            // Left face
            HalfEdge* f2h1 = new HalfEdge(v1);
            HalfEdge* f2h2 = new HalfEdge(v2);
            HalfEdge* f2h3 = new HalfEdge(v4);
            HalfEdge* f2h4 = new HalfEdge(v3);
            HalfEdgeList f2b;
            f2b.push_back(f2h1);
            f2b.push_back(f2h2);
            f2b.push_back(f2h3);
            f2b.push_back(f2h4);
            m_faces.push_back(new Face(std::move(f2b)));

            // Bottom face
            HalfEdge* f3h1 = new HalfEdge(v1);
            HalfEdge* f3h2 = new HalfEdge(v3);
            HalfEdge* f3h3 = new HalfEdge(v7);
            HalfEdge* f3h4 = new HalfEdge(v5);
            HalfEdgeList f3b;
            f3b.push_back(f3h1);
            f3b.push_back(f3h2);
            f3b.push_back(f3h3);
            f3b.push_back(f3h4);
            m_faces.push_back(new Face(std::move(f3b)));

            // Top face
            HalfEdge* f4h1 = new HalfEdge(v2);
            HalfEdge* f4h2 = new HalfEdge(v6);
            HalfEdge* f4h3 = new HalfEdge(v8);
            HalfEdge* f4h4 = new HalfEdge(v4);
            HalfEdgeList f4b;
            f4b.push_back(f4h1);
            f4b.push_back(f4h2);
            f4b.push_back(f4h3);
            f4b.push_back(f4h4);
            m_faces.push_back(new Face(std::move(f4b)));

            // Back face
            HalfEdge* f5h1 = new HalfEdge(v3);
            HalfEdge* f5h2 = new HalfEdge(v4);
            HalfEdge* f5h3 = new HalfEdge(v8);
            HalfEdge* f5h4 = new HalfEdge(v7);
            HalfEdgeList f5b;
            f5b.push_back(f5h1);
            f5b.push_back(f5h2);
            f5b.push_back(f5h3);
            f5b.push_back(f5h4);
            m_faces.push_back(new Face(std::move(f5b)));

            // Right face
            HalfEdge* f6h1 = new HalfEdge(v5);
            HalfEdge* f6h2 = new HalfEdge(v7);
            HalfEdge* f6h3 = new HalfEdge(v8);
            HalfEdge* f6h4 = new HalfEdge(v6);
            HalfEdgeList f6b;
            f6b.push_back(f6h1);
            f6b.push_back(f6h2);
            f6b.push_back(f6h3);
            f6b.push_back(f6h4);
            m_faces.push_back(new Face(std::move(f6b)));

            m_edges.push_back(new Edge(f1h4, f2h1)); // v1, v2
            m_edges.push_back(new Edge(f2h4, f3h1)); // v1, v3
            m_edges.push_back(new Edge(f1h1, f3h4)); // v1, v5
            m_edges.push_back(new Edge(f2h2, f4h4)); // v2, v4
            m_edges.push_back(new Edge(f4h1, f1h3)); // v2, v6
            m_edges.push_back(new Edge(f2h3, f5h1)); // v3, v4
            m_edges.push_back(new Edge(f3h2, f5h4)); // v3, v7
            m_edges.push_back(new Edge(f4h3, f5h2)); // v4, v8
            m_edges.push_back(new Edge(f1h2, f6h4)); // v5, v6
            m_edges.push_back(new Edge(f6h1, f3h3)); // v5, v7
            m_edges.push_back(new Edge(f6h3, f4h2)); // v6, v8
            m_edges.push_back(new Edge(f6h2, f5h3)); // v7, v8
        }

        template <typename T, typename FP, typename VP>
        Polyhedron<T,FP,VP>::Polyhedron(const std::vector<vm::vec<T,3>>& positions) {
            addPoints(std::begin(positions), std::end(positions));
        }

        template <typename T, typename FP, typename VP>
        Polyhedron<T,FP,VP>::Polyhedron(const Polyhedron<T,FP,VP>& other) {
            CopyCallback callback;
            Copy copy(other.faces(), other.edges(), other.vertices(), *this, callback);
        }

        template <typename T, typename FP, typename VP>
        Polyhedron<T,FP,VP>::Polyhedron(const Polyhedron<T,FP,VP>& other, CopyCallback& callback) {
            Copy copy(other.faces(), other.edges(), other.vertices(), *this, callback);
        }

        template <typename T, typename FP, typename VP>
        Polyhedron<T,FP,VP>::Polyhedron(Polyhedron<T,FP,VP>&& other) noexcept :
            m_vertices(std::move(other.m_vertices)),
            m_edges(std::move(other.m_edges)),
            m_faces(std::move(other.m_faces)),
            m_bounds(std::move(other.m_bounds)) {}

        template <typename T, typename FP, typename VP>
        Polyhedron<T,FP,VP>& Polyhedron<T,FP,VP>::operator=(const Polyhedron<T,FP,VP>& other) {
            Polyhedron<T,FP,VP> copy(other);
            swap(*this, copy);
            return *this;
        }

        template <typename T, typename FP, typename VP>
        Polyhedron<T,FP,VP>& Polyhedron<T,FP,VP>::operator=(Polyhedron<T,FP,VP>&& other) = default;

        /**
         * Copies a polyhedron.
         */
        template <typename T, typename FP, typename VP>
        class Polyhedron<T,FP,VP>::Copy {
        private:
            using VertexMap = std::unordered_map<const Vertex*, Vertex*>;
            using VertexMapEntry = typename VertexMap::value_type;

            using HalfEdgeMap = std::unordered_map<const HalfEdge*, HalfEdge*>;
            using HalfEdgeMapEntry = typename HalfEdgeMap::value_type;

            /**
             * Maps the vertices of the original to their copies.
             */
            VertexMap m_vertexMap;

            /**
             * Maps the half edges of the original to their copies.
             */
            HalfEdgeMap m_halfEdgeMap;

            /**
             * The copied vertices.
             */
            VertexList m_vertices;

            /**
             * The copied edges.
             */
            EdgeList m_edges;

            /**
             * The copied faces.
             */
            FaceList m_faces;

            /**
             * The polyhedron which should become a copy.
             */
            Polyhedron& m_destination;
        public:
            /**
             * Copies a polyhedron with the given faces, edges and vertices into the given destination polyhedron.
             * The callback can be used to set up the face and vertex payloads.
             *
             * @param originalFaces the faces to copy
             * @param originalEdges the edges to copy
             * @param originalVertices the vertices to copy
             * @param destination the destination polyhedron that will become a copy
             * @param callback the callback to call for every created face or vertex             *
             */
            Copy(const FaceList& originalFaces, const EdgeList& originalEdges, const VertexList& originalVertices, Polyhedron& destination, CopyCallback& callback) :
                m_destination(destination) {
                copyVertices(originalVertices, callback);
                copyFaces(originalFaces, callback);
                copyEdges(originalEdges);
                swapContents();
            }
        private:
            void copyVertices(const VertexList& originalVertices, CopyCallback& callback) {
                for (const Vertex* currentVertex : originalVertices) {
                    Vertex* copy = new Vertex(currentVertex->position());
                    callback.vertexWasCopied(currentVertex, copy);
                    assert(m_vertexMap.count(currentVertex) == 0u);
                    m_vertexMap.insert(std::make_pair(currentVertex, copy));
                    m_vertices.push_back(copy);
                    currentVertex = currentVertex->next();
                }
            }

            void copyFaces(const FaceList& originalFaces, CopyCallback& callback) {
                for (const Face* currentFace : originalFaces) {
                    copyFace(currentFace, callback);
                }
            }

            void copyFace(const Face* originalFace, CopyCallback& callback) {
                HalfEdgeList myBoundary;

                for (const HalfEdge* currentHalfEdge : originalFace->boundary()) {
                    myBoundary.push_back(copyHalfEdge(currentHalfEdge));
                }

                Face* copy = new Face(std::move(myBoundary));
                callback.faceWasCopied(originalFace, copy);
                m_faces.push_back(copy);
            }

            HalfEdge* copyHalfEdge(const HalfEdge* original) {
                const Vertex* originalOrigin = original->origin();

                Vertex* myOrigin = findVertex(originalOrigin);
                HalfEdge* copy = new HalfEdge(myOrigin);
                assert(m_halfEdgeMap.count(original) == 0u);
                m_halfEdgeMap.insert(std::make_pair(original, copy));
                return copy;
            }

            Vertex* findVertex(const Vertex* original) {
                typename VertexMap::iterator it = m_vertexMap.find(original);
                assert(it != std::end(m_vertexMap));
                return it->second;
            }

            void copyEdges(const EdgeList& originalEdges) {
                for (const Edge* currentEdge : originalEdges) {
                    m_edges.push_back(copyEdge(currentEdge));
                }
            }

            Edge* copyEdge(const Edge* original) {
                HalfEdge* myFirst = findOrCopyHalfEdge(original->firstEdge());
                if (!original->fullySpecified()) {
                    return new Edge(myFirst);
                }

                HalfEdge* mySecond = findOrCopyHalfEdge(original->secondEdge());
                return new Edge(myFirst, mySecond);
            }

            HalfEdge* findOrCopyHalfEdge(const HalfEdge* original) {
                auto it = m_halfEdgeMap.find(original);
                if (it == std::end(m_halfEdgeMap)) {
                    const Vertex* originalOrigin = original->origin();
                    Vertex* myOrigin = findVertex(originalOrigin);
                    HalfEdge* copy = new HalfEdge(myOrigin);
                    m_halfEdgeMap.insert(std::make_pair(original, copy));
                    return copy;
                } else {
                    return it->second;
                }
            }

            void swapContents() {
                using std::swap;
                swap(m_vertices, m_destination.m_vertices);
                swap(m_edges, m_destination.m_edges);
                swap(m_faces, m_destination.m_faces);
                m_destination.updateBounds();
            }
        };

        template <typename T, typename FP, typename VP>
        bool Polyhedron<T,FP,VP>::operator==(const Polyhedron& other) const {
            if (vertexCount() != other.vertexCount()) {
                return false;
            }
            if (edgeCount() != other.edgeCount()) {
                return false;
            }
            if (faceCount() != other.faceCount()) {
                return false;
            }

            for (const Vertex* current : m_vertices) {
                if (!other.hasVertex(current->position(), 0.0)) {
                    return false;
                }
            }

            for (const Edge* current : m_edges) {
                if (!other.hasEdge(current->firstVertex()->position(), current->secondVertex()->position(), 0.0)) {
                    return false;
                }
            }

            for (const Face* current : m_faces) {
                if (!other.hasFace(current->vertexPositions(), 0.0)) {
                    return false;
                }
            }

            return true;
        }

        template <typename T, typename FP, typename VP>
        bool Polyhedron<T,FP,VP>::operator!=(const Polyhedron& other) const {
            return !(*this == other);
        }

        template <typename T, typename FP, typename VP>
        size_t Polyhedron<T,FP,VP>::vertexCount() const {
            return m_vertices.size();
        }

        template <typename T, typename FP, typename VP>
        const typename Polyhedron<T,FP,VP>::VertexList& Polyhedron<T,FP,VP>::vertices() const {
            return m_vertices;
        }

        template <typename T, typename FP, typename VP>
        std::vector<vm::vec<T,3>> Polyhedron<T,FP,VP>::vertexPositions() const {
            std::vector<vm::vec<T,3>> result;
            result.reserve(vertexCount());
            for (const Vertex* vertex : m_vertices) {
                result.push_back(vertex->position());
            }
            return result;
        }

        template <typename T, typename FP, typename VP>
        size_t Polyhedron<T,FP,VP>::edgeCount() const {
            return m_edges.size();
        }

        template <typename T, typename FP, typename VP>
        const typename Polyhedron<T,FP,VP>::EdgeList& Polyhedron<T,FP,VP>::edges() const {
            return m_edges;
        }

        template <typename T, typename FP, typename VP>
        bool Polyhedron<T,FP,VP>::hasEdge(const vm::vec<T,3>& pos1, const vm::vec<T,3>& pos2, const T epsilon) const {
            return findEdgeByPositions(pos1, pos2, epsilon) != nullptr;
        }

        template <typename T, typename FP, typename VP>
        size_t Polyhedron<T,FP,VP>::faceCount() const {
            return m_faces.size();
        }

        template <typename T, typename FP, typename VP>
        const typename Polyhedron<T,FP,VP>::FaceList& Polyhedron<T,FP,VP>::faces() const {
            return m_faces;
        }

        template <typename T, typename FP, typename VP>
        bool Polyhedron<T,FP,VP>::hasFace(const std::vector<vm::vec<T,3>>& positions, const T epsilon) const {
            return findFaceByPositions(positions, epsilon) != nullptr;
        }

        template <typename T, typename FP, typename VP>
        const vm::bbox<T,3>& Polyhedron<T,FP,VP>::bounds() const {
            return m_bounds;
        }

        template <typename T, typename FP, typename VP>
        bool Polyhedron<T,FP,VP>::empty() const {
            return vertexCount() == 0;
        }

        template <typename T, typename FP, typename VP>
        bool Polyhedron<T,FP,VP>::point() const {
            return vertexCount() == 1;
        }

        template <typename T, typename FP, typename VP>
        bool Polyhedron<T,FP,VP>::edge() const {
            return vertexCount() == 2;
        }

        template <typename T, typename FP, typename VP>
        bool Polyhedron<T,FP,VP>::polygon() const {
            return faceCount() == 1;
        }

        template <typename T, typename FP, typename VP>
        bool Polyhedron<T,FP,VP>::polyhedron() const {
            return faceCount() > 3;
        }

        template <typename T, typename FP, typename VP>
        bool Polyhedron<T,FP,VP>::closed() const {
            return vertexCount() + faceCount() == edgeCount() + 2;
        }

        template <typename T, typename FP, typename VP>
        void Polyhedron<T,FP,VP>::clear() {
            m_faces.clear();
            m_edges.clear();
            m_vertices.clear();
            updateBounds();
        }

        template <typename T, typename FP, typename VP>
        Polyhedron<T,FP,VP>::FaceHit::FaceHit(Face* i_face, const T i_distance) : face(i_face), distance(i_distance) {}

        template <typename T, typename FP, typename VP>
        Polyhedron<T,FP,VP>::FaceHit::FaceHit() : face(nullptr), distance(vm::nan<T>()) {}

        template <typename T, typename FP, typename VP>
        bool Polyhedron<T,FP,VP>::FaceHit::isMatch() const { return face != nullptr; }

        template <typename T, typename FP, typename VP>
        typename Polyhedron<T,FP,VP>::FaceHit Polyhedron<T,FP,VP>::pickFace(const vm::ray<T,3>& ray) const {
            const auto side = polygon() ? vm::side::both : vm::side::front;
            auto* firstFace = m_faces.front();
            auto* currentFace = firstFace;
            do {
                const auto distance = currentFace->intersectWithRay(ray, side);
                if (!vm::is_nan(distance)) {
                    return FaceHit(currentFace, distance);
                }
                currentFace = currentFace->next();
            } while (currentFace != firstFace);
            return FaceHit();
        }

        template <typename T, typename FP, typename VP>
        bool Polyhedron<T,FP,VP>::hasVertex(const vm::vec<T,3>& position, const T epsilon) const {
            return findVertexByPosition(position, epsilon) != nullptr;
        }

        template <typename T, typename FP, typename VP>
        bool Polyhedron<T,FP,VP>::hasAnyVertex(const std::vector<vm::vec<T,3>>& positions, const T epsilon) const {
            for (const vm::vec<T,3>& position : positions) {
                if (hasVertex(position, epsilon)) {
                    return true;
                }
            }
            return false;
        }

        template <typename T, typename FP, typename VP>
        bool Polyhedron<T,FP,VP>::hasAllVertices(const std::vector<vm::vec<T,3>>& positions, const T epsilon) const {
            if (positions.size() != vertexCount()) {
                return false;
            }
            for (const vm::vec<T,3>& position : positions) {
                if (!hasVertex(position, epsilon)) {
                    return false;
                }
            }
            return true;
        }

        template <typename T, typename FP, typename VP>
        typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::findVertexByPosition(const vm::vec<T,3>& position, const T epsilon) const {
            if (m_vertices.empty()) {
                return nullptr;
            }

            Vertex* firstVertex = m_vertices.front();
            Vertex* currentVertex = firstVertex;
            do {
                if (vm::is_equal(position, currentVertex->position(), epsilon)) {
                    return currentVertex;
                }
                currentVertex = currentVertex->next();
            } while (currentVertex != firstVertex);
            return nullptr;
        }

        template <typename T, typename FP, typename VP>
        typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::findClosestVertex(const vm::vec<T,3>& position, const T maxDistance) const {
            if (m_vertices.empty()) {
                return nullptr;
            }

            auto closestDistance2 = maxDistance * maxDistance;
            Vertex* closestVertex = nullptr;

            Vertex* firstVertex = m_vertices.front();
            Vertex* currentVertex = firstVertex;
            do {
                const T currentDistance2 = vm::squared_distance(position, currentVertex->position());
                if (currentDistance2 < closestDistance2) {
                    closestDistance2 = currentDistance2;
                    closestVertex = currentVertex;
                }
                currentVertex = currentVertex->next();
            } while (currentVertex != firstVertex);
            return closestVertex;
        }

        template <typename T, typename FP, typename VP>
        typename Polyhedron<T,FP,VP>::Edge* Polyhedron<T,FP,VP>::findEdgeByPositions(const vm::vec<T,3>& pos1, const vm::vec<T,3>& pos2, const T epsilon) const {
            if (m_edges.empty()) {
                return nullptr;
            }

            Edge* firstEdge = m_edges.front();
            Edge* currentEdge = firstEdge;
            do {
                if (currentEdge->hasPositions(pos1, pos2, epsilon)) {
                    return currentEdge;
                }
                currentEdge = currentEdge->next();
            } while (currentEdge != firstEdge);
            return nullptr;
        }

        template <typename T, typename FP, typename VP>
        typename Polyhedron<T,FP,VP>::Edge* Polyhedron<T,FP,VP>::findClosestEdge(const vm::vec<T,3>& pos1, const vm::vec<T,3>& pos2, const T maxDistance) const {
            if (m_edges.empty()) {
                return nullptr;
            }

            auto closestDistance = maxDistance;
            Edge* closestEdge = nullptr;

            Edge* firstEdge = m_edges.front();
            Edge* currentEdge = firstEdge;
            do {
                const auto currentDistance = currentEdge->distanceTo(pos1, pos2);
                if (currentDistance < closestDistance) {
                    closestDistance = currentDistance;
                    closestEdge = currentEdge;
                }
                currentEdge = currentEdge->next();
            } while (currentEdge != firstEdge);
            return closestEdge;
        }

        template <typename T, typename FP, typename VP>
        typename Polyhedron<T,FP,VP>::Face* Polyhedron<T,FP,VP>::findFaceByPositions(const std::vector<vm::vec<T,3>>& positions, const T epsilon) const {
            Face* firstFace = m_faces.front();
            Face* currentFace = firstFace;
            do {
                if (currentFace->hasVertexPositions(positions, epsilon)) {
                    return currentFace;
                }
                currentFace = currentFace->next();
            } while (currentFace != firstFace);
            return nullptr;
        }

        template <typename T, typename FP, typename VP>
        typename Polyhedron<T,FP,VP>::Face* Polyhedron<T,FP,VP>::findClosestFace(const std::vector<vm::vec<T,3>>& positions, const T maxDistance) {
            auto closestDistance = maxDistance;
            Face* closestFace = nullptr;

            Face* firstFace = m_faces.front();
            Face* currentFace = firstFace;
            do {
                const auto currentDistance = currentFace->distanceTo(positions);
                if (currentDistance < closestDistance) {
                    closestDistance = currentDistance;
                    closestFace = currentFace;
                }
                currentFace = currentFace->next();
            } while (currentFace != firstFace);
            return closestFace;
        }

        template <typename T, typename FP, typename VP>
        void Polyhedron<T,FP,VP>::updateBounds() {
            auto builder = typename vm::bbox<T,3>::builder();
            builder.add(std::begin(m_vertices), std::end(m_vertices), GetVertexPosition());

            if (!builder.initialized()) {
                m_bounds.min = m_bounds.max = vm::vec<T,3>::nan();
            } else {
                m_bounds = builder.bounds();
            }
        }

        template <typename T, typename FP, typename VP>
        void Polyhedron<T,FP,VP>::correctVertexPositions(const size_t decimals, const T epsilon) {
            for (auto* vertex : m_vertices) {
                vertex->correctPosition(decimals, epsilon);
            }
            updateBounds();
        }

        template <typename T, typename FP, typename VP>
        bool Polyhedron<T,FP,VP>::healEdges(const T minLength) {
            Callback callback;
            return healEdges(callback, minLength);
        }

        template <typename T, typename FP, typename VP>
        bool Polyhedron<T,FP,VP>::healEdges(Callback& callback, const T minLength) {
            const T minLength2 = minLength * minLength;

            /*
             We have to iterate over all edges while the list of edges is being modified, so we cannot use the usual
             do / while iteration. Instead, we count the number of edges we have examined - but since one or more edges
             can be removed in every iteration, we have to correct that number for the decrease in the total number of
             edges of the brush - this is where sizeDelta comes in. If no edges has been removed, it comes out as -1.
             If one edge has been removed, it comes out as 0, if two edges have been removed, it comes out as +1, and so on.

             Since sizeDelta is subtracted from the number of examined edges, it corrects exactly for the change in the number
             of edges of the brush.
             */

            long examined = 0;
            Edge* currentEdge = m_edges.front();
            while (examined < static_cast<long>(m_edges.size()) && polyhedron()) {
                const size_t oldSize = m_edges.size();

                const T length2 = vm::squared_length(currentEdge->vector());
                if (length2 < minLength2) {
                    currentEdge = removeEdge(currentEdge, callback);
                } else {
                    currentEdge = currentEdge->next();
                }

                const size_t newSize = m_edges.size();
                const long sizeDelta = static_cast<long>(oldSize - newSize) - 1;
                examined -= sizeDelta;
            }

            assert(!polyhedron() || checkEdgeLengths(minLength));

            updateBounds();

            return polyhedron();
        }

        template <typename T, typename FP, typename VP>
        typename Polyhedron<T,FP,VP>::Edge* Polyhedron<T,FP,VP>::removeEdge(Edge* edge, Callback& callback) {
            // First, transfer all edges from the second to the first vertex of the given edge.
            // This results in the edge being a loop and the second vertex to be orphaned.
            auto* firstVertex = edge->firstVertex();
            auto* secondVertex = edge->secondVertex();
            while (secondVertex->leaving() != nullptr) {
                auto* leaving = secondVertex->leaving();
                auto* newLeaving = leaving->previous()->twin();
                leaving->setOrigin(firstVertex);
                if (newLeaving->origin() == secondVertex) {
                    secondVertex->setLeaving(newLeaving);
                } else {
                    secondVertex->setLeaving(nullptr);
                }
            }

            // Remove the edge's first edge from its first face and delete the face if it degenerates
            {
                auto* firstFace = edge->firstFace();
                auto* firstEdge = edge->firstEdge();
                auto* nextEdge = firstEdge->next();

                firstVertex->setLeaving(firstEdge->previous()->twin());
                firstFace->removeFromBoundary(firstEdge);
                nextEdge->setOrigin(firstVertex);

                if (firstFace->vertexCount() == 2) {
                    removeDegenerateFace(firstFace, callback);
                }
            }

            // Remove the edges's second edge from its second face and delete the face if it degenerates
            {
                auto* secondFace = edge->secondFace();
                auto* secondEdge = edge->secondEdge();

                secondFace->removeFromBoundary(secondEdge);

                if (secondFace->vertexCount() == 2) {
                    removeDegenerateFace(secondFace, callback);
                }
            }

            callback.vertexWillBeDeleted(secondVertex);
            m_vertices.remove(secondVertex);

            auto* result = edge->next();
            m_edges.remove(edge);

            // Merge faces that may have become coplanar
            {
                auto* firstEdge = firstVertex->leaving();
                auto* currentEdge = firstEdge;
                do {
                    auto* nextEdge = currentEdge->nextIncident();
                    auto* currentFace = firstEdge->face();
                    auto* neighbour = firstEdge->twin()->face();
                    if (currentFace->coplanar(neighbour)) {
                        result = mergeNeighbours(currentEdge, result, callback);
                    }
                    currentEdge = nextEdge;
                } while (currentEdge != firstEdge);
            }

            return result;
        }

        template <typename T, typename FP, typename VP>
        void Polyhedron<T,FP,VP>::removeDegenerateFace(Face* face, Callback& callback) {
            assert(face != nullptr);
            assert(face->vertexCount() == 2u);

            // The boundary of the face to remove consists of two half edges:
            auto* halfEdge1 = face->boundary().front();
            auto* halfEdge2 = halfEdge1->next();
            assert(halfEdge2->next() == halfEdge1);
            assert(halfEdge1->previous() == halfEdge2);

            // The face has two vertices:
            auto* vertex1 = halfEdge1->origin();
            auto* vertex2 = halfEdge2->origin();

            // Make sure we don't delete the vertices' leaving edges:
            vertex1->setLeaving(halfEdge2->twin());
            vertex2->setLeaving(halfEdge1->twin());

            assert(vertex1->leaving() != halfEdge1);
            assert(vertex1->leaving() != halfEdge2);
            assert(vertex2->leaving() != halfEdge1);
            assert(vertex2->leaving() != halfEdge2);

            // These two edges will be merged into one:
            auto* edge1 = halfEdge1->edge();
            auto* edge2 = halfEdge2->edge();

            // The twins of the two half edges of the degenerate face will become twins now.
            auto* halfEdge1Twin = halfEdge1->twin();
            auto* halfEdge2Twin = halfEdge2->twin();

            // We will keep edge1 and delete edge2.
            // Make sure that halfEdge1's twin is the first edge of edge1:
            edge1->makeFirstEdge(halfEdge1Twin);

            // Now replace halfEdge2 by new halfEdge2Twin:
            assert(halfEdge2Twin->edge() == edge2);
            halfEdge2Twin->unsetEdge();
            edge1->unsetSecondEdge(); // unsets halfEdge1, leaving halfEdge1Twin as the first half edge of edge1
            edge1->setSecondEdge(halfEdge2Twin); // replace halfEdge1 with halfEdge2Twin

            // Now edge1 should be correct:
            assert(edge1->firstEdge() == halfEdge1Twin);
            assert(edge1->secondEdge() == halfEdge2Twin);

            // Delete the now obsolete edge.
            // The constructor doesn't do anything, so no further cleanup is necessary.
            m_edges.remove(edge2);

            // Delete the degenerate face. This also deletes its boundary of halfEdge1 and halfEdge2.
            callback.faceWillBeDeleted(face);
            m_faces.remove(face);
        }

        template <typename T, typename FP, typename VP>
        typename Polyhedron<T,FP,VP>::Edge* Polyhedron<T,FP,VP>::mergeNeighbours(HalfEdge* borderFirst, Edge* validEdge, Callback& callback) {
            Face* face = borderFirst->face();
            Face* neighbour = borderFirst->twin()->face();

            // find the entire border between the two faces
            while (borderFirst->previous()->face() == face &&
                   borderFirst->previous()->twin()->face() == neighbour) {
                borderFirst = borderFirst->previous();
            }

            HalfEdge* twinLast = borderFirst->twin();
            HalfEdge* borderLast = borderFirst;

            while (borderLast->next()->face() == face &&
                   borderLast->next()->twin()->face() == neighbour) {
                borderLast = borderLast->next();
            }

            HalfEdge* twinFirst = borderLast->twin();

            // make sure we don't remove any leaving edges
            borderFirst->origin()->setLeaving(twinLast->next());
            twinFirst->origin()->setLeaving(borderLast->next());

            HalfEdge* remainingFirst = borderLast->next();
            HalfEdge* remainingLast = borderFirst->previous();

            HalfEdgeList edgesToRemove = face->removeFromBoundary(borderFirst, borderLast);
            HalfEdgeList remainingEdges = face->removeFromBoundary(remainingFirst, remainingLast);

            // the replaced twin edges are deleted
            neighbour->replaceBoundary(twinFirst, twinLast, std::move(remainingEdges));

            // now delete any remaining vertices and edges
            // edgesToRemove are deleted when the container falls out of scope
            HalfEdge* firstEdge = edgesToRemove.front();
            HalfEdge* curEdge = firstEdge;
            do {
                Edge* edge = curEdge->edge();
                HalfEdge* next = curEdge->next();
                Vertex* origin = curEdge->origin();

                if (edge == validEdge) {
                    validEdge = validEdge->next();
                }

                m_edges.remove(edge);

                // don't delete the origin of the first border edge!
                if (curEdge != borderFirst) {
                    callback.vertexWillBeDeleted(origin);
                    m_vertices.remove(origin);
                }

                curEdge = next;
            } while (curEdge != firstEdge);

            callback.facesWillBeMerged(neighbour, face);
            m_faces.remove(face);

            return validEdge;
        }
    }
}

#endif
