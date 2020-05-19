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

#ifndef TrenchBroom_Polyhedron_Clip_h
#define TrenchBroom_Polyhedron_Clip_h

#include "Ensure.h"
#include "Macros.h"
#include "Exceptions.h"

#include "Polyhedron.h"

#include <vecmath/plane.h>
#include <vecmath/scalar.h>
#include <vecmath/util.h>

namespace TrenchBroom {
    namespace Model {
        template <typename T, typename FP, typename VP>
        Polyhedron<T,FP,VP>::ClipResult::ClipResult(const Type i_type) :
            type(i_type) {}

        template <typename T, typename FP, typename VP>
        bool Polyhedron<T,FP,VP>::ClipResult::unchanged() const { return type == Type_ClipUnchanged; }

        template <typename T, typename FP, typename VP>
        bool Polyhedron<T,FP,VP>::ClipResult::empty() const     { return type == Type_ClipEmpty; }

        template <typename T, typename FP, typename VP>
        bool Polyhedron<T,FP,VP>::ClipResult::success() const   { return type == Type_ClipSuccess; }

        template <typename T, typename FP, typename VP>
        typename Polyhedron<T,FP,VP>::ClipResult Polyhedron<T,FP,VP>::clip(const vm::plane<T,3>& plane) {
            Callback c;
            return clip(plane, c);
        }

        template <typename T, typename FP, typename VP>
        typename Polyhedron<T,FP,VP>::ClipResult Polyhedron<T,FP,VP>::clip(const vm::plane<T,3>& plane, Callback& callback) {
            const ClipResult vertexResult = checkIntersects(plane);
            if (!vertexResult.success()) {
                return vertexResult;
            }
            
            // The basic idea is now to split all faces which are intersected by the given plane so that the polyhedron
            // can be separated into two halves such that no face has vertices on opposite sides of the plane.
            // Sometimes building a seam fails due to floating point imprecisions. In that case, intersectWithPlane
            // throws a GeometryException which we catch here.
            try {
                const Seam seam = intersectWithPlane(plane, callback);

                // We construct a seam along those edges which are completely inside the plane and delete the half of the
                // polyhedron that is above the plane. The remaining half is an open polyhedron (one face is missing) which
                // is below the plane.
                split(seam, callback);

                // We seal the polyhedron by creating a new face.
                sealWithSinglePolygon(seam, plane, callback);
                updateBounds();

                assert(checkInvariant());

                return ClipResult(ClipResult::Type_ClipSuccess);
            } catch (const NoSeamException& e) {
                /*
                 No seam could be constructed, but the polyhedron may have been modified by splitting
                 some faces. The exception contains the edges connecting the split faces, and now we must
                 merge them again if they are coplanar.
                 */
                for (const Edge* edge : e.splitFaces()) {
                    mergeNeighbours(edge->firstEdge(), nullptr, callback);
                }
                assert(checkInvariant());

                /*
                 We assume that the plane doesn't intersect the polyhedron. The result may either be
                 that the polyhedron remains unchanged or that it becomes empty. We need to look at
                 the vertices to find out exactly.
                 For this, we first find the vertex that is furthest from the given plane, and then
                 examine its point status.
                 */

                auto it = std::max_element(std::begin(m_vertices), std::end(m_vertices),
                    [&plane](const Vertex* lhs, const Vertex* rhs) {
                        const T lhsDist = vm::abs(plane.point_distance(lhs->position()));
                        const T rhsDist = vm::abs(plane.point_distance(rhs->position()));
                        return lhsDist < rhsDist;
                    });

                assert(it != std::end(m_vertices));
                if (plane.point_status((*it)->position()) == vm::plane_status::below) {
                    // The furthest point is below the plane.
                    return ClipResult(ClipResult::Type_ClipUnchanged);
                } else {
                    // The furthest point is above or inside the plane.
                    return ClipResult(ClipResult::Type_ClipEmpty);
                }
            }
        }

        template <typename T, typename FP, typename VP>
        typename Polyhedron<T,FP,VP>::ClipResult Polyhedron<T,FP,VP>::checkIntersects(const vm::plane<T,3>& plane) const {
            std::size_t above = 0u;
            std::size_t below = 0u;
            std::size_t inside = 0u;


            for (const Vertex* currentVertex : m_vertices) {
                const vm::plane_status status = plane.point_status(currentVertex->position());
                switch (status) {
                    case vm::plane_status::above:
                        ++above;
                        break;
                    case vm::plane_status::below:
                        ++below;
                        break;
                    case vm::plane_status::inside:
                        ++inside;
                        break;
                        switchDefault()
                }
            }

            assert(above + below + inside == m_vertices.size());

            if (below + inside == m_vertices.size()) {
                return ClipResult(ClipResult::Type_ClipUnchanged);
            } else if (above + inside == m_vertices.size()) {
                return ClipResult(ClipResult::Type_ClipEmpty);
            } else {
                return ClipResult(ClipResult::Type_ClipSuccess);
            }
        }

        template <typename T, typename FP, typename VP>
        class Polyhedron<T,FP,VP>::NoSeamException : public Exception {
        private:
            std::vector<Edge*> m_splitFaces;
        public:
            NoSeamException(std::vector<Edge*> splitFaces) :
            m_splitFaces(std::move(splitFaces)) {}
            
            const std::vector<Edge*>& splitFaces() const {
                return m_splitFaces;
            }
        };

        template <typename T, typename FP, typename VP>
        typename Polyhedron<T,FP,VP>::Seam Polyhedron<T,FP,VP>::intersectWithPlane(const vm::plane<T,3>& plane, Callback& callback) {
            Seam seam;
            std::vector<Edge*> splitFaces;

            // First, we find a half edge that is intersected by the given plane.
            HalfEdge* initialEdge = findInitialIntersectingEdge(plane);
            ensure(initialEdge != nullptr, "initialEdge is null");

            HalfEdge* currentEdge;
            bool faceWasSplit;
            
            // Now we split the face to which this initial half edge belongs. The call returns the newly inserted edge
            // that connects the (possibly newly inserted) vertices which are now inside of the plane.
            std::tie(currentEdge, faceWasSplit) = intersectWithPlane(initialEdge, plane, callback);
            
            // Keep track of the faces that were split so that we can merge them if no seam can be created.
            if (faceWasSplit) {
                Edge* seamEdge = currentEdge->edge();
                seamEdge->makeSecondEdge(currentEdge);
                splitFaces.push_back(seamEdge);
            }

            // The destination of that edge is the first vertex which we encountered (or inserted) which is inside the plane.
            // This is where our algorithm must stop. When we encounter that vertex again, we have completed the intersection
            // and the polyhedron can now be split in two along the computed seam.
            Vertex* stopVertex = currentEdge->destination();
            do {
                // First we find the next face that is either split by the plane or which has an edge completely in the plane.
                currentEdge = findNextIntersectingEdge(currentEdge, plane);

                // If no edge could be found, then we cannot build a seam because the plane is barely touching the polyhedron.
                if (currentEdge == nullptr) {
                    throw NoSeamException(std::move(splitFaces));
                }

                // Now we split that face. Again, the returned edge connects the two (possibly inserted) vertices of that
                // face which are now inside the plane.
                std::tie(currentEdge, faceWasSplit) = intersectWithPlane(currentEdge, plane, callback);

                // Build a seam while intersecting the polyhedron by remembering the edges we just inserted. To ensure that
                // the seam edges are correctly oriented, we check that the current edge is the second edge, as the current
                // edge belongs to the faces that we are going to clip away.
                Edge* seamEdge = currentEdge->edge();
                seamEdge->makeSecondEdge(currentEdge);

                if (faceWasSplit) {
                    splitFaces.push_back(seamEdge);
                }
                
                // Ensure that the seam remains valid.
                if (!seam.empty() && seamEdge == seam.last()) {
                    throw NoSeamException(std::move(splitFaces));
                }

                seam.push_back(seamEdge);
            } while (currentEdge->destination() != stopVertex);

            return seam;
        }

        template <typename T, typename FP, typename VP>
        typename Polyhedron<T,FP,VP>::HalfEdge* Polyhedron<T,FP,VP>::findInitialIntersectingEdge(const vm::plane<T,3>& plane) const {
            for (const Edge* currentEdge : m_edges) {
                HalfEdge* halfEdge = currentEdge->firstEdge();
                const vm::plane_status os = plane.point_status(halfEdge->origin()->position());
                const vm::plane_status ds = plane.point_status(halfEdge->destination()->position());


                if ((os == vm::plane_status::inside && ds == vm::plane_status::above) ||
                    (os == vm::plane_status::below  && ds == vm::plane_status::above)) {
                    return halfEdge->twin();
                }
                if ((os == vm::plane_status::above  && ds == vm::plane_status::inside) ||
                    (os == vm::plane_status::above  && ds == vm::plane_status::below)) {
                    return halfEdge;
                }

                if (os == vm::plane_status::inside && ds == vm::plane_status::inside) {
                    // If both ends of the edge are inside the plane, we must ensure that we return the correct
                    // half edge, which is either the current one or its twin. Since the returned half edge is supposed
                    // to be clipped away, we must examine the destination of its successor(s). If that is below the plane,
                    // we return the twin, otherwise we return the half edge.
                    HalfEdge* nextEdge = halfEdge->next();
                    vm::plane_status ss = plane.point_status(nextEdge->destination()->position());

                    while (ss == vm::plane_status::inside && nextEdge != halfEdge) {
                        // Due to floating point imprecision, we might run into the case where the successor's destination is
                        // still considered "inside" the plane. In this case, we consider the successor's successor and so on
                        // until we find an edge whose destination is not inside the plane.
                        nextEdge = nextEdge->next();
                        ss = plane.point_status(nextEdge->destination()->position());
                    }

                    if (ss == vm::plane_status::inside) {
                        // We couldn't find a successor whose destination is inside the plane, so we must give up.
                        return nullptr;
                    }

                    if (ss == vm::plane_status::below) {
                        return halfEdge->twin();
                    } else {
                        return halfEdge;
                    }
                }
            }
            return nullptr;
        }

        template <typename T, typename FP, typename VP>
        std::tuple<typename Polyhedron<T,FP,VP>::HalfEdge*, bool> Polyhedron<T,FP,VP>::intersectWithPlane(HalfEdge* firstBoundaryEdge, const vm::plane<T,3>& plane, Callback& callback) {

            // Starting at the given edge, we search the boundary of the incident face until we find an edge that is either split in two by the given plane
            // or where its origin is inside it. In the first case, we split the found edge by inserting a vertex at the position where
            // the plane intersects the edge. We remember the half edge starting at the newly inserted vertex as the seam origin or destination, depending
            // on whether it's the first or second such edge we have found. In the second case (the edge's origin is inside the plane), we just store the
            // half edge as either the seam origin or destination.
            // In the end, we have two vertices, identified by half edges belonging to the currently treated face, which lie inside the plane. If these two
            // vertices aren't already connected by an edge, we split the current face in two by inserting a new edge from the origin to the destination vertex.
            // Finally we must decide where to continue our search, that is, we find a face that is incident to the destination vertex such that it is split
            // by the given plane. We return the half edge of that face's boundary which starts in the destination vertex so that the search can continue there.

            HalfEdge* seamOrigin = nullptr;
            HalfEdge* seamDestination = nullptr;

            HalfEdge* currentBoundaryEdge = firstBoundaryEdge;
            do {
                const vm::plane_status os = plane.point_status(currentBoundaryEdge->origin()->position());
                const vm::plane_status ds = plane.point_status(currentBoundaryEdge->destination()->position());

                if (os == vm::plane_status::inside) {
                    if (seamOrigin == nullptr) {
                        seamOrigin = currentBoundaryEdge;
                    } else {
                        seamDestination = currentBoundaryEdge;
                    }
                    currentBoundaryEdge = currentBoundaryEdge->next();
                } else if ((os == vm::plane_status::below && ds == vm::plane_status::above) ||
                           (os == vm::plane_status::above && ds == vm::plane_status::below)) {
                    // We have to split the edge and insert a new vertex, which will become the origin or destination of the new seam edge.
                    Edge* currentEdge = currentBoundaryEdge->edge();
                    Edge* newEdge = currentEdge->split(plane);
                    m_edges.push_back(newEdge);

                    currentBoundaryEdge = currentBoundaryEdge->next();
                    Vertex* newVertex = currentBoundaryEdge->origin();
                    assert(plane.point_status(newVertex->position()) == vm::plane_status::inside);

                    m_vertices.push_back(newVertex);
                    callback.vertexWasCreated(newVertex);

                    // The newly inserted vertex will be reexamined in the next loop iteration as it is now contained within the plane.
                } else {
                    currentBoundaryEdge = currentBoundaryEdge->next();
                }
            } while (seamDestination == nullptr && currentBoundaryEdge != firstBoundaryEdge);
            ensure(seamOrigin != nullptr, "seamOrigin is null");

            // The plane only touches one vertex of the face.
            if (seamDestination == nullptr) {
                return std::make_tuple(seamOrigin->previous(), false);
            }

            bool faceWasSplit = false;
            if (seamDestination->next() == seamOrigin) {
                using std::swap;
                swap(seamOrigin, seamDestination);
            } else if (seamOrigin->next() != seamDestination) {
                // If the origin and the destination are not already connected by an edge, we must split the current face and insert an edge
                // between them.
                // The newly created faces are supposed to be above the given plane, so we have to consider whether the destination of the
                // seam origin edge is above or below the plane.
                const vm::plane_status os = plane.point_status(seamOrigin->destination()->position());
                assert(os != vm::plane_status::inside);
                if (os == vm::plane_status::below) {
                    intersectWithPlane(seamOrigin, seamDestination, callback);
                } else {
                    intersectWithPlane(seamDestination, seamOrigin, callback);
                }
                faceWasSplit = true;
            }

            return std::make_tuple(seamDestination->previous(), faceWasSplit);
        }

        template <typename T, typename FP, typename VP>
        void Polyhedron<T,FP,VP>::intersectWithPlane(HalfEdge* oldBoundaryFirst, HalfEdge* newBoundaryFirst, Callback& callback) {
            HalfEdge* newBoundaryLast = oldBoundaryFirst->previous();

            HalfEdge* oldBoundarySplitter = new HalfEdge(newBoundaryFirst->origin());
            HalfEdge* newBoundarySplitter = new HalfEdge(oldBoundaryFirst->origin());

            Face* oldFace = oldBoundaryFirst->face();
            oldFace->insertIntoBoundaryAfter(newBoundaryLast, HalfEdgeList({ newBoundarySplitter }));
            HalfEdgeList newBoundary = oldFace->replaceBoundary(newBoundaryFirst, newBoundarySplitter, HalfEdgeList({ oldBoundarySplitter }));

            Face* newFace = new Face(std::move(newBoundary), oldFace->plane());
            Edge* newEdge = new Edge(oldBoundarySplitter, newBoundarySplitter);

            m_edges.push_back(newEdge);
            m_faces.push_back(newFace);

            callback.faceWasSplit(oldFace, newFace);
        }

        /*
         Searches all edges leaving searchFrom's destination for an edge that is intersected by the given plane.
         */
        template <typename T, typename FP, typename VP>
        typename Polyhedron<T,FP,VP>::HalfEdge* Polyhedron<T,FP,VP>::findNextIntersectingEdge(HalfEdge* searchFrom, const vm::plane<T,3>& plane) const {
            HalfEdge* currentEdge = searchFrom->next();
            HalfEdge* stopEdge = searchFrom->twin();
            do {
                assert(currentEdge != stopEdge);

                // Select two vertices that form a triangle (of an adjacent face) together with currentEdge's origin vertex.
                // If either of the two vertices is inside the plane or if they lie on different sides of it, then we have found
                // the next face to handle.

                Vertex* cd = currentEdge->destination();
                Vertex* po = currentEdge->previous()->origin();
                const vm::plane_status cds = plane.point_status(cd->position());
                const vm::plane_status pos = plane.point_status(po->position());

                if ((cds == vm::plane_status::inside) ||
                    (cds == vm::plane_status::below && pos == vm::plane_status::above) ||
                    (cds == vm::plane_status::above && pos == vm::plane_status::below)) {
                    return currentEdge;
                }

                currentEdge = currentEdge->twin()->next();
            } while (currentEdge != stopEdge);
            return nullptr;
        }
    }
}

#endif
