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

#pragma once

#include "Polyhedron.h"

namespace TrenchBroom {
    namespace Model {
        template <typename T, typename FP, typename VP>
        kdl::intrusive_circular_link<Polyhedron_HalfEdge<T,FP,VP>>& Polyhedron_GetHalfEdgeLink<T,FP,VP>::operator()(Polyhedron_HalfEdge<T,FP,VP>* halfEdge) const {
            return halfEdge->m_link;
        }

        template <typename T, typename FP, typename VP>
        const kdl::intrusive_circular_link<Polyhedron_HalfEdge<T,FP,VP>>& Polyhedron_GetHalfEdgeLink<T,FP,VP>::operator()(const Polyhedron_HalfEdge<T,FP,VP>* halfEdge) const {
            return halfEdge->m_link;
        }

        template <typename T, typename FP, typename VP>
        Polyhedron_HalfEdge<T,FP,VP>::Polyhedron_HalfEdge(Vertex* origin) :
            m_origin(origin),
            m_edge(nullptr),
            m_face(nullptr),
#ifdef _MSC_VER
        // MSVC throws a warning because we're passing this to the FaceLink constructor, but it's okay because we just store the pointer there.
#pragma warning(push)
#pragma warning(disable : 4355)
m_link(this)
#pragma warning(pop)
#else
            m_link(this)
#endif
        {
            assert(m_origin != nullptr);
            setAsLeaving();
        }

        template <typename T, typename FP, typename VP>
        Polyhedron_HalfEdge<T,FP,VP>::~Polyhedron_HalfEdge() {
            if (m_origin->leaving() == this)
                m_origin->setLeaving(nullptr);
        }

        template <typename T, typename FP, typename VP>
        typename Polyhedron_HalfEdge<T,FP,VP>::Vertex* Polyhedron_HalfEdge<T,FP,VP>::origin() const {
            return m_origin;
        }

        template <typename T, typename FP, typename VP>
        typename Polyhedron_HalfEdge<T,FP,VP>::Vertex* Polyhedron_HalfEdge<T,FP,VP>::destination() const {
            return next()->origin();
        }

        template <typename T, typename FP, typename VP>
        typename Polyhedron_HalfEdge<T,FP,VP>::Edge* Polyhedron_HalfEdge<T,FP,VP>::edge() const {
            return m_edge;
        }

        template <typename T, typename FP, typename VP>
        typename Polyhedron_HalfEdge<T,FP,VP>::Face* Polyhedron_HalfEdge<T,FP,VP>::face() const {
            return m_face;
        }

        template <typename T, typename FP, typename VP>
        Polyhedron_HalfEdge<T,FP,VP>* Polyhedron_HalfEdge<T,FP,VP>::next() const {
            return m_link.next();
        }

        template <typename T, typename FP, typename VP>
        Polyhedron_HalfEdge<T,FP,VP>* Polyhedron_HalfEdge<T,FP,VP>::previous() const {
            return m_link.previous();
        }

        template <typename T, typename FP, typename VP>
        vm::vec<T,3> Polyhedron_HalfEdge<T,FP,VP>::vector() const {
            return destination()->position() - origin()->position();
        }

        template <typename T, typename FP, typename VP>
        Polyhedron_HalfEdge<T,FP,VP>* Polyhedron_HalfEdge<T,FP,VP>::twin() const {
            assert(m_edge != nullptr);
            return m_edge->twin(this);
        }

        template <typename T, typename FP, typename VP>
        Polyhedron_HalfEdge<T,FP,VP>* Polyhedron_HalfEdge<T,FP,VP>::nextIncident() const {
            return previous()->twin();
        }


        template <typename T, typename FP, typename VP>
        Polyhedron_HalfEdge<T,FP,VP>* Polyhedron_HalfEdge<T,FP,VP>::previousIncident() const {
            return twin()->next();
        }

        template <typename T, typename FP, typename VP>
        bool Polyhedron_HalfEdge<T,FP,VP>::hasOrigins(const std::vector<vm::vec<T,3>>& origins, const T epsilon) const {
            const HalfEdge* edge = this;
            for (const vm::vec<T,3>& origin : origins) {
                if (!vm::is_equal(edge->origin()->position(), origin, epsilon)) {
                    return false;
                }
                edge = edge->next();
            }
            return true;
        }

        template <typename T, typename FP, typename VP>
        vm::plane_status Polyhedron_HalfEdge<T,FP,VP>::pointStatus(const vm::vec<T,3>& normal, const vm::vec<T,3>& point, const T epsilon) const {
            const auto planeNormal = vm::normalize(vm::cross(vm::normalize(vector()), normal));
            const auto plane = vm::plane<T,3>(origin()->position(), planeNormal);
            return plane.point_status(point, epsilon);
        }

        template <typename T, typename FP, typename VP>
        bool Polyhedron_HalfEdge<T,FP,VP>::colinear(const HalfEdge* other) const {
            assert(other != nullptr);
            assert(other != this);
            assert(destination() == other->origin());

            const auto& p0 = origin()->position();
            const auto& p1 = destination()->position();
            const auto& p2 = other->destination()->position();

            return vm::is_colinear(p0, p1, p2) && vm::dot(vector(), other->vector()) > 0.0;
        }

        template <typename T, typename FP, typename VP>
        void Polyhedron_HalfEdge<T,FP,VP>::setOrigin(Vertex* origin) {
            assert(origin != nullptr);
            m_origin = origin;
            setAsLeaving();
        }

        template <typename T, typename FP, typename VP>
        void Polyhedron_HalfEdge<T,FP,VP>::setEdge(Edge* edge) {
            assert(edge != nullptr);
            assert(m_edge == nullptr);
            m_edge = edge;
        }

        template <typename T, typename FP, typename VP>
        void Polyhedron_HalfEdge<T,FP,VP>::unsetEdge() {
            assert(m_edge != nullptr);
            m_edge = nullptr;
        }

        template <typename T, typename FP, typename VP>
        void Polyhedron_HalfEdge<T,FP,VP>::setFace(Face* face) {
            assert(face != nullptr);
            assert(m_face == nullptr);
            m_face = face;
        }

        template <typename T, typename FP, typename VP>
        void Polyhedron_HalfEdge<T,FP,VP>::unsetFace() {
            assert(m_face != nullptr);
            m_face = nullptr;
        }

        template <typename T, typename FP, typename VP>
        void Polyhedron_HalfEdge<T,FP,VP>::setAsLeaving() {
            m_origin->setLeaving(this);
        }
    }
}

#endif
