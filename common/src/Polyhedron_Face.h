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

template <typename T>
typename Polyhedron<T>::FaceLink& Polyhedron<T>::FaceList::doGetLink(Face* face) const {
    return face->m_link;
}

template <typename T>
const typename Polyhedron<T>::FaceLink& Polyhedron<T>::FaceList::doGetLink(const Face* face) const {
    return face->m_link;
}

template <typename T>
class Polyhedron<T>::Face {
private:
    friend class FaceList;
    friend class Polyhedron<T>;
private:
    HalfEdgeList m_boundary;
    FaceLink m_link;
private:
    Face(const HalfEdgeList& boundary) :
    m_boundary(boundary),
    m_link(this) {
        assert(m_boundary.size() >= 3);
        
        typename HalfEdgeList::Iterator it = m_boundary.iterator();
        while (it.hasNext()) {
            HalfEdge* edge = it.next();
            edge->setFace(this);
        }
    }
public:
    ~Face() {
        m_boundary.deleteAll();
    }
    
    size_t vertexCount() const {
        return m_boundary.size();
    }
    
    const HalfEdgeList& boundary() const {
        return m_boundary;
    }
    
    V origin() const {
        typename HalfEdgeList::ConstIterator it = m_boundary.iterator();
        const HalfEdge* edge = it.next();
        return edge->origin()->position();
    }
    
    bool hasPositions(const typename V::List& positions, const T epsilon = Math::Constants<T>::almostZero()) const {
        if (positions.size() != vertexCount())
            return false;
        
        typename HalfEdgeList::ConstIterator it = m_boundary.iterator();
        while (it.hasNext()) {
            const HalfEdge* edge = it.next();
            if (edge->hasOrigins(positions))
                return true;
        }
        return false;
    }
    
    V normal() const {
        typename HalfEdgeList::ConstIterator it = m_boundary.iterator();
        const HalfEdge* edge = it.next();
        const V p1 = edge->origin()->position();
        
        edge = it.next();
        const V p2 = edge->origin()->position();
        
        edge = it.next();
        const V p3 = edge->origin()->position();
        
        return crossed(p2 - p1, p3 - p1).normalized();
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
    
    void removeFromBoundary(HalfEdge* edge) {
        removeFromBoundary(edge, edge);
    }
    
    void removeFromBoundary(HalfEdge* from, HalfEdge* to) {
        assert(from != NULL);
        assert(to != NULL);
        assert(from->face() == this);
        assert(to->face() == this);
        
        const size_t removeCount = countAndSetFace(from, to->next(), NULL);
        m_boundary.remove(from, to, removeCount);
    }
    
    void replaceBoundary(HalfEdge* edge, HalfEdge* with) {
        replaceBoundary(edge, edge, with);
    }
    
    void replaceBoundary(HalfEdge* from, HalfEdge* to, HalfEdge* with) {
        assert(from != NULL);
        assert(to != NULL);
        assert(with != NULL);
        assert(from->face() == this);
        assert(to->face() == this);
        assert(with->face() == NULL);
        
        const size_t removeCount = countAndSetFace(from, to->next(), NULL);
        const size_t insertCount = countAndSetFace(with, with, this);
        m_boundary.replace(from, to, removeCount, with, insertCount);
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
};

#endif
