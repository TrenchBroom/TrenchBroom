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

#ifndef TrenchBroom_Polyhedron_Vertex_h
#define TrenchBroom_Polyhedron_Vertex_h

#include "Polyhedron.h"

#include <kdl/intrusive_circular_list.h>

namespace TrenchBroom {
    namespace Model {
        template <typename T, typename FP, typename VP>
        kdl::intrusive_circular_link<Polyhedron_Vertex<T,FP,VP>>& Polyhedron_GetVertexLink<T,FP,VP>::operator()(Polyhedron_Vertex<T,FP,VP>* vertex) const {
            return vertex->m_link;
        }

        template <typename T, typename FP, typename VP>
        const kdl::intrusive_circular_link<Polyhedron_Vertex<T,FP,VP>>& Polyhedron_GetVertexLink<T,FP,VP>::operator()(const Polyhedron_Vertex<T,FP,VP>* vertex) const {
            return vertex->m_link;
        }

        template <typename T, typename FP, typename VP>
        Polyhedron_Vertex<T,FP,VP>::Polyhedron_Vertex(const vm::vec<T,3>& position) :
            m_position(position),
            m_leaving(nullptr),
#ifdef _MSC_VER
        // MSVC throws a warning because we're passing this to the FaceLink constructor, but it's okay because we just store the pointer there.
#pragma warning(push)
#pragma warning(disable : 4355)
m_link(this),
#pragma warning(pop)
#else
            m_link(this),
#endif
            m_payload(VP::defaultValue()) {}

        template <typename T, typename FP, typename VP>
        const vm::vec<T,3>& Polyhedron_Vertex<T,FP,VP>::position() const {
            return m_position;
        }

        template <typename T, typename FP, typename VP>
        void Polyhedron_Vertex<T,FP,VP>::setPosition(const vm::vec<T,3>& position) {
            m_position = position;
        }

        template <typename T, typename FP, typename VP>
        typename Polyhedron_Vertex<T,FP,VP>::HalfEdge* Polyhedron_Vertex<T,FP,VP>::leaving() const {
            return m_leaving;
        }

        template <typename T, typename FP, typename VP>
        void Polyhedron_Vertex<T,FP,VP>::setLeaving(HalfEdge* edge) {
            assert(edge == nullptr || edge->origin() == this);
            m_leaving = edge;
        }

        template <typename T, typename FP, typename VP>
        Polyhedron_Vertex<T,FP,VP>* Polyhedron_Vertex<T,FP,VP>::next() const {
            return m_link.next();
        }

        template <typename T, typename FP, typename VP>
        Polyhedron_Vertex<T,FP,VP>* Polyhedron_Vertex<T,FP,VP>::previous() const {
            return m_link.previous();
        }

        template <typename T, typename FP, typename VP>
        typename VP::Type Polyhedron_Vertex<T,FP,VP>::payload() const {
            return m_payload;
        }

        template <typename T, typename FP, typename VP>
        void Polyhedron_Vertex<T,FP,VP>::setPayload(typename VP::Type payload) {
            m_payload = payload;
        }

        template <typename T, typename FP, typename VP>
        bool Polyhedron_Vertex<T,FP,VP>::hasTwoIncidentEdges() const {
            assert(m_leaving != nullptr);
            HalfEdge* nextLeaving = m_leaving->nextIncident();
            return nextLeaving != m_leaving && nextLeaving->nextIncident() == m_leaving;
        }

        template <typename T, typename FP, typename VP>
        bool Polyhedron_Vertex<T,FP,VP>::incident(const Face* face) const {
            assert(face != nullptr);
            assert(m_leaving != nullptr);

            HalfEdge* curEdge = m_leaving;
            do {
                if (curEdge->face() == face)
                    return true;
                curEdge = curEdge->nextIncident();
            } while (curEdge != m_leaving);
            return false;
        }

        template <typename T, typename FP, typename VP>
        void Polyhedron_Vertex<T,FP,VP>::correctPosition(const size_t decimals, const T epsilon) {
            m_position = vm::correct(m_position, decimals, epsilon);
        }
    }
}

#endif
