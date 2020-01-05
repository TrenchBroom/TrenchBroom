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

#ifndef TrenchBroom_Polyhedron_ConvexHull_h
#define TrenchBroom_Polyhedron_ConvexHull_h

#include "Macros.h"

#include "Polyhedron.h"

#include <vecmath/segment.h>
#include <vecmath/plane.h>
#include <vecmath/bbox.h>
#include <vecmath/constants.h>
#include <vecmath/util.h>

#include <list>
#include <unordered_set>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        template <typename T, typename FP, typename VP>
        void Polyhedron<T,FP,VP>::addPoints(const std::vector<vm::vec<T,3>>& points) {
            addPoints(std::begin(points), std::end(points));
        }

        template <typename T, typename FP, typename VP>
        void Polyhedron<T,FP,VP>::addPoints(const std::vector<vm::vec<T,3>>& points, Callback& callback) {
            addPoints(std::begin(points), std::end(points), callback);
        }

        template <typename T, typename FP, typename VP> template <typename I>
        void Polyhedron<T,FP,VP>::addPoints(I cur, I end) {
            Callback c;
            while (cur != end) {
                addPoint(*cur++, c);
            }
        }

        template <typename T, typename FP, typename VP> template <typename I>
        void Polyhedron<T,FP,VP>::addPoints(I cur, I end, Callback& callback) {
            while (cur != end) {
                addPoint(*cur++, callback);
            }
        }

        template <typename T, typename FP, typename VP>
        typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::addPoint(const vm::vec<T,3>& position) {
            Callback c;
            return addPoint(position, c);
        }

        template <typename T, typename FP, typename VP>
        typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::addPoint(const vm::vec<T,3>& position, Callback& callback) {
            assert(checkInvariant());
            Vertex* result = nullptr;
            switch (vertexCount()) {
                case 0:
                    result = addFirstPoint(position, callback);
                    m_bounds.min = m_bounds.max = position;
                    break;
                case 1:
                    result = addSecondPoint(position, callback);
                    m_bounds = vm::merge(m_bounds, position);
                    break;
                case 2:
                    result = addThirdPoint(position, callback);
                    m_bounds = vm::merge(m_bounds, position);
                    break;
                default:
                    result = addFurtherPoint(position, callback);
                    if (result != nullptr) {
                        m_bounds = vm::merge(m_bounds, position);
                    }
                    break;
            }
            assert(checkInvariant());
            if (result != nullptr) {
                callback.vertexWasAdded(result);
            }
            return result;
        }

        template <typename T, typename FP, typename VP>
        void Polyhedron<T,FP,VP>::merge(const Polyhedron& other) {
            Callback c;
            merge(other, c);
        }

        template <typename T, typename FP, typename VP>
        void Polyhedron<T,FP,VP>::merge(const Polyhedron& other, Callback& callback) {
            for (const Vertex* vertex : other.vertices()) {
                addPoint(vertex->position(), callback);
            }
        }

        template <typename T, typename FP, typename VP>
        typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::addFirstPoint(const vm::vec<T,3>& position, Callback& callback) {
            assert(empty());
            Vertex* newVertex = new Vertex(position);
            m_vertices.push_back(newVertex);
            callback.vertexWasCreated(newVertex);
            return newVertex;
        }

        template <typename T, typename FP, typename VP>
        typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::addSecondPoint(const vm::vec<T,3>& position, Callback& callback) {
            assert(point());

            Vertex* onlyVertex = *std::begin(m_vertices);
            if (position != onlyVertex->position()) {
                Vertex* newVertex = new Vertex(position);
                m_vertices.push_back(newVertex);
                callback.vertexWasCreated(newVertex);

                HalfEdge* halfEdge1 = new HalfEdge(onlyVertex);
                HalfEdge* halfEdge2 = new HalfEdge(newVertex);
                Edge* edge = new Edge(halfEdge1, halfEdge2);
                m_edges.push_back(edge);
                return newVertex;
            } else {
                return nullptr;
            }
        }

        template <typename T, typename FP, typename VP>
        typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::addThirdPoint(const vm::vec<T,3>& position, Callback& callback) {
            assert(edge());

            Vertex* v1 = m_vertices.front();
            Vertex* v2 = v1->next();

            if (vm::is_colinear(v1->position(), v2->position(), position)) {
                return addColinearThirdPoint(position, callback);
            } else {
                return addNonColinearThirdPoint(position, callback);
            }
        }

        template <typename T, typename FP, typename VP>
        typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::addColinearThirdPoint(const vm::vec<T,3>& position, Callback& /* callback */) {
            assert(edge());

            auto* v1 = m_vertices.front();
            auto* v2 = v1->next();
            assert(vm::is_colinear(v1->position(), v2->position(), position));

            if (vm::segment<T,3>(v1->position(), v2->position()).contains(position, vm::constants<T>::almost_zero())) {
                return nullptr;
            }

            if (vm::segment<T,3>(position, v2->position()).contains(v1->position(), vm::constants<T>::almost_zero())) {
                v1->setPosition(position);
                return v1;
            }

            assert((vm::segment<T,3>(position, v1->position()).contains(v2->position(), vm::constants<T>::almost_zero())));
            v2->setPosition(position);
            return v2;
        }

        template <typename T, typename FP, typename VP>
        typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::addNonColinearThirdPoint(const vm::vec<T,3>& position, Callback& callback) {
            assert(edge());

            Vertex* v1 = m_vertices.front();
            Vertex* v2 = v1->next();
            assert(!vm::is_colinear(v1->position(), v2->position(), position));

            HalfEdge* h1 = v1->leaving();
            HalfEdge* h2 = v2->leaving();
            assert(h1->next() == h1);
            assert(h1->previous() == h1);
            assert(h2->next() == h2);
            assert(h2->previous() == h2);

            Vertex* v3 = new Vertex(position);
            HalfEdge* h3 = new HalfEdge(v3);

            Edge* e1 = m_edges.front();
            e1->makeFirstEdge(h1);
            e1->unsetSecondEdge();

            HalfEdgeList boundary;
            boundary.push_back(h1);
            boundary.push_back(h2);
            boundary.push_back(h3);

            Face* face = new Face(std::move(boundary));

            Edge* e2 = new Edge(h2);
            Edge* e3 = new Edge(h3);

            m_vertices.push_back(v3);
            m_edges.push_back(e2);
            m_edges.push_back(e3);
            m_faces.push_back(face);

            callback.vertexWasCreated(v1);
            callback.faceWasCreated(face);

            return v3;
        }


        template <typename T, typename FP, typename VP>
        typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::addFurtherPoint(const vm::vec<T,3>& position, Callback& callback) {
            assert(faceCount() > 0u);
            if (faceCount() == 1u) {
                return addFurtherPointToPolygon(position, callback);
            } else {
                return addFurtherPointToPolyhedron(position, callback);
            }
        }

        template <typename T, typename FP, typename VP>
        typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::addFurtherPointToPolygon(const vm::vec<T,3>& position, Callback& callback) {
            Face* face = m_faces.front();
            const vm::plane_status status = face->pointStatus(position);
            switch (status) {
                case vm::plane_status::inside:
                    return addPointToPolygon(position, callback);
                case vm::plane_status::above:
                    face->flip();
                    callback.faceWasFlipped(face);
                    switchFallthrough();
                case vm::plane_status::below:
                    return makePolyhedron(position, callback);
            }
            // will never be reached
            return nullptr;
        }

        template <typename T, typename FP, typename VP>
        typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::addPointToPolygon(const vm::vec<T,3>& position, Callback& callback) {
            assert(polygon());

            Face* face = m_faces.front();
            vm::plane<T,3> facePlane = callback.getPlane(face);

            HalfEdge* firstVisibleEdge = nullptr;
            HalfEdge* lastVisibleEdge = nullptr;

            for (HalfEdge* curEdge : face->boundary()) {
                HalfEdge* prevEdge = curEdge->previous();
                HalfEdge* nextEdge = curEdge->next();
                const vm::plane_status prevStatus = prevEdge->pointStatus(facePlane.normal, position);
                const vm::plane_status curStatus = curEdge->pointStatus(facePlane.normal, position);
                const vm::plane_status nextStatus = nextEdge->pointStatus(facePlane.normal, position);

                // If the current edge contains the point, it will not be added anyway.
                if (curStatus == vm::plane_status::inside &&
                    vm::segment<T, 3>(curEdge->origin()->position(), curEdge->destination()->position()).contains(position,
                        vm::constants<T>::almost_zero())) {
                    return nullptr;
                }

                if (prevStatus == vm::plane_status::below && curStatus != vm::plane_status::below) {
                    firstVisibleEdge = curEdge;
                }

                if (curStatus != vm::plane_status::below && nextStatus == vm::plane_status::below) {
                    lastVisibleEdge = curEdge;
                }

                if (firstVisibleEdge != nullptr && lastVisibleEdge != nullptr) {
                    break;
                }
            }

            // Is the point contained in the polygon?
            if (firstVisibleEdge == nullptr || lastVisibleEdge == nullptr) {
                return nullptr;
            }

            // Now we know which edges are visible from the point. These will have to be replaced with two new edges.
            Vertex* newVertex = new Vertex(position);
            HalfEdge* h1 = new HalfEdge(firstVisibleEdge->origin());
            HalfEdge* h2 = new HalfEdge(newVertex);

            face->insertIntoBoundaryAfter(lastVisibleEdge, HalfEdgeList({ h1 }));
            face->insertIntoBoundaryAfter(h1, HalfEdgeList({ h2 }));
            HalfEdgeList visibleEdges = face->removeFromBoundary(firstVisibleEdge, lastVisibleEdge);

            h1->setAsLeaving();

            Edge* e1 = new Edge(h1);
            Edge* e2 = new Edge(h2);

            // delete the visible vertices and edges.
            // the visible half edges are deleted when visibleEdges goes out of scope
            for (HalfEdge* curEdge : visibleEdges) {
                Edge* edge = curEdge->edge();
                m_edges.remove(edge);

                if (curEdge != visibleEdges.front()) {
                    Vertex* vertex = curEdge->origin();
                    callback.vertexWillBeDeleted(vertex);
                    m_vertices.remove(vertex);
                }
            }

            m_edges.push_back(e1);
            m_edges.push_back(e2);
            m_vertices.push_back(newVertex);
            callback.vertexWasCreated(newVertex);

            return newVertex;
        }

        template <typename T, typename FP, typename VP>
        void Polyhedron<T,FP,VP>::makePolygon(const std::vector<vm::vec<T,3>>& positions, Callback& callback) {
            assert(empty());
            assert(positions.size() > 2);

            HalfEdgeList boundary;
            for (size_t i = 0u; i < positions.size(); ++i) {
                const vm::vec<T,3>& p = positions[i];
                Vertex* v = new Vertex(p);
                HalfEdge* h = new HalfEdge(v);
                Edge* e = new Edge(h);

                m_vertices.push_back(v);
                callback.vertexWasCreated(v);

                boundary.push_back(h);
                m_edges.push_back(e);
            }

            Face* f = new Face(std::move(boundary));
            callback.faceWasCreated(f);
            m_faces.push_back(f);
        }

        template <typename T, typename FP, typename VP>
        typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::makePolyhedron(const vm::vec<T,3>& position, Callback& callback) {
            assert(polygon());

            Seam seam;
            Face* face = m_faces.front();
            const HalfEdgeList& boundary = face->boundary();

            // The seam must be CCW, so we have to iterate in reverse order in this case.
            for (auto it = boundary.rbegin(), end = boundary.rend(); it != end; ++it) {
                seam.push_back((*it)->edge());
            }

            return weave(seam, position, callback);
        }

        template <typename T, typename FP, typename VP>
        typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::addFurtherPointToPolyhedron(const vm::vec<T,3>& position, Callback& callback) {
            assert(polyhedron());
            if (contains(position, callback)) {
                return nullptr;
            }

            const Seam seam = createSeam(SplitByVisibilityCriterion(position));

            // If no correct seam could be created, we assume that the vertex was inside the polyhedron.
            // If the seam has multiple loops, this indicates that the point to be added is very close to
            // another vertex and no correct seam can be computed due to imprecision. In that case, we just
            // assume that the vertex is inside the polyhedron and skip it.
            if (seam.empty() || seam.hasMultipleLoops()) {
                return nullptr;
            }

            assert(seam.size() >= 3);
            split(seam, callback);

            return weave(seam, position, callback);
        }

        template <typename T, typename FP, typename VP>
        class Polyhedron<T,FP,VP>::Seam {
        private:
            using List = std::list<Edge*>;
            List m_edges;
        public:
            using iterator = typename List::iterator;
            using const_iterator = typename List::const_iterator;
        public:
            /**
             * Appends the given edge to the end of this seam.
             *
             * If this seam is not empty, then the given edge must not be identical to the last edge of this seam, and
             * its first vertex must be identical to the last edge's second vertex.
             *
             * @param edge the edge to append, must not be null
             */
            void push_back(Edge* edge) {
                assert(edge != nullptr);
                assert(empty() || edge != last());
                assert(checkEdge(edge));
                m_edges.push_back(edge);
            }


            /**
             * Replaces the range [first, end) of this seam with the given edge.
             *
             * @param first start of the range of edges to replace
             * @param end end end of the range of edges to replace
             * @param replacement the replacemenet edge, must not be null
             */
            void replace(typename List::iterator first, typename List::iterator end, Edge* replacement) {
                m_edges.erase(first, end);
                m_edges.insert(end, replacement);
                assert(check());
            }

            /**
             * Shifts this seam until the given criterion evaluates to true. If continued shifting doesn't satisfy the
             * given criterion, this function stops and returns false.
             *
             * @tparam C the type of the given criterion
             * @param criterion the criterion to check after each time this seam is shifted
             * @return true if shifting satisfied the given criterion, and false otherwise
             */
            template <typename C>
            bool shift(const C& criterion) {
                size_t i = 0;
                while (i < m_edges.size()) {
                    if (criterion(static_cast<const Seam&>(*this))) {
                        return true;
                    }
                    shift();
                    ++i;
                }
                return false;
            }

            /**
             * Shifts this seam by taking its first edge and moving it to the back of this seam.
             *
             * Given a seam of three edges e1, e2, e3, the effect of shifting it will be that the seam becomes e2, e3, e1.
             *
             * Assumes that this seam is not empty.
             */
            void shift() {
                assert(!m_edges.empty());
                const auto pos = std::end(m_edges);
                const auto first = std::begin(m_edges);
                const auto last = std::next(std::begin(m_edges), 1u);

                m_edges.splice(pos, m_edges, first, last);
                assert(check());
            }

            /**
             * Indicates whether this seam is empty.
             *
             * @return true if this seam is empty and false otherwise
             */
            bool empty() const {
                return m_edges.empty();
            }

            /**
             * Returns the number of edges in this seam.
             */
            size_t size() const {
                return m_edges.size();
            }

            /**
             * Returns the first edge of this seam.
             *
             * Assumes that this seam is not empty.
             */
            Edge* first() const {
                assert(!empty());
                return m_edges.front();
            }

            /**
             * Returns the second edge of this seam.
             *
             * Assumes that this seam contains at least two edges.
             */
            Edge* second() const {
                assert(size() > 1u);
                const_iterator it = std::begin(m_edges);
                std::advance(it, 1u);
                return *it;
            }

            /**
             * Returns the last edge of this seam.
             *
             * Assumes that this seam is not empty.
             */
            Edge* last() const {
                assert(!empty());
                return m_edges.back();
            }

            /**
             * Returns an iterator pointing to the first edge in this seam, or an end iterator if this seam is empty.
             */
            iterator begin() {
                return std::begin(m_edges);
            }

            /**
             * Returns an iterator pointing to the end of this seam.
             */
            iterator end() {
                return std::end(m_edges);
            }

            /**
             * Returns a const iterator pointing to the first edge in this seam, or an end iterator if this seam is empty.
             */
            const_iterator begin() const {
                return std::begin(m_edges);
            }

            /**
             * Returns a const iterator pointing to the end of this seam.
             */
            const_iterator end() const {
                return std::end(m_edges);
            }

            /**
             * Removes all edges from this seam.
             */
            void clear() {
                m_edges.clear();
            }

            /**
             * Checks whether this seam is a consecutive list of edges connected with their vertices.
             */
            bool hasMultipleLoops() const {
                assert(size() > 2);

                std::unordered_set<Vertex*> visitedVertices;
                for (const Edge* edge : m_edges) {
                    if (!visitedVertices.insert(edge->secondVertex()).second) {
                        return true;
                    }
                }
                return false;
            }
        private:
            /**
             * Checks whether the given edge is connected to last edge of the current seam, or more precisely, whether
             * the second vertex of the given edge is identical to the first vertex of the last edge of this seam.
             *
             * @param edge the edge to check
             * @return true if this seam is empty or if the given edge shares a vertex with the last edge of this seam,
             * and false otherwise
             */
            bool checkEdge(Edge* edge) const {
                if (m_edges.empty()) {
                    return true;
                }

                Edge* last = m_edges.back();
                return last->firstVertex() == edge->secondVertex();
            }

            /**
             * Checks whether the edges of this seam share their vertices, that is, for each edge, its second vertex is
             * identical to its predecessors first vertex.
             *
             * @return true if the edges of this seam share their vertices and false otherwise
             */
            bool check() const {
                assert(size() > 2);

                const Edge* last = m_edges.back();
                for (const Edge* edge : m_edges) {
                    if (last->firstVertex() != edge->secondVertex()) {
                        return false;
                    }

                    last = edge;
                }
                return true;
            }
        };

        template <typename T, typename FP, typename VP>
        class Polyhedron<T,FP,VP>::SplittingCriterion {
        private:
            typedef enum {
                MatchResult_First,
                MatchResult_Second,
                MatchResult_Both,
                MatchResult_Neither
            } MatchResult;
        public:
            virtual ~SplittingCriterion() {}
        public:
            Edge* findFirstSplittingEdge(EdgeList& edges) const {
                for (Edge* edge : edges) {
                    const MatchResult result = matches(edge);
                    switch (result) {
                        case MatchResult_Second:
                            edge->flip();
                            switchFallthrough();
                        case MatchResult_First:
                            return edge;
                        case MatchResult_Both:
                        case MatchResult_Neither:
                            break;
                            switchDefault()
                    }
                }
                return nullptr;
            }

            // finds the next seam edge in counter clockwise orientation
            Edge* findNextSplittingEdge(Edge* last) const {
                assert(last != nullptr);

                HalfEdge* halfEdge = last->firstEdge()->previous();
                Edge* next = halfEdge->edge();

                MatchResult result = matches(next);
                while (result != MatchResult_First && result != MatchResult_Second && next != last) {
                    halfEdge = halfEdge->twin()->previous();
                    next = halfEdge->edge();
                    result = matches(next);
                }

                if (result != MatchResult_First && result != MatchResult_Second) {
                    return nullptr;
                }

                if (result == MatchResult_Second) {
                    next->flip();
                }

                return next;
            }
        private:
            MatchResult matches(const Edge* edge) const {
                const bool firstResult = matches(edge->firstFace());
                const bool secondResult = matches(edge->secondFace());
                if (firstResult && secondResult) {
                    return MatchResult_Both;
                } else if (firstResult) {
                    return MatchResult_First;
                } else if (secondResult) {
                    return MatchResult_Second;
                } else {
                    return MatchResult_Neither;
                }
            }
        public:
            bool matches(const Face* face) const {
                return doMatches(face);
            }
        private:
            virtual bool doMatches(const Face* face) const = 0;
        };

        template <typename T, typename FP, typename VP>
        class Polyhedron<T,FP,VP>::SplitByConnectivityCriterion : public Polyhedron<T,FP,VP>::SplittingCriterion {
        private:
            const Vertex* m_vertex;
        public:
            SplitByConnectivityCriterion(const Vertex* vertex) :
                m_vertex(vertex) {}
        private:
            bool doMatches(const Face* face) const override {
                return !m_vertex->incident(face);
            }
        };

        template <typename T, typename FP, typename VP>
        class Polyhedron<T,FP,VP>::SplitByVisibilityCriterion : public Polyhedron<T,FP,VP>::SplittingCriterion {
        private:
            vm::vec<T,3> m_point;
        public:
            SplitByVisibilityCriterion(const vm::vec<T,3>& point) :
                m_point(point) {}
        private:
            bool doMatches(const Face* face) const override {
                return face->pointStatus(m_point) == vm::plane_status::below;
            }
        };

        template <typename T, typename FP, typename VP>
        typename Polyhedron<T,FP,VP>::Seam Polyhedron<T,FP,VP>::createSeam(const SplittingCriterion& criterion) {
            Seam seam;

            Edge* first = criterion.findFirstSplittingEdge(m_edges);
            if (first != nullptr) {
                Edge* current = first;
                do {
                    assert(current != nullptr);
                    seam.push_back(current);
                    current = criterion.findNextSplittingEdge(current);
                } while (current != first);
            }

            return seam;
        }

        template <typename T, typename FP, typename VP>
        void Polyhedron<T,FP,VP>::split(const Seam& seam, Callback& callback) {
            assert(seam.size() >= 3);
            assert(!seam.hasMultipleLoops());

            // First, unset the second half edge of every seam edge.
            // Thereby remember the second half edge of the first seam edge.
            // Note that all seam edges are oriented such that their second half edge belongs
            // to the portion of the polyhedron that must be removed.
            HalfEdge* first = seam.first()->secondEdge();
            for (Edge* edge : seam) {
                // Set the first edge as the leaving edge. Since the first one will remain
                // in the polyhedron, we can use this as an indicator whether or not to
                // delete a vertex in the call to deleteFaces.
                edge->setFirstAsLeaving();
                edge->unsetSecondEdge();
            }

            // Now we must delete all the faces, edges, and vertices which are above the seam.
            // Since we opened the seam, that is, we unset the 2nd half edge of each seam edge,
            // which belongs to the portion of the polyhedron that will be deleted, the deletion
            // will not touch the faces that should remain in the polyhedron. Additionally, the
            // seam edges will also not be deleted.
            // The first half edge we remembered above is our entry point into that portion of the polyhedron.
            // We must remember which faces we have already visited to stop the recursion.
            std::unordered_set<Face*> visitedFaces;
            VertexList verticesToDelete; // Will automatically delete the vertices when it falls out of scope
            deleteFaces(first, visitedFaces, verticesToDelete, callback);
        }

        template <typename T, typename FP, typename VP> template <typename FaceSet>
        void Polyhedron<T,FP,VP>::deleteFaces(HalfEdge* first, FaceSet& visitedFaces, VertexList& verticesToDelete, Callback& callback) {
            Face* face = first->face();

            // Have we already visited this face?
            if (!visitedFaces.insert(face).second) {
                return;
            }

            // Callback must be called now when the face is still fully intact.
            callback.faceWillBeDeleted(face);

            HalfEdge* current = first;
            do {
                Edge* edge = current->edge();
                if (edge != nullptr) {
                    // This indicates that the current half edge was not part of the seam before
                    // the seam was opened, i.e., it may have a neighbour that should also be deleted.

                    // If the current edge has a neighbour, we can go ahead and delete it.
                    // Once the function returns, the neighbour is definitely deleted unless
                    // we are in a recursive call where that neighbour is being deleted by one
                    // of our callers. In that case, the call to deleteFaces returned immediately.
                    if (edge->fullySpecified()) {
                        deleteFaces(edge->twin(current), visitedFaces, verticesToDelete, callback);
                    }

                    if (edge->fullySpecified()) {
                        // This indicates that we are in a recursive call and that the neighbour across
                        // the current edge is going to be deleted by one of our callers. We open the
                        // edge and unset it so that it is not considered again later.
                        edge->makeSecondEdge(current);
                        edge->unsetSecondEdge();
                    } else {
                        // This indicates that the neighbour across the current edges has already been deleted
                        // or that it will be deleted by one of our callers.
                        // This means that we can safely unset the edge and delete it.
                        current->unsetEdge();
                        m_edges.remove(edge);
                    }
                }

                Vertex* origin = current->origin();
                if (origin->leaving() == current) {
                    // We expect that the vertices on the seam have had a remaining edge
                    // set as their leaving edge before the call to this function.
                    callback.vertexWillBeDeleted(origin);
                    verticesToDelete.splice_back(m_vertices, VertexList::iter(origin), std::next(VertexList::iter(origin)), 1u);
                }
                current = current->next();
            } while (current != first);

            m_faces.remove(face);
        }

        template <typename T, typename FP, typename VP>
        void Polyhedron<T,FP,VP>::sealWithSinglePolygon(const Seam& seam, Callback& callback) {
            assert(seam.size() >= 3);
            assert(!seam.hasMultipleLoops());
            assert(!empty() && !point() && !edge() && !polygon());

            HalfEdgeList boundary;
            for (Edge* seamEdge : seam) {
                assert(!seamEdge->fullySpecified());

                Vertex* origin = seamEdge->secondVertex();
                HalfEdge* boundaryEdge = new HalfEdge(origin);
                boundary.push_back(boundaryEdge);
                seamEdge->setSecondEdge(boundaryEdge);
            }

            Face* face = new Face(std::move(boundary));
            callback.faceWasCreated(face);
            m_faces.push_back(face);
        }

        template <typename T, typename FP, typename VP>
        class Polyhedron<T,FP,VP>::ShiftSeamForWeaving {
        private:
            const vm::vec<T,3> m_position;
        public:
            explicit ShiftSeamForWeaving(const vm::vec<T,3>& position) : m_position(position) {}
        public:
            bool operator()(const Seam& seam) const {
                const auto* last = seam.last();
                const auto* first = seam.first();

                const auto* v1 = last->firstVertex();
                const auto* v2 = last->secondVertex();
                const auto* v3 = first->firstVertex();
                assert(v3 != v1);
                assert(v3 != v2);

                const auto [valid, lastPlane] = vm::from_points(m_position, v1->position(), v2->position());
                assert(valid); unused(valid);

                const auto status = lastPlane.point_status(v3->position());
                return status == vm::plane_status::below;
            }
        };

        template <typename T, typename FP, typename VP>
        typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::weave(Seam seam, const vm::vec<T,3>& position, Callback& callback) {
            assert(seam.size() >= 3);
            assert(!seam.hasMultipleLoops());
            assert(!empty() && !point() && !edge());

            if (!seam.shift(ShiftSeamForWeaving(position))) {
                return nullptr;
            }

            if (polygon()) {
                // When adding a vertex to a large polygon, it can happen that the vertex is so close to the
                // polygon's plane that most woven faces are considered coplanar and fewer than three faces
                // would be created. In this case, we reject and return null.
                size_t faceCount = 0;
                auto it = std::begin(seam);
                while (it != std::end(seam)) {
                    auto* edge = *it++;

                    auto* v1 = edge->secondVertex();
                    auto* v2 = edge->firstVertex();

                    if (it != std::end(seam)) {
                        const auto [valid, plane] = vm::from_points(position, v2->position(), v1->position());
                        assert(valid); unused(valid);

                        auto* next = *it;

                        // TODO use same coplanarity check as in Face::coplanar(const Face*) const ?
                        while (it != std::end(seam) && plane.point_status(next->firstVertex()->position()) == vm::plane_status::inside) {
                            if (++it != std::end(seam)) {
                                next = *it;
                            }
                        }
                    }

                    ++faceCount;
                }

                if (faceCount < 3) {
                    return nullptr;
                }
            }

            auto* top = new Vertex(position);

            HalfEdge* first = nullptr;
            HalfEdge* last = nullptr;

            auto it = std::begin(seam);
            while (it != std::end(seam)) {
                auto* edge = *it++;

                assert(!edge->fullySpecified());
                auto* v1 = edge->secondVertex();
                auto* v2 = edge->firstVertex();

                auto* h1 = new HalfEdge(top);
                auto* h2 = new HalfEdge(v1);
                auto* h3 = new HalfEdge(v2);
                auto* h = h3;

                HalfEdgeList boundary;
                boundary.push_back(h1);
                boundary.push_back(h2);
                boundary.push_back(h3);
                edge->setSecondEdge(h2);

                if (it != std::end(seam)) {
                    const auto [valid, plane] = vm::from_points(top->position(), v2->position(), v1->position());
                    assert(valid); unused(valid);

                    auto* next = *it;

                    // TODO use same coplanarity check as in Face::coplanar(const Face*) const ?
                    while (it != std::end(seam) && plane.point_status(next->firstVertex()->position()) == vm::plane_status::inside) {
                        next->setSecondEdge(h);

                        auto* v = next->firstVertex();
                        h = new HalfEdge(v);
                        boundary.push_back(h);

                        if (++it != std::end(seam)) {
                            next = *it;
                        }
                    }
                }

                Face* newFace = new Face(std::move(boundary));
                callback.faceWasCreated(newFace);
                m_faces.push_back(newFace);

                if (last != nullptr) {
                    m_edges.push_back(new Edge(h1, last));
                }

                if (first == nullptr) {
                    first = h1;
                }
                last = h;
            }

            assert(first->face() != last->face());
            m_edges.push_back(new Edge(first, last));

            m_vertices.push_back(top);
            callback.vertexWasCreated(top);

            return top;
        }
    }
}

#endif
