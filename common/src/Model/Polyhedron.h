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

#ifndef TrenchBroom_Polyhedron_h
#define TrenchBroom_Polyhedron_h

#include "Allocator.h"

#include "Polyhedron_Forward.h"

#include <kdl/intrusive_circular_list.h>

#include <vecmath/forward.h>
#include <vecmath/vec.h>
#include <vecmath/bbox.h>
#include <vecmath/util.h>

#include <initializer_list>
#include <limits>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        /* ====================== Implementation in Polyhedron_Vertex.h ====================== */

        /**
         * Maps a vertex to its contained kdl::intrusive_circular_link member, used for intrusive_circular_list.
         */
        template <typename T, typename FP, typename VP>
        struct Polyhedron_GetVertexLink {
            kdl::intrusive_circular_link<Polyhedron_Vertex<T,FP,VP>>& operator()(Polyhedron_Vertex<T,FP,VP>* vertex) const;
            const kdl::intrusive_circular_link<Polyhedron_Vertex<T,FP,VP>>& operator()(const Polyhedron_Vertex<T,FP,VP>* vertex) const;
        };

        /**
         * A vertex of a polyhedron.
         *
         * Each vertex of a polyhedron has a position, a leaving half edge, a link to its previous and next neighbours in
         * the containing intrusive circular list, and a payload.
         *
         * The leaving half edge of a vertex is any half edge that has the vertex as its origin. It is used to find
         * the incident faces of a vertex.
         *
         * The payload of a vertex can be used to store user data.
         */
        template <typename T, typename FP, typename VP>
        class Polyhedron_Vertex : public Allocator<Polyhedron_Vertex<T,FP,VP>> {
        private:
            friend class Polyhedron<T,FP,VP>;
            friend class Polyhedron_Edge<T,FP,VP>;
            friend class Polyhedron_HalfEdge<T,FP,VP>;
            friend class Polyhedron_Face<T,FP,VP>;
            friend struct Polyhedron_GetVertexLink<T,FP,VP>;

            using Vertex = Polyhedron_Vertex<T,FP,VP>;
            using HalfEdge = Polyhedron_HalfEdge<T,FP,VP>;
            using Face = Polyhedron_Face<T,FP,VP>;
        private:
            /**
             * The vertex position.
             */
            vm::vec<T,3> m_position;

            /**
             * A half edge that originates at this vertex.
             */
            HalfEdge* m_leaving;

            /**
             * The kdl::intrusive_circular_link member required to put vertices in an intrusive_circular_list.
             */
            kdl::intrusive_circular_link<Vertex> m_link;

            /**
             * A payload data item that can be set on this vertex.
             */
            typename VP::Type m_payload;
        private:
            /**
             * Creates a new vertex at the given position. The leaving half edge will be null.
             *
             * @param position the position of the new vertex
             */
            explicit Polyhedron_Vertex(const vm::vec<T,3>& position);
        public:
            /**
             * Returns the position of this vertex.
             */
            const vm::vec<T,3>& position() const;

            /**
             * Sets the position of this vertex.
             *
             * @param position the position to set
             */
            void setPosition(const vm::vec<T,3>& position);

            /**
             * Returns the leaving half edge assigned to this vertex.
             */
            HalfEdge* leaving() const;

            /**
             * Sets the leaving half edge for this vertex. The given edge must not be null and its origin must be
             * this vertex.
             *
             * @param edge the half edge to set
             */
            void setLeaving(HalfEdge* edge);

            /**
             * Returns the next vertex in its containing circular list.
             */
            Vertex* next() const;

            /**
             * Returns the previous vertex in its containing circular list.
             */
            Vertex* previous() const;

            /**
             * Returns the payload assigned to this vertex.
             */
            typename VP::Type payload() const;

            /**
             * Sets the payload assigned to this vertex.
             *
             * @param payload the payload to set
             */
            void setPayload(typename VP::Type payload);

            /**
             * Indicates whether the given face is incident to this vertex.
             *
             * @param face the face to check, must not be null
             * @return true if this vertex is incident to the given face and false otherwise
             */
            bool incident(const Face* face) const;

            /**
             * Rounds each component of this vertices' position to the nearest integer if the distance of the component's
             * value to that integer is less  than the given epsilon value. Furthermore, the component value is rounded
             * such that at most the given number of decimals are retained.
             *
             * @param decimals the number of decimals to retain
             * @param epsilon an epsilon value
             */
            void correctPosition(const size_t decimals = 0, const T epsilon = vm::constants<T>::correct_epsilon());
        };

        /* ====================== Implementation in Polyhedron_Edge.h ====================== */

        /**
         * Maps an edge to its contained kdl::intrusive_circular_link member, used for intrusive_circular_list.
         */
        template <typename T, typename FP, typename VP>
        struct Polyhedron_GetEdgeLink {
            kdl::intrusive_circular_link<Polyhedron_Edge<T,FP,VP>>& operator()(Polyhedron_Edge<T,FP,VP>* edge) const;
            const kdl::intrusive_circular_link<Polyhedron_Edge<T,FP,VP>>& operator()(const Polyhedron_Edge<T,FP,VP>* edge) const;
        };

        /**
         * And edge of a polyhedron.
         *
         * Each edge consists of two half edges with opposite directions. These half edges belong to adjacent faces
         * that share the edge. During execution of some algorithms, edges may be underspecified, meaning that they
         * have only one half edge. If both half edges are set, then this edge is called fully specified.
         *
         * The naming of the contained half edges as first or second does not imply any precedence, but it is
         * sometimes used to distinguish the half edges in algorithms. For example, when splitting a polyhedron along
         * a seam, the first and second half edge will be used to determine which part of the polyhedron is deleted
         * and which part is retained.
         *
         * Furthermore, an edge has a link to its previous and next neighbours in the containing intrusive circular
         * list.
         */
        template <typename T, typename FP, typename VP>
        class Polyhedron_Edge : public Allocator<Polyhedron_Edge<T,FP,VP>> {
        private:
            friend class Polyhedron<T,FP,VP>;
            friend class Polyhedron_Vertex<T,FP,VP>;
            friend class Polyhedron_HalfEdge<T,FP,VP>;
            friend class Polyhedron_Face<T,FP,VP>;
            friend struct Polyhedron_GetEdgeLink<T,FP,VP>;

            using Vertex = Polyhedron_Vertex<T,FP,VP>;
            using Edge = Polyhedron_Edge<T,FP,VP>;
            using HalfEdge = Polyhedron_HalfEdge<T,FP,VP>;
            using Face = Polyhedron_Face<T,FP,VP>;
        private:
            /**
             * The first half edge.
             */
            HalfEdge* m_first;

            /**
             * The second half edge.
             */
            HalfEdge* m_second;

            /**
             * The kdl::intrusive_circular_link member required to put vertices in an intrusive_circular_list.
             */
            kdl::intrusive_circular_link<Edge> m_link;
        private:
            /**
             * Creates a new edge with the given half edges.
             *
             * @param first the first half edge, must not be null
             * @param second the second half edge, may be null
             */
            Polyhedron_Edge(HalfEdge* first, HalfEdge* second = nullptr);
        public:
            /**
             * Returns the origin of the first half edge.
             */
            Vertex* firstVertex() const;

            /**
             * Returns the origin of the second half edge if one is set. Otherwise returns the destination of the
             * first half edge.
             */
            Vertex* secondVertex() const;

            /**
             * Returns the first half edge.
             */
            HalfEdge* firstEdge() const;

            /**
             * Returns the second half edge. Behavior is unspecified if the second half edge is unset.
             */
            HalfEdge* secondEdge() const;

            /**
             * Returns the twin of the given half edge, which must be one of the half edges of this edge. If a half
             * edge is passed that doesn't belong to this edge, the behavior is unspecified.
             *
             * @param halfEdge the half edge of which the twin should be returned
             * @return the second half edge if given the first half edge, and the first half edge if given the second
             * half edge
             */
            HalfEdge* twin(const HalfEdge* halfEdge) const;

            /**
             * Returns the vector from the position of the first vertex to the position of the second vertex.
             * The behavior is unspecified if this edge is not fully specified.
             */
            vm::vec<T,3> vector() const;

            /**
             * Returns the center between the position of the first vertex and the position of the second vertex.
             * The behavior is unspecified if this edge is not fully specified.
             */
            vm::vec<T,3> center() const;

            /**
             * Returns the face to which the first half edge belongs.
             */
            Face* firstFace() const;

            /**
             * Returns the face to which the second half edge belongs. The behavior is unspecified if no second half
             * edge is set.
             */
            Face* secondFace() const;

            /**
             * Indicates whether the given vertex is either the first or the second vertex of this edge.
             *
             * @param vertex the vertex to check, which must not be null
             * @return true if the given vertex is either the first or the second vertex of this edge, and false
             * otherwise
             */
            bool hasVertex(const Vertex* vertex) const;

            /**
             * Indicates whether this edge has a vertex with the given position, up to the given epsilon value.
             *
             * @param position the position to check
             * @param epsilon the epsilon value to use for equality checks
             * @return true if this edge has a vertex with the given position
             */
            bool hasPosition(const vm::vec<T,3>& position, T epsilon = static_cast<T>(0.0)) const;

            /**
             * Indicates whether this edge has the given positions for its first and second vertex, up to the given
             * epsilon value. The order of the given positions does not matter.
             *
             * @param position1 the first position to check
             * @param position2 the second position to check
             * @param epsilon the epsilon value to use for equality checks
             * @return true if this edge has both of the given positions for its first and second vertex and false
             * otherwise
             */
            bool hasPositions(const vm::vec<T,3>& position1, const vm::vec<T,3>& position2, T epsilon = static_cast<T>(0.0)) const;

            /**
             * Computes the maximum of the minimal distances of each of the positions this edge's vertices and the
             * given two points. The result is the maximum of two distances d1 and d2, where
             *
             * - d1 is the minimum of the distances between the positions of this edge's vertices and position1
             * - d2 is the minimum of the distances between the positions of this edge's vertices and position2
             *
             * If this edge is not fully specified, the behavior is unspecified.
             *
             * @param position1 the first point
             * @param position2 the second point
             * @return the maximum of the distances d1 and d2 as specified above
             */
            T distanceTo(const vm::vec<T,3>& position1, const vm::vec<T,3>& position2) const;

            /**
             * Indicates whether this edge is fully specified, that is, its second half edge is set.
             *
             * @return true if this edge is fully specified and false otherwise
             */
            bool fullySpecified() const;

            /**
             * Returns the next edge in its containing circular list.
             */
            Edge* next() const;

            /**
             * Returns the previous edge in its containing circular list.
             */
            Edge* previous() const;
        private:
            /**
             * Splits this edge by inserting a new vertex at the position where this edge intersects the given plane.
             *
             * The newly created vertices' position will be the point at which the line segment defined by this
             * edge's vertices' positions intersects the given plane.

             * This function assumes that the vertices of this edge are on opposite sides of the given plane.
             *
             * @param plane the plane at which to split this edge
             * @return the newly created edge
             */
            Edge* split(const vm::plane<T,3>& plane);

            /**
             * Inserts a new vertex at the given position into this edge, creating two new half edges, and a new edge.
             * The newly created half edges are added to the boundaries of the corresponding faces, but the newly
             * created vertex and edge must be stored in their respective containing circular lists.
             *
             * The newly created vertex will be the origin of the newly created edge's first half edge.
             *
             * The following diagram illustrates the effects of this function
             *
             * Before calling this function, this edge looks as follows
             *
             * /\------------old1st----------->/\
             * \/<-----------new2nd------------\/
             * |                               |
             * 1st vertex                      2nd vertex
             *
             * Suppose that the given plane intersects this edge at its center, then the result will look as follows.
             *
             *   |-this edge--|  |--new edge--|
             *   |            |  |            |
             * /\----old1st--->/\----new1st--->/\
             * \/<---new2nd----\/----old2nd----\/
             * |               |               |
             * 1st vertex      new vertex      2nd vertex
             *
             * @param position the positition of the newly created vertex
             * @return the newly created edge
             */
            Edge* insertVertex(const vm::vec<T,3>& position);

            /**
             * Flips this edge by swapping its first and second half edges.
             */
            void flip();

            /**
             * Ensures that the given half edge is the first half edge of this edge. If the given edge is null, or
             * if it is neither the first nor the second half edge of this edge, the behavior is unspecfied. If the
             * given half edge already is the first half edge of this edge, then nothing happens. Otherwise, this
             * edge is flipped and thereby the given half edge is made the first half edge of this edge.
             *
             * @param edge the half edge to make the first half edge of this edge
             */
            void makeFirstEdge(HalfEdge* edge);

            /**
             * Ensures that the given half edge is the second half edge of this edge. If the given edge is null, or
             * if it is neither the first nor the second half edge of this edge, the behavior is unspecfied. If the
             * given half edge already is the second half edge of this edge, then nothing happens. Otherwise, this
             * edge is flipped and thereby the given half edge is made the second half edge of this edge.
             *
             * @param edge the half edge to make the second half edge of this edge
             */
            void makeSecondEdge(HalfEdge* edge);

            /**
             * Ensures that the first half edge of this edge is the leaving half edge of its origin vertex.
             */
            void setFirstAsLeaving();

            /**
             * Sets the second half edge of this edge to null. Assumes that this edge is fully specified, otherwise
             * the behavior is unspecified.
             */
            void unsetSecondEdge();

            /**
             * Sets the second half edge of this edge. Assumes that this edge is not fully specified, otherwise the
             * behavior is undefined. Furthermore, the given half edge must not be associated with any other edge.
             *
             * @param second the half edge to set as the second half edge of this edge, must not be null
             */
            void setSecondEdge(HalfEdge* second);
        };

        /* ====================== Implementation in Polyhedron_HalfEdge.h ====================== */

        /**
         * Maps a half edge to its contained kdl::intrusive_circular_link member, used for intrusive_circular_list.
         */
        template <typename T, typename FP, typename VP>
        struct Polyhedron_GetHalfEdgeLink {
            kdl::intrusive_circular_link<Polyhedron_HalfEdge<T,FP,VP>>& operator()(Polyhedron_HalfEdge<T,FP,VP>* halfEdge) const;
            const kdl::intrusive_circular_link<Polyhedron_HalfEdge<T,FP,VP>>& operator()(const Polyhedron_HalfEdge<T,FP,VP>* halfEdge) const;
        };

        /**
         * A half edge of a polyhedron. Every edge of a polyhedron is made up of two half edges, each of which belongs
         * to the two faces containing the edge. The half edges belonging to a face make up its boundary.
         *
         * Each half edge has an origin vertex, the edge to which the half edge belongs, and the face to which it
         * belongs. The origin vertex may have a pointer to this half edge if it was set as the leaving half edge
         * of that vertex.
         *
         * Furthermore, an edge has a link to its previous and next neighbours in the containing intrusive circular
         * list.
         *
         * The destination vertex of a half edge is the vertex at which the half edge ends and where its successor
         * in the boundary of the containing face originates.
         *
         * If this half edge is part of a fully specified edge, then the other half edge of that edge is called
         * the twin of this half edge.
         *
         * A half edge is stored in an intrusive circular list that belongs to the face whose boundary the half edge
         * belongs to.
         */
        template <typename T, typename FP, typename VP>
        class Polyhedron_HalfEdge : public Allocator<Polyhedron_HalfEdge<T,FP,VP>> {
        private:
            friend class Polyhedron<T,FP,VP>;
            friend class Polyhedron_Vertex<T,FP,VP>;
            friend class Polyhedron_Edge<T,FP,VP>;
            friend class Polyhedron_Face<T,FP,VP>;
            friend struct Polyhedron_GetHalfEdgeLink<T,FP,VP>;

            using Vertex = Polyhedron_Vertex<T,FP,VP>;
            using Edge = Polyhedron_Edge<T,FP,VP>;
            using HalfEdge = Polyhedron_HalfEdge<T,FP,VP>;
            using Face = Polyhedron_Face<T,FP,VP>;
        private:
            /**
             * The origin vertex of this half edge.
             */
            Vertex* m_origin;

            /**
             * The edge to which this half edge belongs.
             */
            Edge* m_edge;

            /**
             * The face whose boundary this half edge belongs to.
             */
            Face* m_face;

            /**
             * The kdl::intrusive_circular_link member required to put half edges in an intrusive_circular_list.
             */
            kdl::intrusive_circular_link<HalfEdge> m_link;
        private:
            /**
             * Creates a new half edge with the given vertex as its origin. This half edge will be set as the leaving
             * half edge of the given vertex.
             *
             * @param origin the origin vertex, must not be null
             */
            Polyhedron_HalfEdge(Vertex* origin);
        public:
            /**
             * The destructor resets the leaving half edge of its origin vertex to null if this is the leading half
             * edge of the origin vertex.
             */
            ~Polyhedron_HalfEdge();
        public:
            /**
             * Returns the origin vertex of this half edge.
             */
            Vertex* origin() const;

            /**
             * Returns the destination vertex of this half edge, which is the vertex where its successor originates.
             */
            Vertex* destination() const;

            /**
             * Returns the edge to which this half edge belongs, which can be null.
             */
            Edge* edge() const;

            /**
             * Returns the face to which this half edge belongs, which can be null.
             */
            Face* face() const;

            /**
             * Returns the next half edge in its containing circular list.
             */
            HalfEdge* next() const;

            /**
             * Returns the previous half edge in its containing circular list.
             */
            HalfEdge* previous() const;

            /**
             * Returns a vector from the position of this half edge's origin to the position of its destination
             * vertex.
             */
            vm::vec<T,3 >vector() const;

            /**
             * Returns the twin of this half edge, that is, the other half edge of the edge to which this half edge
             * belongs.
             *
             * If this half edge does not belong to an edge, the behavior is unspecified. If the corresponding edge
             * is not fully specified, then null is returned.
             */
            HalfEdge* twin() const;

            /**
             * Returns the next incident half edge, that is, the next half edge, in counter clockwise order, that
             * originates at the origin of this half edge.
             *
             * This function is used to enumerate the faces incident to a vertex.
             *
             * @return the next incident half edge
             */
            HalfEdge* nextIncident() const;

            /**
             * Returns the previous incident half edge, that is, the previous half edge, in counter clockwise order,
             * that originates at the origin of this half edge.
             *
             * This function is used to enumerate the faces incident to a vertex.
             *
             * @return the previous incident half edge
             */
            HalfEdge* previousIncident() const;

            /**
             * Indicates whether the positions of the origin vertices of this half edge and its successors have the
             * given positions. The first position is compared to the position of this half edge's origin, the next
             * given position is compared to the position of this half edge's successor's origin, and so on.
             *
             * @param positions the positions to check
             * @param epsilon an epsilon value for comparing the vertex positions agains the given positions
             * @return true if the given positions match the positions of the origin vertices, and false otherwise
             */
            bool hasOrigins(const std::vector<vm::vec<T,3>>& positions, T epsilon = static_cast<T>(0.0)) const;
        private:
            /**
             * Determines the relative location of the given point and a plane p that is defined as follows.
             *
             * Let u be the normalized vector of this half edge (see vector() member function).
             * Let p.normal be the cross product of u and the given normal.
             * Let p.anchor be the position of this half edge's origin.
             *
             * @param normal the normal vector, this is expected to be a unit vector
             * @param point the point to check
             * @return the relative location of the given point and the plane p
             */
            vm::plane_status pointStatus(const vm::vec<T,3>& normal, const vm::vec<T,3>& point) const;

            /**
             * Determines whether this half edge is colinear to the given half edge, which is expected to have this
             * half edge's destination as its origin.
             *
             * @param other the half edge to check against, must not be null and must not be identical to this half
             * edge
             * @return true if this half edge is colinear with the given half edge
             */
            bool colinear(const HalfEdge* other) const;

            /**
             * Sets the origin of this half edge and sets this as the leaving half edge of the given vertex.
             *
             * @param origin the origin to set, which must not be null
             */
            void setOrigin(Vertex* origin);

            /**
             * Sets the edge to which this half edge belongs. This half edge must not already belong to an edge when
             * this function is called.
             *
             * @param edge the edge to set, must not be null
             */
            void setEdge(Edge* edge);

            /**
             * Sets the edge of this half edge to null. Assumes that this half edge does belong to an edge when this
             * function is called.
             */
            void unsetEdge();

            /**
             * Sets the face whose boundary this half edge belongs to. This half edge must not already belong to the
             * boundary of any other face when this function is called.
             *
             * @param face the face to set, must not be null
             */
            void setFace(Face* face);

            /**
             * Sets the face to which this half edge belongs to null. Assumes that this half edge does belong to a
             * face when this function is called.
             */
            void unsetFace();

            /**
             * Sets this half edge as the leaving half edge of its origin vertex.
             */
            void setAsLeaving();
        };

        /* ====================== Implementation in Polyhedron_Face.h ====================== */

        /**
         * Maps a face to its contained kdl::intrusive_circular_link member, used for intrusive_circular_list.
         */
        template <typename T, typename FP, typename VP>
        struct Polyhedron_GetFaceLink {
            kdl::intrusive_circular_link<Polyhedron_Face<T,FP,VP>>& operator()(Polyhedron_Face<T,FP,VP>* face) const;
            const kdl::intrusive_circular_link<Polyhedron_Face<T,FP,VP>>& operator()(const Polyhedron_Face<T,FP,VP>* face) const;
        };

        /**
         * A face of a polyhedron. Each face has a boundary that is a circular list of half edges (in counter
         * clockwise order) and a payload that can be used to attach some user data to the face.
         *
         * Furthermore, a face has a link to its previous and next neighbours in the containing intrusive circular
         * list.
         */
        template <typename T, typename FP, typename VP>
        class Polyhedron_Face : public Allocator<Polyhedron_Face<T,FP,VP>> {
        private:
            friend class Polyhedron<T,FP,VP>;
            friend class Polyhedron_Vertex<T,FP,VP>;
            friend class Polyhedron_Edge<T,FP,VP>;
            friend class Polyhedron_HalfEdge<T,FP,VP>;
            friend struct Polyhedron_GetFaceLink<T,FP,VP>;

            using Vertex = Polyhedron_Vertex<T,FP,VP>;
            using Edge = Polyhedron_Edge<T,FP,VP>;
            using HalfEdge = Polyhedron_HalfEdge<T,FP,VP>;
            using Face = Polyhedron_Face<T,FP,VP>;

            using HalfEdgeList = Polyhedron_HalfEdgeList<T,FP,VP>;
        private:
            /**
             * The boundary of this face. The boundary of a face is a circular list of half edges, usually three or
             * more (but in some cases less if the face is degenerate).
             */
            HalfEdgeList m_boundary;

            /**
             * The payload attached to this face.
             */
            typename FP::Type m_payload;

            /**
             * The kdl::intrusive_circular_link member required to put half edges in an intrusive_circular_list.
             */
            kdl::intrusive_circular_link<Face> m_link;
        private:
            /**
             * Creates a new face with the given boundary and sets the face of each of the boundary's half edges
             * to this. The given boundary is moved into this face.
             *
             * @param boundary the boundary of the newly created face, which must contain at least three half edges
             */
            explicit Polyhedron_Face(HalfEdgeList&& boundary);
        public:
            /**
             * Returns the circular list of half edges that make up the boundary of this face.
             */
            const HalfEdgeList& boundary() const;

            /**
             * Returns the circular list of half edges that make up the boundary of this face.
             */
            HalfEdgeList& boundary();

            /**
             * Returns the next face in its containing circular list.
             */
            Face* next() const;

            /**
             * Returns the next face in its containing circular list.
             */
            Face* previous() const;

            /**
             * Returns the payload assigned to this vertex.
             */
            typename FP::Type payload() const;

            /**
             * Sets the payload assigned to this vertex.
             *
             * @param payload the payload to set
             */
            void setPayload(typename FP::Type payload);

            /**
             * Returns the number of vertices of this face. This is identical to the size of its boundary.
             */
            size_t vertexCount() const;

            /**
             * Returns the half edge of this face's boundary whose origin's position is equal to the given origin,
             * up to the given epsilon.
             *
             * @param origin the position of the origin to find
             * @param epsilon the epsilon value to use for comparison
             * @return the half edge whose origin has the given position, or null if no such half edge exists in
             * this face's boundary
             */
            const HalfEdge* findHalfEdge(const vm::vec<T,3>& origin, T epsilon = static_cast<T>(0.0)) const;

            /**
             * Finds an edge that is adjacent to this face whose first and second vertex have the given positions,
             * up to the given epsilon.
             *
             * @param first the first position to find
             * @param second the second position to find
             * @param epsilon the epsilon value to use for comparison
             * @return the edge whose vertices have the given positions, or null if no such edge is adjacent to
             * this face
             */
            const Edge* findEdge(const vm::vec<T,3>& first, const vm::vec<T,3>& second, T epsilon = static_cast<T>(0.0)) const;

            /**
             * Returns the position of the origin of the first half edge in this face's boundary.
             */
            vm::vec<T,3> origin() const;

            /**
             * Returns a list of the positions of this face's vertices.
             */
            std::vector<vm::vec<T,3>> vertexPositions() const;

            /**
             * Indicates whether this face has a vertex with the given position, up to the given epsilon.
             *
             * @param position the position to check
             * @param epsilon the epsilon value to use for comparison
             * @return true if this face has a vertex with the given position and false otherwise
             */
            bool hasVertexPosition(const vm::vec<T,3>& position, T epsilon = static_cast<T>(0.0)) const;

            /**
             * Indicates whether this face has the given vertex positions. Thereby, the first given position is
             * compared to the position of the origin of the first half edge of this face's boundary, the next given
             * position is compared to its successor, and so on.
             *
             * @param positions the positions to check
             * @param epsilon the epsilon value to use for comparison
             * @return true if this face has the given vertex positions and false otherwise
             */
            bool hasVertexPositions(const std::vector<vm::vec<T,3>>& positions, T epsilon = static_cast<T>(0.0)) const;

            /**
             * Returns the maximum distance between this face's vertices and the given positions. First, the algorithm
             * finds the vertex with the minimal distance to the first given position.
             *
             * If no vertex is within the given max distance of the given points, then the given max distance is
             * returned.
             *
             * Then it proceeds to compute the distance of the succeeding vertices (in the order in which the vertices
             * appear in the boundary) to the succeeding given positions, and takes the maximum of all computed
             * distances and returns it.
             *
             * @param positions the positions to compute the distance to
             * @param maxDistance the max distance
             * @return the maximum distance as computed
             */
            T distanceTo(const std::vector<vm::vec<T,3>>& positions, T maxDistance = std::numeric_limits<T>::max()) const;

            /**
             * Returns the normal of this face. The normal is computed by finding three non colinear vertices in the
             * boundary of this face, and taking the cross product of the vectors between these vertices.
             *
             * If no colinear vertices are found, then a zero vector is returned.
             *
             * @return the normal of this face
             */
            vm::vec<T,3> normal() const;

            /**
             * Returns the center of this face.
             */
            vm::vec<T,3> center() const;

            /**
             * Intersects this face with the given ray and returns the distance along the given ray's direction, from
             * the given ray's origin to the point of intersection, or NaN if the given reay does not intersect this
             * face.
             *
             * The given side determines whether hits from above or below this face should be returned. A hit is
             * considered from above if the ray's origin is above this face (the face normal points towards it) and
             * it is considered from below if the ray's origin is below this face (the face normal points away from
             * it).
             *
             * @param ray the ray
             * @param side the side(s) from which a hit should be considered
             * @return the distance to the point of intersection or NaN if the given ray does not intersect this face
             * from the given side
             */
            T intersectWithRay(const vm::ray<T,3>& ray, const vm::side side) const;

            /**
             * Computes the position of the given point in relation to the plane on which this face lies. This plane
             * is determined by the position of the origin of the first half edge of this face and the face normal.
             *
             * @param point the point to check
             * @param epsilon the epsilon value to use for the position check
             * @return the relative position of the given point
             */
            vm::plane_status pointStatus(const vm::vec<T,3>& point, T epsilon = vm::constants<T>::point_status_epsilon()) const;
        private:
            /**
             * Checks whether this face is coplanar with the given face, that is, if both faces lie in the same plane.
             *
             * @param other the other face
             * @return true if the faces are coplanar and false otherwise
             */
            bool coplanar(const Face* other) const;

            /**
             * Checks whether all vertices of this face lie in the given plane.
             *
             * @param plane the plane to check
             * @return true if all vertices of this face lie on the given plane, and false otherwise
             */
            bool verticesOnPlane(const vm::plane<T,3>& plane) const;

            /**
             * Flips this face by reversing the order of its half edges.
             */
            void flip();

            /**
             * Inserts the given circular list of edges into the boundary of this face after the given half edge.
             * After insertion, the given list of edges will be empty.
             *
             * If the given half edge does not belong to this face's boundary, the behavior is unspecified.
             *
             * @tparam H the type of the given list of edges
             * @param after the half edge after which the given half edges should be inserted
             * @param edges the half edges to insert
             */
            template <typename H>
            void insertIntoBoundaryAfter(HalfEdge* after, H&& edges);

            /**
             * Removes the range [from, to] of half edges from this face's boundary. If either of the given half
             * edges does not belong to this face's boundary, the behavior is unspecified.
             *
             * @param from the first half edge to remove
             * @param to the last half edge to remove (inclusive)
             * @return a circular list containing the removed half edges
             */
            HalfEdgeList removeFromBoundary(HalfEdge* from, HalfEdge* to);

            /**
             * Removes the given edge from this face's boundary. This is equivalent to calling
             *
             * removeFromBoundary(edge, edge);
             *
             * @param edge the half edge to remove
             * @return a circular list containing the removed half edge
             */
            HalfEdgeList removeFromBoundary(HalfEdge* edge);

            /**
             * Replaces the range [from, to] of half edges of this face's boundary with the half edges in the given
             * circular list. After replacement, the given circular list will be empty.
             *
             * If the given range of half edges does not belong to this face's boundary, the behavior is unspecified.
             *
             * @tparam H the type of the given list of edges
             * @param from the first half edge to replace
             * @param to the last edge to replace (inclusive)
             * @param with the half edges to insert
             * @return a circular list containing the replaced half edges
             */
            template <typename H>
            HalfEdgeList replaceBoundary(HalfEdge* from, HalfEdge* to, H&& with);

            /**
             * Counts number of half edges in the range [from, to] and sets their face to the given face.
             *
             * @param from the first half edge to count
             * @param to the last half edge to count (inclusive)
             * @param face the face to set for the given range of half edges
             * @return the number of half edges in the given range
             */
            size_t countAndSetFace(HalfEdge* from, HalfEdge* to, Face* face);

            /**
             * Counts number of half edges in the range [from, to] and sets their face to null.
             *
             * @param from the first half edge to count
             * @param to the last half edge to count (inclusive)
             * @return the number of half edges in the given range
             */
            size_t countAndUnsetFace(HalfEdge* from, HalfEdge* to);

            /**
             * Counts the number of vertices which are shared between this face and the given face. Two faces share
             * a vertex if this vertex is the origin of a half edge of both faces.
             *
             * @param other the other face, which must not be null and it must not be identical to this face
             * @return the number of shared vertices
             */
            size_t countSharedVertices(const Face* other) const;

            /**
             * The result of intersecting a polyhedron face with a ray.
             */
            class RayIntersection;

            /**
             * Intersects this face with the given ray and returns the result.
             */
            RayIntersection intersectWithRay(const vm::ray<T,3>& ray) const;
        };

        template <typename T, typename FP, typename VP>
        class Polyhedron {
        public:
            using FloatType = T;
            using FacePayloadType = FP;
            using VertexPayloadType = VP;
        private:
            static constexpr const auto MinEdgeLength = T(0.01);
        public:
            using Vertex = Polyhedron_Vertex<T,FP,VP>;
            using Edge = Polyhedron_Edge<T,FP,VP>;
            using HalfEdge = Polyhedron_HalfEdge<T,FP,VP>;
            using Face = Polyhedron_Face<T,FP,VP>;
        private:
            using VertexLink = kdl::intrusive_circular_link<Vertex>;
            using EdgeLink = kdl::intrusive_circular_link<Edge>;
            using HalfEdgeLink = kdl::intrusive_circular_link<HalfEdge>;
            using FaceLink = kdl::intrusive_circular_link<Face>;
        public:
            using VertexList = Polyhedron_VertexList<T,FP,VP>;
            using EdgeList = Polyhedron_EdgeList<T,FP,VP>;
            using HalfEdgeList = Polyhedron_HalfEdgeList<T,FP,VP>;
            using FaceList = Polyhedron_FaceList<T,FP,VP>;
        public:
            /**
             * Helper that maps a vertex to its position or a half edge to the position of its origin.
             */
            struct GetVertexPosition {
                const vm::vec<T,3>& operator()(const Vertex* vertex) const;
                const vm::vec<T,3>& operator()(const HalfEdge* halfEdge) const;
            };

            /**
             * A callback with different lifetime events that occur while a polyhedron is modified.
             */
            class Callback {
            public:
                virtual ~Callback();
            public:
                /**
                 * Called after a new vertex was created.
                 *
                 * @param vertex the newly created vertex
                 */
                virtual void vertexWasCreated(Vertex* vertex);

                /**
                 * Called before a vertex is deleted.
                 *
                 * @param vertex the vertex that will be deleted
                 */
                virtual void vertexWillBeDeleted(Vertex* vertex);

                /**
                 * Called after a vertex was added to this polyhedron.
                 *
                 * @param vertex the newly added vertex
                 */
                virtual void vertexWasAdded(Vertex* vertex);

                /**
                 * Called before a vertex will be removed from this polyhedron.
                 *
                 * @param vertex the vertex that will be removed
                 */
                virtual void vertexWillBeRemoved(Vertex* vertex);

                /**
                 * Called to compute the plane of a face. The face of a plane is the plane on which the vertices of that
                 * face all lie. By default this method builds a plane using the given face's normal and origin.
                 * Overriding this function allows to use other methods to get the face's plane, such as via its payload.
                 *
                 * @param face the face for which the plane should be computed
                 * @return the face plane
                 */
                virtual vm::plane<T,3> getPlane(const Face* face) const;

                /**
                 * Called after a new face was created.
                 *
                 * @param face the newly created face
                 */
                virtual void faceWasCreated(Face* face);

                /**
                 * Called before a face is deleted.
                 *
                 * @param face the face to be deleted
                 */
                virtual void faceWillBeDeleted(Face* face);

                /**
                 * Called after a face was changed.
                 *
                 * @param face the face
                 */
                virtual void faceDidChange(Face* face);

                /**
                 * Called after a face was flipped (see Face::flip).
                 *
                 * @param face the face
                 */
                virtual void faceWasFlipped(Face* face);

                /**
                 * Called after a face was split into two faces. Both of the given faces will be retained. The first given
                 * face is the first portion of the face that was split, and the second given face is the other portion.
                 *
                 * @param original the original face after it was split
                 * @param clone the other portion that remained of the face after splitting
                 */
                virtual void faceWasSplit(Face* original, Face* clone);

                /**
                 * Called before two faces are merged into one face.
                 *
                 * @param remaining the face that will remain after merging
                 * @param toDelete the face that will be deleted after merging
                 */
                virtual void facesWillBeMerged(Face* remaining, Face* toDelete);
            };
            
            /**
             * A callback for the copy operation. Useful for setting up face and vertex payloads.
             */
            class CopyCallback {
            public:
                virtual ~CopyCallback();
                
                /**
                 * Called when a vertex was copied.
                 *
                 * @param original the original vertex
                 * @param copy the vertex copy
                 */
                virtual void vertexWasCopied(const Vertex* original, Vertex* copy);
                
                /**
                 * Called when a face was copied.
                 *
                 * @param original the original face
                 * @param copy the face copy
                 */
                virtual void faceWasCopied(const Face* original, Face* copy);
            };
        private:
            /**
             * The vertices of this polyhedron, stored in a circular list that owns them.
             */
            VertexList m_vertices;

            /**
             * The edges of this polyhedron, stored in a circular list that owns them.
             */
            EdgeList m_edges;

            /**
             * The faces of this polyhedron, stored in a circular list that owns them.
             */
            FaceList m_faces;

            /**
             * The bounds of this polyhedron.
             */
            vm::bbox<T,3> m_bounds;

            /* ====================== Implementation in Polyhedron_Misc.h ====================== */
        public: // Constructors
            /**
             * Constructs an empty polyhedron.
             */
            Polyhedron();

            /**
             * Constructs a polyhedron that corresponds to the convex hull of the given points.
             *
             * @param positions the points from which the convex hull is computed
             */
            Polyhedron(std::initializer_list<vm::vec<T,3>> positions);

            /**
             * Constructs a polyhedron that corresponds to the given axis aligned cuboid.
             */
            explicit Polyhedron(const vm::bbox<T,3>& bounds);

            /**
             * Constructs a polyhedron that corresponds to the convex hull of the given points
             *
             * @param positions the points from which the convex hull is computed
             */
            explicit Polyhedron(const std::vector<vm::vec<T,3>>& positions);

            /**
             * Copy constructor.
             */
            Polyhedron(const Polyhedron<T,FP,VP>& other);

            /**
             * Copy constructor with callback. The callback can be used to set up the face and vertex payloads.
             *
             * @param callback the callback to call for every created face or vertex
             */
            Polyhedron(const Polyhedron<T,FP,VP>& other, CopyCallback& callback);

            /**
             * Move constructor.
             */
            Polyhedron(Polyhedron<T,FP,VP>&& other) noexcept;
        public: // copy and move assignment
            /**
             * Copy assignment operator.
             */
            Polyhedron<T,FP,VP>& operator=(const Polyhedron<T,FP,VP>& other);

            /**
             * Move assignment operator.
             */
            Polyhedron<T,FP,VP>& operator=(Polyhedron<T,FP,VP>&& other);
        private: // Copy helper
            class Copy;
        public: // swap function, must be implemented here because it's a public template
            friend void swap(Polyhedron<T,FP,VP>& first, Polyhedron<T,FP,VP>& second) {
                using std::swap;
                swap(first.m_vertices, second.m_vertices);
                swap(first.m_edges, second.m_edges);
                swap(first.m_faces, second.m_faces);
                swap(first.m_bounds, second.m_bounds);
            }
        public: // comparison operators
            /**
             * Returns true if this polyhedron is equal to the given polyhedron.
             *
             * Two polyhedron are considered equal if the following criteria hold:
             *
             * - they have an identical vertex count and for every vertex of one polyhedron, the other has a vertex
             *   with an identical position
             * - they have an identical edge count and for every edge of one polyhedron, the other has an edge with
             *   identical positions
             * - the have an identical face count and for every face of one polyhedron, the other has a face with
             *   identical vertex positions
             */
            bool operator==(const Polyhedron& other) const;

            /**
             * Returns true if this polyhedron is not equal to the given polyhedron.
             */
            bool operator!=(const Polyhedron& other) const;
        public: // Accessors
            /**
             * Returns the number of vertices of this polyhedron.
             */
            size_t vertexCount() const;

            /**
             * Returns the vertices of this polyhedron as a reference to the containing circular list.
             */
            const VertexList& vertices() const;

            /**
             * Returns a vector containing the positions of all vertices of this polyhedron.
             */
            std::vector<vm::vec<T,3>> vertexPositions() const;

            /**
             * Returns the number of edges of this polyhedron.
             */
            size_t edgeCount() const;

            /**
             * Returns the edges of this polyhedron as a reference to the containing circular list.
             */
            const EdgeList& edges() const;

            /**
             * Checks whether this polyhedron has any edge with the given vertex positions, up to the given epsilon.
             *
             * @param pos1 the first position
             * @param pos2 the second position
             * @param epsilon the epsilon value to use for comparison
             * @return true if this polyhedron has an edge with the given vertex positions
             */
            bool hasEdge(const vm::vec<T,3>& pos1, const vm::vec<T,3>& pos2, T epsilon = static_cast<T>(0.0)) const;

            /**
             * Returns the number of faces of this polyhedron.
             */
            size_t faceCount() const;

            /**
             * Returns the faces of this polyhedron as a reference to the containing circular list.
             */
            const FaceList& faces() const;

            /**
             * Checks whether this polyhedron has any face with the given vertex positions, up to the given epsilon.
             *
             * @param positions the list of positions
             * @param epsilon the epsilon value to use for comparison
             * @return true if this polyhedron has an edge with the given vertex positions
             */
            bool hasFace(const std::vector<vm::vec<T,3>>& positions, T epsilon = static_cast<T>(0.0)) const;

            /**
             * Returns the bounds of this polyhedron.
             */
            const vm::bbox<T,3>& bounds() const;

            /**
             * Indicates whether this polyhedron is empty.
             *
             * A polyhedron is empty if it has no vertices.
             *
             * @return true if this polyhedron is empty and false otherwise
             */
            bool empty() const;

            /**
             * Indicates whether this polyhedron is a point.
             *
             * A polyhedron is a point if it has exactly one vertex.
             *
             * @return true if this polyhedron is a point and false otherwise
             */
            bool point() const;

            /**
             * Indicates whether this polyhedron is an edge.
             *
             * A polyhedron is an edge if it has exactly two vertices.
             *
             * @return true if this polyhedron is an edge and false otherwise
             */
            bool edge() const;

            /**
             * Indicates whether this polyhedron is a polygon.
             *
             * A polyhedron is a polygon if it has exactly one face.
             *
             * @return true if this polyhedron is a polygon and false otherwise
             */
            bool polygon() const;

            /**
             * Indicates whether this polyhedron is a convex volume.
             *
             * A polyhedron is a convex volume if it has more than three faces.
             *
             * @return true if this polyhedron is a convex volume and false otherwise
             */
            bool polyhedron() const;

            /**
             * Indicates whether this polyhedron is closed.
             *
             * A polyhedron is closed if the sum of its vertex and face count is equal to its edge count plus two
             * (Euler criterion for convex polyhedra).
             *
             * @return true if this polyhedron is closed and false otherwise
             */
            bool closed() const;

            /**
             * Clears this polyhedron. Afterwards, this polyhedron will have 0 vertices, 0 edges, 0 faces and its bounds
             * will have NaN vectors for its min and max values.
             */
            void clear();

            /**
             * The result of picking this polyhedron with a ray. If the polyhedron was hit, the face member contains a
             * pointer to the face that was hit by the ray, and the distance member contains the distance from the ray
             * origin to the hit point. If the polyhedron was not hit, the face member is null and the distance member
             * is NaN.
             */
            struct FaceHit {
                Face* face;
                T distance;

                FaceHit(Face* i_face, const T i_distance);
                FaceHit();
                bool isMatch() const;
            };

            /**
             * Picks this polyhedron with the given ray and returns a face hit.
             *
             * @param ray the pick ray
             * @return the face hit
             */
            FaceHit pickFace(const vm::ray<T,3>& ray) const;
        public: // General purpose methods
            /**
             * Checks whether this polyhedron has a vertex with the given position, up to the given epsilon.
             *
             * @param position the position to check
             * @param epsilon the epsilon value to use for comparison
             * @return true if this polyhedron has a vertex with the given position and false otherwise
             */
            bool hasVertex(const vm::vec<T,3>& position, T epsilon = static_cast<T>(0.0)) const;

            /**
             * Checks whether any of the given positions corresponds to the position of a vertex of this polyhedron.
             *
             * @param positions the positions to check
             * @param epsilon the epsilon value to use for comparison
             * @return true if any of the given positions has a corresponding vertex of this polyhedron and false
             * otherwise
             */
            bool hasAnyVertex(const std::vector<vm::vec<T,3>>& positions, T epsilon = static_cast<T>(0.0)) const;

            /**
             * Checks whether all of the given positions corresponds to the position of a vertex of this polyhedron.
             *
             * @param positions the positions to check
             * @param epsilon the epsilon value to use for comparison
             * @return true if all of the given positions have a corresponding vertex of this polyhedron and false
             * otherwise
             */
            bool hasAllVertices(const std::vector<vm::vec<T,3>>& positions, T epsilon = static_cast<T>(0.0)) const;

            /**
             * Finds a vertex with the given position.
             *
             * @param position the position to find
             * @param epsilon the epsilon value to use for comparison
             * @return a vertex whose position compares equal to the given position or null if no such vertex exists in
             * this polyhedron
             */
            Vertex* findVertexByPosition(const vm::vec<T,3>& position, T epsilon = static_cast<T>(0.0)) const;

            /**
             * Finds a vertex such that
             * - its distance to the given position is less than the given maximum distance and
             * - its distance to the given position is minimal among all vertices of this polyhedron.
             *
             * @param position the position
             * @param maxDistance the maximum distance at which a vertex is considered
             * @return a vertex or null if no vertex satisfies the criteria listed above
             */
            Vertex* findClosestVertex(const vm::vec<T,3>& position, T maxDistance = std::numeric_limits<T>::max()) const;

            /**
             * Finds an edge with the given vertex positions. An edge is considered a match if
             * Edge::hasVertexPositions(pos1, pos2, epsilon) returns true.
             *
             * @param pos1 the first position
             * @param pos2 the second position
             * @param epsilon the epsilon value to use for comparison
             * @return an edge with the given vertex positions or null if no such edge exists in this polyhedron
             */
            Edge* findEdgeByPositions(const vm::vec<T,3>& pos1, const vm::vec<T,3>& pos2, T epsilon = static_cast<T>(0.0)) const;

            /**
             * Finds an edge such that
             * - its distance to the given positions is less than the given maximum distance and
             * - its distance to the given positions is minimal among all edges of this polyhedron.
             *
             * Thereby, the distance from an edge to the given positions is determined by Edge::distanceTo(pos1, pos2).
             *
             * @param pos1 the first position
             * @param pos2 the second position
             * @param maxDistance the maximum distance at which an edge is considered
             * @return an edge or null if no edge satisfies the criteria listed above
             */
            Edge* findClosestEdge(const vm::vec<T,3>& pos1, const vm::vec<T,3>& pos2, T maxDistance = std::numeric_limits<T>::max()) const;

            /**
             * Finds a face with the given vertex positions. A face is considered a match if
             * Face::hasVertexPositions(positions, epsilon) returns true.
             *
             * @param positions the positions
             * @param epsilon the epsilon value to use for comparison
             * @return a face with the given vertex positions or null if no such face exists in this polyhedron
             */
            Face* findFaceByPositions(const std::vector<vm::vec<T,3>>& positions, T epsilon = static_cast<T>(0.0)) const;

            /**
             * Finds a face such that
             * - its distance to the given positions is less than the given maximum distance and
             * - its distance to the given positions is minimal among all faces of this polyhedron.
             *
             * Thereby, the distance from a face to the given positions is determined by Face::distanceTo(positions).
             *
             * @param positions the positions
             * @param maxDistance the maximum distance at which a face is considered
             * @return a face or null if no face satisfies the criteria listed above
             */
            Face* findClosestFace(const std::vector<vm::vec<T,3>>& positions, T maxDistance = std::numeric_limits<T>::max());
        private:
            /**
             * Updates the bounds to the smallest bounding box that contains the positions of all vertices of this
             * polyhedron. If this polyhedron does not have any vertices, the min and max of the bounds are set to NaN
             * vectors.
             */
            void updateBounds();
        public: // Vertex correction and edge healing
            /**
             * Rounds each component of position of every vertex to the nearest integer if the distance of the
             * component's value to that integer is less  than the given epsilon value. Furthermore, the component value
             * is rounded such that at most the given number of decimals are retained.
             *
             * Updates the bounds of this polyhedron afterwards.
             *
             * @param decimals the number of decimals to retain
             * @param epsilon an epsilon value
             */
            void correctVertexPositions(const size_t decimals = 0, const T epsilon = vm::constants<T>::correct_epsilon());

            /**
             * Heals short edges by removing all edges shorter than the given minimum length. If removing an edge leads
             * to degenerate faces, these degenerate faces are removed, too.
             *
             * Updates the bounds of this polyhedron afterwards.
             *
             * @param minLength the minimum edge length, edges shorter than this length will be removed
             * @return true if this polyhedron is a convex volume afterwards
             */
            bool healEdges(const T minLength = MinEdgeLength);

            /**
             * Heals short edges by removing all edges shorter than the given minimum length. If removing an edge leads
             * to degenerate faces, these degenerate faces are removed, too.
             *
             * Updates the bounds of this polyhedron afterwards.
             *
             * @param callback the callback to inform of lifecycle events
             * @param minLength the minimum edge length, edges shorter than this length will be removed
             * @return true if this polyhedron is a convex volume afterwards
             */
            bool healEdges(Callback& callback, const T minLength = MinEdgeLength);
        private:
            /**
             * Removes the given edge from this polyhedron. The incident faces are updated accordingly, and they are
             * removed if they become degenerate.
             *
             * @param edge the edge to remove
             * @param callback the callback to inform of lifecycle events
             * @return removes the successor of the given edge in the containing circular list
             */
            Edge* removeEdge(Edge* edge, Callback& callback);

            /**
             * Removes the given degenerate face. A face is considered degenerate if it has only two vertices.
             *
             * If the given face is not degenerate, the behavior is undefined.
             *
             * @param face the face to remove, must not be null
             * @param callback the callback to inform of lifecycle events
             */
            void removeDegenerateFace(Face* face, Callback& callback);

            /**
             * Merges two adjacent faces. The faces to be merged are those that share the edge to which the given half
             * edge belongs.
             *
             * Let f1 be the face to which the given half edge borderFirst belongs. Let f2 be the face to which the twin
             * of the given half edge belongs, and let e be the edge to which the given half edge belongs. Then f1 and
             * f2 are adjacent and should be merged.
             *
             * In some cases, f1 and f2 can share more edges than e. In such a case, the shared edges preceede and / or
             * succeed e, so this algorithm will search for all shared edges and handle them accordingly. The effects
             * of merging f1 and f2 are
             *
             * - that f2 is deleted
             * - the remaining part of f2's boundary replaces the shared half edges in f1's boundary
             * - the shared edges and half edges are deleted
             * - all vertices which have only f1 and f2 as their incident faces are deleted
             * - f2 is deleted
             *
             * Finally, if the given validEdge is deleted by this algorithm, than its first successor that is not deleted
             * by this algorithm is returned. This return value can be used by the caller during iteration over all edges
             * of this polyhedron.
             *
             * @param borderFirst a half edge that belongs to the edge to which the faces to be merged are incident
             * @param validEdge an edge that is used for iteration of all edges (see result)
             * @param callback the callback to inform of lifecycle events
             * @return the given valid edge, or its first successor that was not deleted by this function
             */
            Edge* mergeNeighbours(HalfEdge* borderFirst, Edge* validEdge, Callback& callback);

            /* ====================== Implementation in Polyhedron_ConvexHull.h ====================== */
        public: // Convex hull; adding and removing points
            /**
             * Adds the given points to this polyhedron. The effect of adding the given points to a polyhedron is that
             * the resulting polyhedron is the convex hull of the union of the polyhedron's vertices and the given points.
             *
             * @param points the points to add to this polyhedron
             */
            void addPoints(const std::vector<vm::vec<T,3>>& points);

            /**
             * Adds the given points to this polyhedron. The effect of adding the given points to a polyhedron is that
             * the resulting polyhedron is the convex hull of the union of the polyhedron's vertices and the given points.
             *
             * @param points the points to add to this polyhedron
             * @param callback the callback to inform of lifecycle events
             */
            void addPoints(const std::vector<vm::vec<T,3>>& points, Callback& callback);
        private:
            /**
             * Adds the points in range [cur, end) to this polyhedron. The effect of adding the given points to a
             * polyhedron is that the resulting polyhedron is the convex hull of the union of the polyhedron's vertices
             * and the given points.
             *
             * @tparam I the type of the given iterators
             * @param cur start of the range of points to add
             * @param end end of the range of points to add
             */
            template <typename I> void addPoints(I cur, I end);

            /**
             * Adds the points in range [cur, end) to this polyhedron. The effect of adding the given points to a
             * polyhedron is that the resulting polyhedron is the convex hull of the union of the polyhedron's vertices
             * and the given points.
             *
             * @tparam I the type of the given iterators
             * @param cur start of the range of points to add
             * @param end end of the range of points to add
             * @param callback the callback to inform of lifecycle events
             */
            template <typename I> void addPoints(I cur, I end, Callback& callback);
        public:
            /**
             * Adds the given point to this polyhedron. The effect of adding the given point to a polyhedron is that the
             * resulting polyhedron is the convex hull of the union of the polyhedron's vertices and the given point.
             *
             * If the given point is within this polyhedron, the it will not be added.
             *
             * @param position the point to add
             * @return the newly created vertex, or null if the given point was not added to this polyhedron
             */
            Vertex* addPoint(const vm::vec<T,3>& position);

            /**
             * Adds the given point to this polyhedron. The effect of adding the given point to a polyhedron is that the
             * resulting polyhedron is the convex hull of the union of the polyhedron's vertices and the given point.
             *
             * If the given point is within this polyhedron, the it will not be added.
             *
             * @param position the point to add
             * @param callback the callback to inform of lifecycle events
             * @return the newly created vertex, or null if the given point was not added to this polyhedron
             */
            Vertex* addPoint(const vm::vec<T,3>& position, Callback& callback);

            /**
             * Merges this polyhedron with the given polyhedron. The effect of merging two polyhedra is that the resulting
             * polyhedron is the convex hull of the union of the vertices of both polyhedra.
             *
             * @param other the polyhedron to merge this polyhedron with
             */
            void merge(const Polyhedron& other);

            /**
             * Merges this polyhedron with the given polyhedron. The effect of merging two polyhedra is that the resulting
             * polyhedron is the convex hull of the union of the vertices of both polyhedra.
             *
             * @param other the polyhedron to merge this polyhedron with
             * @param callback the callback to inform of lifecycle events
             */
            void merge(const Polyhedron& other, Callback& callback);
        private:
            /**
             * Helper function that adds the given point to an empty polyhedron. Afterwards, this polyhedron will be a
             * point.
             *
             * @param position the point to add
             * @param callback the callback to inform of lifecycle events
             * @return the newly created vertex
             */
            Vertex* addFirstPoint(const vm::vec<T,3>& position, Callback& callback);

            /**
             * Helper function that adds the given point to a point polyhedron. Afterwards, this polyhedron will be a
             * point or an edge.
             *
             * If the given point is identical to this polyhedron's single vertices' position, then nothing happens.
             *
             * Assumes that this is a point polyhedron.
             *
             * @param position the point to add
             * @param callback the callback to inform of lifecycle events
             * @return the newly created vertex or null if no vertex was created
             */
            Vertex* addSecondPoint(const vm::vec<T,3>& position, Callback& callback);

            /**
             * Helper function that adds the given point to an edge polyhedron. Afterwards, this polyhedron will be an
             * edge or a triangle.
             *
             * If the given point is contained in the edge formed by this polyhedron, nothing happens. Otherwise, if the
             * given point and this polyhedron's vertices are linearly dependent, then this polyhedron is modified, but
             * remains an edge.
             *
             * Assumes that this is an edge polyhedron.
             *
             * @param position the point to add
             * @param callback the callback to inform of lifecycle events
             * @return the newly created vertex or null if no vertex was created
             */
            Vertex* addThirdPoint(const vm::vec<T,3>& position, Callback& callback);

            /**
             * Helper function that adds the given point to an edge polyhedron. Afterwards, this polyhedron is an edge.
             *
             * Assumes that this is an edge polyhedron and that the given point and this polyhedron's vertices are
             * linearly dependent.
             *
             * @param position the point to add
             * @param callback the callback to inform of lifecycle events
             * @return the newly created vertex or null if no vertex was created
             */
            Vertex* addColinearThirdPoint(const vm::vec<T,3>& position, Callback& callback);

            /**
             * Helper function that adds the given point to an edge polyhedron. Afterwards, this polyhedron is a triangle.
             *
             * Assumes that this is an edge polyhedron and that the given point and this polyhedron's vertices are
             * linearly independent.
             *
             * @param position the point to add
             * @param callback the callback to inform of lifecycle events
             * @return the newly created vertex
             */
            Vertex* addNonColinearThirdPoint(const vm::vec<T,3>& position, Callback& callback);

            /**
             * Helper function that adds the given point to a polyhedron that is either a polygon or a convex volume.
             *
             * Adding a point to such a polyhedron has one of the following effects:
             * - if the given point is contained in this polyhedron, nothing happens
             * - if this is a polygon, and the given point lies on the same plane as the polygon, then it is added, but
             *   the polyhedron remains a polygon
             * - if this is a polygon, and the given point does not lie on the same plane as the polygon, then it is
             *   added and the polyhedron becomes a convex volume
             * - if this is a convex volume, then it is updated to the convex hull of the union of its vertices and the
             *   given point
             *
             * Assumes that this is either a polygon or a convex volume.
             *
             * @param position the point to add
             * @param callback the callback to inform of lifecycle events
             * @return the newly created vertex or null if no vertex was created
             */
            Vertex* addFurtherPoint(const vm::vec<T,3>& position, Callback& callback);

            /**
             * Helper function that adds the given point to a polygon.
             *
             * Adding a point to a polygon either turns it into a convex volume or it remains a polygon, depending on
             * whether the given point lies on the same plane as the polygon itself.
             *
             * Assumes that this polyhedron is a polygon.
             *
             * @param position the point to add
             * @param callback the callback to inform of lifecycle events
             * @return the newly created vertex or null if no vertex was created
             */
            Vertex* addFurtherPointToPolygon(const vm::vec<T,3>& position, Callback& callback);

            /**
             * Helper function that adds a coplanar point to a polygon.
             *
             * Assumes that this polyhedron is a polygon and that the given position lies in the same plane as the
             * polygon.
             *
             * @param position the point to add
             * @param callback the callback to inform of lifecycle events
             * @return the newly created vertex or null if no vertex was created
             */
            Vertex* addPointToPolygon(const vm::vec<T,3>& position, Callback& callback);

            /**
             * Helper function that creates a new polygon from the given vector of coplanar points.
             *
             * Assumes that this polyhedron is empty and that the given vector contains at least three linearly
             * independent points.
             *
             * @param positions the points to create a polygon from
             * @param callback the callback to inform of lifecycle events
             */
            void makePolygon(const std::vector<vm::vec<T,3>>& positions, Callback& callback);

            /**
             * Helper function that adds the given non coplanar point to a polygon, turning it into a convex volume.
             *
             * Assumes that this polyhedron is a polygon and that the given point does not lie on the same plane as the
             * polygon.
             *
             * @param position the point to add
             * @param callback the callback to inform of lifecycle events
             * @return the newly created vertex or null if no vertex was created
             */
            Vertex* makePolyhedron(const vm::vec<T,3>& position, Callback& callback);

            /**
             * Helper function that adds the given point to a convex volume.
             *
             * Assumes that this polyhedron is a convex volume.
             *
             * @param position the point to add
             * @param callback the callback to inform of lifecycle events
             * @return the newly created vertex or null if no vertex was created
             */
            Vertex* addFurtherPointToPolyhedron(const vm::vec<T,3>& position, Callback& callback);

            /**
             * A seam is a circular sequence of consecutive edges. For each edge of a seam, it must hold that its first
             * vertex is identical to the second vertex of its predecessor.
             */
            class Seam;

            class SplittingCriterion;
            class SplitByVisibilityCriterion;
            class SplitByConnectivityCriterion;

            /**
             * Creates a seam using the given splitting criterion.
             *
             * Suppose that the faces of a polyhedron are the vertices of a graph G, and two graph vertices are connected
             * in if and only if the corresponding faces are adjacent.
             *
             * Given such a graph, a seam divides the faces of a graph into two disjoint sets such that, for every pair
             * of faces f1, f2 from the same set, there is a graph in G that connects them.
             *
             * In other words, a seam separates the faces of a polyhedron in such a way that the polyhedron can be
             * pulled apart into two "open" polyhedra along the seam.
             *
             * Thereby, the returned seam contains those edges where the incident faces belong to different sets, i.e.
             * the edges of the seam are oriented such that the first faces matches the splitting criterion and the other
             * faces does not.
             *
             * @param criterion the splitting criterion
             * @return a seam that separates the faces that match the given criterion from those that do not
             */
            Seam createSeam(const SplittingCriterion& criterion);

            /**
             * Splits this polyhedron along the given seam. The edges of the seam must be oriented in such a way that
             * the portion of this polyhedron that is connected to the first faces of the given edges should be retained
             * and the portion of this polyhedron that is connected to the second faces of the given edges should be
             * removed.
             *
             * The removed faces, edges and vertices are deleted. The seam edges will have their second half edges unset.
             *
             * Therefore, this polyhedron will be open after this function finishes.
             *
             * @param seam the seam to split this polyhedron along
             * @param callback the callback to inform of lifecycle events
             */
            void split(const Seam& seam, Callback& callback);

            /**
             * Helper that recursively deletes faces and edges, and adds vertices to be deleted to the given list.
             *
             * This function will delete all faces and edges which are somehow connected to the given half edge.
             * Furthermore, it will delete all vertices it encounters where the leaving half edge is to be deleted also.
             *
             * To protect a vertex from being deleted, it should therefore have a leaving half edge that is not going to
             * be deleted here, i.e. to protect a vertex from being deleted, its leaving edge should belong to a face
             * which will not be visited by this function.
             *
             * Therefore, a caller should make sure that the portion of this polyhedron that should be retained is not
             * connected to the portion that should be deleted. This can be ensured by unsetting the second half edge of
             * every seam edge before calling this function.
             *
             * @tparam FaceSet a set of faces
             * @param first the half edge at which to start deleting faces
             * @param visitedFaces the faces that have already been visited by this function in previous calls
             * @param verticesToDelete the vertices that should be deleted later
             * @param callback the callback to inform of lifecycle events
             */
            template <typename FaceSet>
            void deleteFaces(HalfEdge* first, FaceSet& visitedFaces, VertexList& verticesToDelete, Callback& callback);

            /**
             * Waves a new cap onto this polyhedron. The new cap will be a single polygon, so this function assumes that
             * this polygon is opened along the given seam, and all edges of the given seam lie on a plane.
             *
             * The edges of the given seam are expected to be oriented such that their second edges are unset. The new
             * polygon will then be the first face of every edge in the given seam.
             *
             * Assumes that this polyhedron is neither empty, nor a point, nor an edge, nor a polygon.
             *
             * @param seam the seam to weave a polygon onto
             * @param callback the callback to inform of lifecycle events
             */
            void sealWithSinglePolygon(const Seam& seam, Callback& callback);

            class ShiftSeamForWeaving;

            /**
             * Weaves a new cap onto this polyhedron. The new cap will be a cone, the tip of which will be a newly created
             * vertex at the given position. If two adjacent faces of the cone are coplanar, these will be merged.
             *
             * The edges of the given seam are expected to be oriented such that their second edges are unset. Each edge
             * will then have a newly created polygon as its first face.
             *
             * Assumes that this polyhedron is neither empty, nor a point, nor an edge. Note that this polyhedron can be
             * a polygon, however.
             *
             * @param seam the seam to weave a cone onto
             * @param position the position of the cone's tip
             * @param callback the callback to inform of lifecycle events
             * @return the newly created vertex at the tip of the newly created cone
             */
            Vertex* weave(Seam seam, const vm::vec<T,3>& position, Callback& callback);

            /* ====================== Implementation in Polyhedron_Clip.h ====================== */
        public: // Clipping
            /**
             * The result of clipping this polyhedron with a plane.
             */
            struct ClipResult {
                typedef enum {
                    /**
                     * Clipping did not change this polyhedron.
                     */
                        Type_ClipUnchanged,

                    /**
                     * Clipping resulted in an empty polyhedron.
                     */
                        Type_ClipEmpty,

                    /**
                     * Clipping was successful and effective.
                     */
                        Type_ClipSuccess
                } Type;

                /**
                 * The type of result.
                 */
                const Type type;

                /**
                 * Creates a new clip result of the given type.
                 * @param i_type the type of the clip result
                 */
                ClipResult(const Type i_type);

                /**
                 * Indicates whether clipping this polyhedron had any effect.
                 */
                bool unchanged() const;

                /**
                 * Indicates whether clipping this polyhedron resulted in an empty polyhedron.
                 */
                bool empty() const;

                /**
                 * Indicates whether clipping this polyhedron was successful and effective.
                 */
                bool success() const;
            };

            /**
             * Removes the part of this polyhedron that is in front of the given plane.
             *
             * @param plane the plane to clip with
             * @return the result of the clipping operation
             * @throw GeometryException if the polyhedron cannot be intersected with the given plane
             */
            ClipResult clip(const vm::plane<T,3>& plane);

            /**
             * Removes the part of this polyhedron that is in front of the given plane.
             *
             * @param plane the plane to clip with
             * @param callback the callback to inform of lifecycle events
             * @return the result of the clipping operation
             * @throw GeometryException if the polyhedron cannot be intersected with the given plane
             */
            ClipResult clip(const vm::plane<T,3>& plane, Callback& callback);

        private:
            /**
             * Checks whether this polyhedron is intersected by the given plane. Returns either of the following results.
             *
             * - a clip result of type Type_ClipUnchanged if intersecting this polyhedron with the given plane would leave
             *   it unchanged
             * - a clip result of Type_ClipEmpty if intersecting this polyhedron with the given plane would leave it
             *   empty
             * - a clip result of Type_ClipSuccess if intersecting this polyhedron with the given plane would be successful
             *    and effective.
             *
             * @param plane the plane to check
             * @return a clip result
             */
            ClipResult checkIntersects(const vm::plane<T,3>& plane) const;

            class NoSeamException;

            /**
             May throw a NoSeamException if the polyhedron cannot be intersected with the given plane due.
             */

            /**
             * Creates a seam along the intersection of this polyhedron with the given plane. This will create
             *
             * - new vertices where the plane intersects with an edge of this polyhedron,
             * - new edges connecting the newly created vertices such that a seam can be created and
             * - new faces by splitting those faces that are intersected with the given plane.
             *
             * The newly created edges are oriented such that their second edge is incident to the faces which are
             * above the given plane. If a face is split, then the portion that is above the given plane is newly created
             * and the portion that is below the given plane will be the original face, modified so that it does not
             * intersect with the given plane.
             *
             * Afterwards, this polyhedron will be modified in such a way that it has no edge or face that intersects with
             * the given plane. The edges of the returned seam will lie entirely inside of the given plane.
             *
             * This function assumes that the given plane does intersect with this polyhedron.
             *
             * In some corner cases, it is impossible to construct a seam due to floating point inaccuracies. If this
             * function detects such a case, it throws a NoSeamException.
             *
             * @param plane the plane to intersect this polyhedron with
             * @param callback the callback to inform of lifecycle events
             * @return the constructed seam, which will not be empty and valid
             * @throw NoSeamException if no seam could be constructed
             */
            Seam intersectWithPlane(const vm::plane<T,3>& plane, Callback& callback);

            /**
             * This function finds the starting edge for intersecting a polyhedron with a plane. It returns a half edge
             * such that one of the following conditions hold:
             *
             * - The half edge is split in two by the given plane, and its origin is be above the given plane or
             *   its destination is be below the given plane.
             * - The half edge lies entirely within the given plane, and it belongs to a face that lies above the given
             *   plane.
             *
             * This function may fail to find such a half edge in certain corner cases. If such a case is detected, the
             * function returns null.
             *
             * @param plane the intersecting plane
             * @return the starting edge for intersecting this polyhedron with the plane, or null if no such edge could
             * be found
             */
            HalfEdge* findInitialIntersectingEdge(const vm::plane<T,3>& plane) const;

            /**
             * Intersects a face with the given plane. There are three cases to consider.
             *
             * 1. The plane just touches the face, i.e. one vertex of the face is inside the plane. In this case, the
             *    face is left untouched and the half edge whose destination is inside the plane is returned.
             * 2. The plane just touches an edge of the face, i.e. two consecutive vertices of the face are inside the
             *    plane. In that case, the face is left untouched and the touched half edge is returned.
             * 3. The plane intersects the face. In this case, there are two points where the given plane intersects the
             *    boundary of the face, and any number of these points of intersection may coincide with a vertex of the
             *    face.
             *
             *    In this case, the algorithm will insert vertices at those points of intersection where there isn't a
             *    vertex already, splitting the intersected edges in the process. Finally, the algorithm will split the
             *    face into two such that the newly created face coincides with the portion of the given face that lies
             *    above the given plane, and the given face is modified so that it lies entirely below the given plane.
             *
             *    Finally, the algorithm then returns the newly created half edge which was inserted into the given face.
             *
             * The returned half edge can be used by the caller to continue splitting this polyhedron with the given plane.
             *
             * @param firstBoundaryEdge a half edge that belongs to the face being split
             * @param plane the intersecting plane
             * @param callback the callback to inform of lifecycle events
             * @return a half edge as specified in the description above
             */
            HalfEdge* intersectWithPlane(HalfEdge* firstBoundaryEdge, const vm::plane<T,3>& plane, Callback& callback);

            /**
             * Splits a face in two, creating a new face and a new edge. Expects that both given half edges
             * oldBoundaryFirst and newBoundaryFirst belong to the same face, which is the face to be split.
             *
             * The following diagram illustrates how the face is split. Given a face with the following shape:
             *
             *      oldBoundaryFirst
             *   *<------------------*<------------------*
             *   |                                       /\
             *   |                                       |
             *   |                 face                  |
             *   |                                       |
             *   \/                                      |
             *   *------------------>*------------------>*
             *                         newBoundaryFirst
             *
             * The result of splitting this face will then look like this:
             *
             *      oldBoundaryFirst
             *   *<------------------*<------------------*
             *   |                   |/\                 /\
             *   |                   ||                  |
             *   |       face        ||     new face     |
             *   |                   ||                  |
             *   \/                 \/|                  |
             *   *------------------>*------------------>*
             *                         newBoundaryFirst
             *
             * @param oldBoundaryFirst the first half edge of the boundary of the remaining portion of the face
             * @param newBoundaryFirst the first half edge of the boundary of the newly created face
             * @param callback the callback to inform of lifecycle events
             */
            void intersectWithPlane(HalfEdge* oldBoundaryFirst, HalfEdge* newBoundaryFirst, Callback& callback);

            /**
             * Searches all half edges leaving the destination of half edge searchFrom for a half edge that is intersected
             * by the given plane under the assumption that searchFrom was also intersected.
             *
             * A half edge leaving searchEdge's destination is considered to be intersected by the given plane if either
             * of the following conditions hold:
             *
             * - its destination is inside the plane
             * - its destination and the origin of its predecessor lie on different sides of the plane.
             *
             * The search stops if such a half edge is found or if the search encounters searchFrom's twin, which is
             * already known to be intersected by the given plane. In the first case, the found half edge is returned, and
             * in the latter case, the function returns null.
             *
             * @param searchFrom the half edge at which the search starts and ends
             * @param plane the intersecting plane
             * @return a half edge that is intersected by the given plane and that is different from the given half edge's
             * twin, or null if no such half edge could be found
             */
            HalfEdge* findNextIntersectingEdge(HalfEdge* searchFrom, const vm::plane<T,3>& plane) const;

            /* ====================== Implementation in Polyhedron_CSG.h ====================== */
        public: // Intersection
            /**
             * Clips this polyhedron with all faces of the given polyhedron. The result is the intersection of this and
             * the given polyhedron. This polyhedron remains unchanged.
             *
             * @param other the polyhedron to clip with
             * @param callback the callback to use when determining the normals of the given subtrahend's faces
             * @return the result of the clipping operation
             * @throw GeometryException if the polyhedron cannot be intersected with any face of the given polyhedron
             */
            Polyhedron intersect(Polyhedron other, const Callback& callback = Callback()) const;

        public: // Subtraction
            /**
             * Subtracts the given polyhedron from this polyhedron and returns the resulting fragments. This polyhedron
             * remains unchanged.
             *
             * In general, the result of a CSG subtraction is concave and can therefore not be represented by one single
             * polyhedron. This algorithm represents the concave shape as a disjoint union of convex polyhedra. In the
             * context of subtraction, these polyhedra are called fragments.
             *
             * If the given polyhedron and this polyhedron are disjoint, the result of the subtraction is identical to
             * this polyhedron.
             *
             * @param subtrahend the polyhedron to subtract from this polyhedron
             * @param callback the callback to use when determining the normals of the given subtrahend's faces
             * @return the resulting fragments
             */
            std::vector<Polyhedron> subtract(const Polyhedron& subtrahend, const Callback& callback = Callback()) const;
        private:
            class Subtract;

            /* ====================== Implementation in Polyhedron_Queries.h ====================== */
        public: // geometrical queries
            /**
             * Checks whether this polyhedron contains the given point. A point is considered to be contained in this
             * polyhedron if it isn't above any of its faces' planes.
             *
             * @param point the point to check
             * @param callback the callback to use when determining plane normals
             * @return true if the given point is contained in this polyhedron and false otherwise
             */
            bool contains(const vm::vec<T,3>& point, const Callback& callback = Callback()) const;

            /**
             * Checks whether this polyhedron contains the given polyhedron. A polyhedron is considered to be contained
             * in this polyhedron if the CSG intersection of the polyhedra is identical to the given polyhedron.
             *
             * @param other the polyhedron to check
             * @return true if the given polyhedron is contained in this polyhedron and false otherwise
             */
            bool contains(const Polyhedron& other) const;

            /**
             * Checks whether this polyhedron intersects with the given polyhedron, i.e. whether their CSG intersection
             * is an empty polyhedron.
             *
             * @param other the polyhedron to check
             * @param callback the callback to use when determining the normals of the given subtrahend's faces
             * @return true if this polyhedron intersects the other polyhedron
             */
            bool intersects(const Polyhedron& other, const Callback& callback = Callback()) const;
        private: // helper functions for all cases of polygon / polygon intersection
            static bool pointIntersectsPoint(const Polyhedron& lhs, const Polyhedron& rhs);
            static bool pointIntersectsEdge(const Polyhedron& lhs, const Polyhedron& rhs);
            static bool pointIntersectsPolygon(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback = Callback());
            static bool pointIntersectsPolyhedron(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback = Callback());

            static bool edgeIntersectsPoint(const Polyhedron& lhs, const Polyhedron& rhs);
            static bool edgeIntersectsEdge(const Polyhedron& lhs, const Polyhedron& rhs);
            static bool edgeIntersectsPolygon(const Polyhedron& lhs, const Polyhedron& rhs);
            static bool edgeIntersectsPolyhedron(const Polyhedron& lhs, const Polyhedron& rhs);

            static bool edgeIntersectsFace(const Edge* lhsEdge, const Face* rhsFace);

            static bool polygonIntersectsPoint(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback = Callback());
            static bool polygonIntersectsEdge(const Polyhedron& lhs, const Polyhedron& rhs);
            static bool polygonIntersectsPolygon(const Polyhedron& lhs, const Polyhedron& rhs);
            static bool polygonIntersectsPolyhedron(const Polyhedron& lhs, const Polyhedron& rhs);

            static bool faceIntersectsFace(const Face* lhsFace, const Face* rhsFace);

            static bool polyhedronIntersectsPoint(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback = Callback());
            static bool polyhedronIntersectsEdge(const Polyhedron& lhs, const Polyhedron& rhs);
            static bool polyhedronIntersectsPolygon(const Polyhedron& lhs, const Polyhedron& rhs);
            static bool polyhedronIntersectsPolyhedron(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback = Callback());

            /**
             * Checks whether there is a face among the given faces such that all of the given vertices are above that
             * face's plane.
             *
             * @param faces the faces to check
             * @param vertices the vertices to check against each face plane
             * @param callback the callback to use when determining the normals of the given subtrahend's faces
             * @return true if a face was found such that all of the given vertices have their position above the face
             * plane and false otherwise
             */
            static bool separate(const FaceList& faces, const VertexList& vertices, const Callback& callback);

            /**
             * Checks the relative positions of the given points to the given plane. Returns
             *
             * - vm::plane_status::above if all of the given points are above the given plane
             * - vm::plane_status::below if all of the given points are below the given plane
             * - vm::plane_status::inside otherwise
             *
             * @param plane the plane
             * @param vertices the vertices to check
             * @return the relative position of the given points to the given plane
             */
            static vm::plane_status pointStatus(const vm::plane<T,3>& plane, const VertexList& vertices);

            /* ====================== Implementation in Polyhedron_Checks.h ====================== */
        private: // invariants and checks
            bool checkInvariant() const;
            bool checkEulerCharacteristic() const;
            bool checkOverlappingFaces() const;
            bool checkFaceBoundaries() const;
            bool checkFaceNeighbours() const;
            bool checkConvex() const;
            bool checkClosed() const;
            bool checkNoCoplanarFaces() const;
            bool checkNoDegenerateFaces() const;
            bool checkVertexLeavingEdges() const;
            bool checkEdges() const;
            bool checkEdgeLengths(const T minLength = MinEdgeLength) const;
            bool checkLeavingEdges(const Vertex* v) const;
        };
    }
}

#endif
