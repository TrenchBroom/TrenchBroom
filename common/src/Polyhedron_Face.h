/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef TrenchBroom_Polyhedron_Face_h
#define TrenchBroom_Polyhedron_Face_h

template <typename T, typename FP>
class Polyhedron<T,FP>::GetFaceLink {
public:
    typename DoublyLinkedList<Face, GetFaceLink>::Link& operator()(Face* face) const {
        return face->m_link;
    }
    
    const typename DoublyLinkedList<Face, GetFaceLink>::Link& operator()(const Face* face) const {
        return face->m_link;
    }
};

template <typename T, typename FP> template <typename P>
class Polyhedron<T,FP>::FaceT : public Allocator<FaceT<P> > {
private:
    friend class Polyhedron<T,FP>;
private:
    // Boundary is counter clockwise.
    HalfEdgeList m_boundary;
    P* m_payload;
    FaceLink m_link;
private:
    FaceT(HalfEdgeList& boundary) :
    m_payload(NULL),
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
        using std::swap;
        swap(m_boundary, boundary);
        
        assert(m_boundary.size() >= 3);
        updateBoundaryFaces(this);
    }
public:
    P* payload() const {
        return m_payload;
    }
    
    void setPayload(P* payload) {
        m_payload = payload;
    }
    
    Face* next() const {
        return m_link.next();
    }
    
    Face* previous() const {
        return m_link.previous();
    }

    size_t vertexCount() const {
        return m_boundary.size();
    }
    
    const HalfEdgeList& boundary() const {
        return m_boundary;
    }
    
    V origin() const {
        const HalfEdge* edge = *m_boundary.begin();
        return edge->origin()->position();
    }
    
    bool hasPositions(const typename V::List& positions, const T epsilon = Math::Constants<T>::almostZero()) const {
        if (positions.size() != vertexCount())
            return false;
        
        typename HalfEdgeList::const_iterator it, end;
        for (it = m_boundary.begin(), end = m_boundary.end(); it != end; ++it) {
            const HalfEdge* edge = *it;
            if (edge->hasOrigins(positions))
                return true;
        }
        return false;
    }
    
    V normal() const {
        const HalfEdge* first = m_boundary.front();
        const HalfEdge* current = first;
        V cross;
        do {
            const V& p1 = current->origin()->position();
            const V& p2 = current->next()->origin()->position();
            const V& p3 = current->next()->next()->origin()->position();
            cross = crossed(p2 - p1, p3 - p1);
            if (!cross.null())
                return cross.normalized();
            current = current->next();
        } while (first != current);
        return cross;
    }
    
    V center() const {
        return V::center(m_boundary.begin(), m_boundary.end(), GetVertexPosition());
    }
    
    T intersectWithRay(const Ray<T,3>& ray, const Math::Side side) const {
        const Plane<T,3> plane(origin(), normal());
        const T dot = plane.normal.dot(ray.direction);
        if (Math::zero(dot))
            return Math::nan<T>();
        if (side != Math::Side_Both) {
            if (side == Math::Side_Front) {
                if (dot > 0.0)
                    return Math::nan<T>();
            } else if (dot < 0.0) { // and side == Math::Side_Back
                return Math::nan<T>();
            }
        }
        
        typename HalfEdge::GetOriginPosition getPosition;
        return intersectPolygonWithRay(ray, plane, m_boundary.begin(), m_boundary.end(), getPosition);
    }
private:
    bool visibleFrom(const V& point) const {
        return pointStatus(point) == Math::PointStatus::PSAbove;
    }
    
    bool coplanar(const Face* other) const {
        assert(other != NULL);
        return normal().equals(other->normal(), Math::Constants<T>::colinearEpsilon());
    }
    
    bool isDegenerateTriangle(const T epsilon = Math::Constants<T>::almostZero()) const {
        if (vertexCount() != 3)
            return false;
        
        typename HalfEdgeList::ConstIterator it = m_boundary.iterator();
        const HalfEdge* edge = it.next();
        const T l1 = edge->squaredLength();
        
        edge = it.next();
        const T l2 = edge->squaredLength();
        
        edge = it.next();
        const T l3 = edge->squaredLength();
        
        const T epsilon2 = epsilon * epsilon;
        if (l1 > l2) {
            if (l1 > l3)
                return Math::eq(l1, l2 + l3, epsilon2);
        } else {
            if (l2 > l3)
                return Math::eq(l2, l1 + l3, epsilon2);
        }
        return Math::eq(l3, l1 + l2, epsilon2);
    }
    
    Math::PointStatus::Type pointStatus(const V& point, const T epsilon = Math::Constants<T>::pointStatusEpsilon()) const {
        const V norm = normal();
        const T distance = (point - origin()).dot(norm);
        if (distance > epsilon)
            return Math::PointStatus::PSAbove;
        if (distance < -epsilon)
            return Math::PointStatus::PSBelow;
        return Math::PointStatus::PSInside;
    }
    
    void flip() {
        m_boundary.reverse();
    }
    
    void insertIntoBoundaryBefore(HalfEdge* before, HalfEdge* edge) {
        assert(before != NULL);
        assert(edge != NULL);
        assert(before->face() == this);
        assert(edge->face() == this);
        
        m_boundary.insertBefore(before, edge, 1);
    }

    void insertIntoBoundaryAfter(HalfEdge* after, HalfEdge* edge) {
        assert(after != NULL);
        assert(edge != NULL);
        assert(after->face() == this);
        assert(edge->face() == this);
        
        m_boundary.insertAfter(after, edge, 1);
    }
    
    size_t removeFromBoundary(HalfEdge* from, HalfEdge* to) {
        assert(from != NULL);
        assert(to != NULL);
        assert(from->face() == this);
        assert(to->face() == this);
        
        const size_t removeCount = countAndSetFace(from, to->next(), NULL);
        m_boundary.remove(from, to, removeCount);
        return removeCount;
    }
    
    size_t removeFromBoundary(HalfEdge* edge) {
        removeFromBoundary(edge, edge);
        return 1;
    }
    
    size_t replaceBoundary(HalfEdge* edge, HalfEdge* with) {
        return replaceBoundary(edge, edge, with);
    }
    
    size_t replaceBoundary(HalfEdge* from, HalfEdge* to, HalfEdge* with) {
        assert(from != NULL);
        assert(to != NULL);
        assert(with != NULL);
        assert(from->face() == this);
        assert(to->face() == this);
        assert(with->face() == NULL);
        
        const size_t removeCount = countAndSetFace(from, to->next(), NULL);
        const size_t insertCount = countAndSetFace(with, with, this);
        m_boundary.replace(from, to, removeCount, with, insertCount);
        return removeCount;
    }
    
    void replaceEntireBoundary(HalfEdgeList& newBoundary) {
        using std::swap;
        
        updateBoundaryFaces(NULL);
        swap(m_boundary, newBoundary);
        updateBoundaryFaces(this);
    }

    size_t countAndSetFace(HalfEdge* from, HalfEdge* until, Face* face) {
        size_t count = 0;
        HalfEdge* cur = from;
        do {
            cur->setFace(face);
            cur = cur->next();
            ++count;
        } while (cur != until);
        return count;
    }
    
    void updateBoundaryFaces(Face* face) {
        HalfEdge* first = m_boundary.front();
        HalfEdge* current = first;
        do {
            current->setFace(face);
            current = current->next();
        } while (current != first);
    }
};

#endif
