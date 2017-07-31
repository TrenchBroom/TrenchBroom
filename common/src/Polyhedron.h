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

#include "Algorithms.h"
#include "Allocator.h"
#include "VecMath.h"
#include "DoublyLinkedList.h"

#include <cassert>
#include <queue>
#include <vector>

template <typename T, typename FP, typename VP>
class Polyhedron {
public:
    typedef Vec<T,3> V;
private:
    typedef typename Vec<T,3>::List PosList;
public:
    typedef std::list<Polyhedron> List;

    class Vertex;
    class Edge;
    class HalfEdge;
    class Face;
private:
    class GetVertexLink {
    public:
        typename DoublyLinkedList<Vertex, GetVertexLink>::Link& operator()(Vertex* vertex) const;
        const typename DoublyLinkedList<Vertex, GetVertexLink>::Link& operator()(const Vertex* vertex) const;
    };
    
    class GetEdgeLink {
    public:
        typename DoublyLinkedList<Edge, GetEdgeLink>::Link& operator()(Edge* edge) const;
        const typename DoublyLinkedList<Edge, GetEdgeLink>::Link& operator()(const Edge* edge) const;
    };

    class GetHalfEdgeLink {
    public:
        typename DoublyLinkedList<HalfEdge, GetHalfEdgeLink>::Link& operator()(HalfEdge* halfEdge) const;
        const typename DoublyLinkedList<HalfEdge, GetHalfEdgeLink>::Link& operator()(const HalfEdge* halfEdge) const;
    };

    class GetFaceLink {
    public:
        typename DoublyLinkedList<Face, GetFaceLink>::Link& operator()(Face* face) const;
        const typename DoublyLinkedList<Face, GetFaceLink>::Link& operator()(const Face* face) const;
    };
    
    typedef typename DoublyLinkedList<Vertex, GetVertexLink>::Link VertexLink;
    typedef typename DoublyLinkedList<Edge, GetEdgeLink>::Link EdgeLink;
    typedef typename DoublyLinkedList<HalfEdge, GetHalfEdgeLink>::Link HalfEdgeLink;
    typedef typename DoublyLinkedList<Face, GetFaceLink>::Link FaceLink;
public:
    typedef DoublyLinkedList<Vertex, GetVertexLink> VertexList;
    typedef DoublyLinkedList<Edge, GetEdgeLink> EdgeList;
    typedef DoublyLinkedList<HalfEdge, GetHalfEdgeLink> HalfEdgeList;
    typedef DoublyLinkedList<Face, GetFaceLink> FaceList;

    class VertexDistanceCmp;
    typedef std::set<Vertex*, VertexDistanceCmp> ClosestVertexSet;
private:
    typedef std::set<Vertex*> VertexSet;
    typedef std::set<Face*> FaceSet;
public:
    class Vertex : public Allocator<Vertex> {
    public:
        typedef std::set<Vertex*> Set;
        typedef std::vector<Vertex*> List;
    private:
        friend class Polyhedron<T,FP,VP>;
    private:
        V m_position;
        VertexLink m_link;
        HalfEdge* m_leaving;
        typename VP::Type m_payload;
    private:
        Vertex(const V& position);
    public:
        typename VP::Type payload() const;
        void setPayload(typename VP::Type payload);

        const V& position() const;
        Vertex* next() const;
        Vertex* previous() const;
        HalfEdge* leaving() const;
        bool incident(const Face* face) const;
    private:
        HalfEdge* findConnectingEdge(const Vertex* vertex) const;
        HalfEdge* findColinearEdge(const HalfEdge* arriving) const;
        void correctPosition(const size_t decimals = 0, const T epsilon = Math::Constants<T>::correctEpsilon());
        void setPosition(const V& position);
        void setLeaving(HalfEdge* edge);
    };

    class Edge : public Allocator<Edge> {
    public:
        typedef std::vector<Edge*> List;
    private:
        friend class Polyhedron<T,FP,VP>;
        
        HalfEdge* m_first;
        HalfEdge* m_second;
        EdgeLink m_link;
    private:
        Edge(HalfEdge* first, HalfEdge* second = NULL);
    public:
        Vertex* firstVertex() const;
        Vertex* secondVertex() const;
        Vertex* otherVertex(Vertex* vertex) const;
        HalfEdge* firstEdge() const;
        HalfEdge* secondEdge() const;
        HalfEdge* twin(const HalfEdge* halfEdge) const;
        V vector() const;
        V center() const;
        Face* firstFace() const;
        Face* secondFace() const;
        Vertex* commonVertex(const Edge* other) const;
        bool hasVertex(const Vertex* vertex) const;
        bool hasPosition(const V& position, T epsilon = Math::Constants<T>::almostZero()) const;
        bool hasPositions(const V& position1, const V& position2, T epsilon = Math::Constants<T>::almostZero()) const;
        bool orphaned() const;
        bool fullySpecified() const;
        bool contains(const V& point, T maxDistance = Math::Constants<T>::almostZero()) const;
        Edge* next() const;
        Edge* previous() const;
    private:
        Edge* split(const Plane<T,3>& plane);
        Edge* splitAtCenter();
        Edge* insertVertex(const V& position);
        void flip();
        void makeFirstEdge(HalfEdge* edge);
        void makeSecondEdge(HalfEdge* edge);
        void setFirstAsLeaving();
        void unsetSecondEdge();
        void setSecondEdge(HalfEdge* second);
    };
    
    class HalfEdge : public Allocator<HalfEdge> {
    private:
        friend class Polyhedron<T,FP,VP>;
        
        Vertex* m_origin;
        Edge* m_edge;
        Face* m_face;
        HalfEdgeLink m_link;
    private:
        HalfEdge(Vertex* origin);
    public:
        ~HalfEdge();
    public:
        Vertex* origin() const;
        Vertex* destination() const;
        T length() const;
        T squaredLength() const;
        V vector() const;
        Edge* edge() const;
        Face* face() const;
        HalfEdge* next() const;
        HalfEdge* previous() const;
        HalfEdge* twin() const;
        HalfEdge* previousIncident() const;
        HalfEdge* nextIncident() const;
        bool hasOrigins(const typename V::List& positions, T epsilon = Math::Constants<T>::almostZero()) const;
        String asString() const;
    private:
        Math::PointStatus::Type pointStatus(const V& faceNormal, const V& point) const;
        bool isLeavingEdge() const;
        bool colinear(const HalfEdge* other) const;
        void setOrigin(Vertex* origin);
        void setEdge(Edge* edge);
        void unsetEdge();
        void setFace(Face* face);
        void unsetFace();
        void setAsLeaving();
    };

    class Face : public Allocator<Face> {
    public:
        typedef std::set<Face*> Set;
    private:
        friend class Polyhedron<T,FP,VP>;
        
        // Boundary is counter clockwise.
        HalfEdgeList m_boundary;
        typename FP::Type m_payload;
        FaceLink m_link;
    private:
        Face(HalfEdgeList& boundary);
    public:
        typename FP::Type payload() const;
        void setPayload(typename FP::Type payload);
        Face* next() const;
        Face* previous() const;
        size_t vertexCount() const;
        const HalfEdgeList& boundary() const;
        HalfEdge* findHalfEdge(const V& origin) const;
        HalfEdge* findHalfEdge(const Vertex* origin) const;
        Edge* findEdge(const V& first, const V& second) const;
        void printBoundary() const;
        V origin() const;
        typename V::List vertexPositions() const;
        typename V::Set vertexPositionSet(T epsilon = Math::Constants<T>::almostZero()) const;
        bool hasVertexPosition(const V& position, T epsilon = Math::Constants<T>::almostZero()) const;
        bool hasVertexPositions(const typename V::List& positions, T epsilon = Math::Constants<T>::almostZero()) const;
        V normal() const;
        V center() const;
        T intersectWithRay(const Ray<T,3>& ray, const Math::Side side) const;
        Math::PointStatus::Type pointStatus(const V& point, T epsilon = Math::Constants<T>::pointStatusEpsilon()) const;
    private:
        // Template methods must remain private!
        template <typename O>
        void getVertexPositions(O output) const;
        typename Vertex::Set vertexSet() const;
        
        bool coplanar(const Face* other) const;
        bool verticesOnPlane(const Plane<T,3>& plane) const;
        void flip();
        void insertIntoBoundaryBefore(HalfEdge* before, HalfEdge* edge);
        void insertIntoBoundaryAfter(HalfEdge* after, HalfEdge* edge);
        size_t removeFromBoundary(HalfEdge* from, HalfEdge* to);
        size_t removeFromBoundary(HalfEdge* edge);
        size_t replaceBoundary(HalfEdge* edge, HalfEdge* with);
        size_t replaceBoundary(HalfEdge* from, HalfEdge* to, HalfEdge* with);
        void replaceEntireBoundary(HalfEdgeList& newBoundary);
        size_t countAndSetFace(HalfEdge* from, HalfEdge* until, Face* face);
        size_t countAndUnsetFace(HalfEdge* from, HalfEdge* until);
        void setBoundaryFaces();
        void unsetBoundaryFaces();
        void removeBoundaryFromEdges();
        void setLeavingEdges();
        
        size_t countSharedVertices(const Face* other) const;
        
        class RayIntersection;
        RayIntersection intersectWithRay(const Ray<T,3>& ray) const;
    };
private:
    class Seam;
public:
    struct GetVertexPosition {
        const V& operator()(const Vertex* vertex) const;
        const V& operator()(const HalfEdge* halfEdge) const;
    };

    class Callback {
    public:
        virtual ~Callback();
    public:
        virtual void vertexWasCreated(Vertex* vertex);
        virtual void vertexWillBeDeleted(Vertex* vertex);
        virtual void vertexWasAdded(Vertex* vertex);
        virtual void vertexWillBeRemoved(Vertex* vertex);
        virtual Plane<T,3> plane(const Face* face) const;
        virtual void faceWasCreated(Face* face);
        virtual void faceWillBeDeleted(Face* face);
        virtual void faceDidChange(Face* face);
        virtual void faceWasFlipped(Face* face);
        virtual void faceWasSplit(Face* original, Face* clone);
        virtual void facesWillBeMerged(Face* remaining, Face* toDelete);
    };
protected:
    VertexList m_vertices;
    EdgeList m_edges;
    FaceList m_faces;
    BBox<T,3> m_bounds;
public: // Constructors
    Polyhedron();
    
    Polyhedron(std::initializer_list<V> positions);
    Polyhedron(std::initializer_list<V> positions, Callback& callback);
    
    Polyhedron(const V& p1, const V& p2, const V& p3, const V& p4);
    Polyhedron(const V& p1, const V& p2, const V& p3, const V& p4, Callback& callback);

    Polyhedron(const BBox<T,3>& bounds);
    Polyhedron(const BBox<T,3>& bounds, Callback& callback);
    
    Polyhedron(const typename V::List& positions);
    Polyhedron(const typename V::List& positions, Callback& callback);

    Polyhedron(const typename V::Set& positions);
    Polyhedron(const typename V::Set& positions, Callback& callback);

    Polyhedron(const Polyhedron<T,FP,VP>& other);
    Polyhedron(Polyhedron<T,FP,VP>&& other);
private: // Constructor helpers
    void addPoints(const V& p1, const V& p2, const V& p3, const V& p4, Callback& callback);
    void setBounds(const BBox<T,3>& bounds, Callback& callback);
private: // Copy helper
    class Copy;
public: // Destructor
    virtual ~Polyhedron();
public: // operators
    // Implements both copy and move assignment.
    Polyhedron<T,FP,VP>& operator=(Polyhedron<T,FP,VP> other);
public: // swap function
    friend void swap(Polyhedron<T,FP,VP>& first, Polyhedron<T,FP,VP>& second) {
        using std::swap;
        swap(first.m_vertices, second.m_vertices);
        swap(first.m_edges, second.m_edges);
        swap(first.m_faces, second.m_faces);
        swap(first.m_bounds, second.m_bounds);
    }
public: // Operators
    bool operator==(const Polyhedron& other) const;
    bool operator!=(const Polyhedron& other) const;
public: // Accessors
    size_t vertexCount() const;
    const VertexList& vertices() const;
    bool hasVertex(const V& position, T epsilon = Math::Constants<T>::almostZero()) const;
    bool hasVertex(const typename V::List& positions, T epsilon = Math::Constants<T>::almostZero()) const;
    bool hasVertices(const typename V::List& positions, T epsilon = Math::Constants<T>::almostZero()) const;
    typename V::List vertexPositions() const;
    typename V::Set vertexPositionSet(T epsilon = Math::Constants<T>::almostZero()) const;
    void printVertices() const;
    
    size_t edgeCount() const;
    const EdgeList& edges() const;
    bool hasEdge(const V& pos1, const V& pos2, T epsilon = Math::Constants<T>::almostZero()) const;
    
    size_t faceCount() const;
    const FaceList& faces() const;
    bool hasFace(const typename V::List& positions, T epsilon = Math::Constants<T>::almostZero()) const;
    
    const BBox<T,3>& bounds() const;
    
    bool empty() const;
    bool point() const;
    bool edge() const;
    bool polygon() const;
    bool polyhedron() const;
    bool closed() const;

    void clear();
    
    struct FaceHit {
        Face* face;
        T distance;
        
        FaceHit(Face* i_face, const T i_distance);
        FaceHit();
        bool isMatch() const;
    };
    
    FaceHit pickFace(const Ray<T,3>& ray) const;
public: // General purpose methods
    Vertex* findVertexByPosition(const V& position, const Vertex* except = NULL, T epsilon = Math::Constants<T>::almostZero()) const;
    Vertex* findClosestVertex(const V& position) const;
    ClosestVertexSet findClosestVertices(const V& position) const;
    Edge* findEdgeByPositions(const V& pos1, const V& pos2, T epsilon = Math::Constants<T>::almostZero()) const;
    Face* findFaceByPositions(const typename V::List& positions, T epsilon = Math::Constants<T>::almostZero()) const;
private:
    template <typename O>
    void getVertexPositions(O output) const;
    
    bool hasVertex(const Vertex* vertex) const;
    bool hasEdge(const Edge* edge) const;
    bool hasFace(const Face* face) const;
    
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
    bool checkEdgeLengths(const T minLength = Math::Constants<T>::pointStatusEpsilon()) const;
    bool checkLeavingEdges(const Vertex* v) const;
    
    void updateBounds();
public: // Vertex correction and edge healing
    void correctVertexPositions(const size_t decimals = 0, const T epsilon = Math::Constants<T>::correctEpsilon());
    bool healEdges(const T minLength = Math::Constants<T>::pointStatusEpsilon());
    bool healEdges(Callback& callback, const T minLength = Math::Constants<T>::pointStatusEpsilon());
private:
    Edge* removeEdge(Edge* edge, Callback& callback);
    void removeDegenerateFace(Face* face, Callback& callback);
    void mergeNeighbours(HalfEdge* borderFirst, Callback& callback);
public: // Convex hull; adding and removing points
    void addPoints(const typename V::List& points);
    void addPoints(const typename V::List& points, Callback& callback);
private:
    template <typename I> void addPoints(I cur, I end);
    template <typename I> void addPoints(I cur, I end, Callback& callback);
public:
    Vertex* addPoint(const V& position);
    Vertex* addPoint(const V& position, Callback& callback);
    void removeVertex(Vertex* vertex);
    void removeVertex(Vertex* vertex, Callback& callback);
    void removeVertexByPosition(const V& position);
    void merge(const Polyhedron& other);
    void merge(const Polyhedron& other, Callback& callback);
private:
    Vertex* addFirstPoint(const V& position, Callback& callback);
    Vertex* addSecondPoint(const V& position, Callback& callback);
    
    Vertex* addThirdPoint(const V& position, Callback& callback);
    Vertex* addLinearlyDependentThirdPoint(const V& position, Callback& callback);
    Vertex* addLinearlyIndependentThirdPoint(const V& position, Callback& callback);
    
    Vertex* addFurtherPoint(const V& position, Callback& callback);
    Vertex* addFurtherPointToPolygon(const V& position, Callback& callback);
    Vertex* addPointToPolygon(const V& position, Callback& callback);
    void makePolygon(const typename V::List& positions, Callback& callback);
    Vertex* makePolyhedron(const V& position, Callback& callback);
    
    Vertex* addFurtherPointToPolyhedron(const V& position, Callback& callback);
    Vertex* addPointToPolyhedron(const V& position, const Seam& seam, Callback& callback);
    
    void removeSingleVertex(Vertex* vertex, Callback& callback);
    void removeVertexFromEdge(Vertex* vertex, Callback& callback);
    void removeVertexFromPolygon(Vertex* vertex, Callback& callback);
    void removeThirdVertexFromPolygon(Vertex* vertex, Callback& callback);
    void removeFurtherVertexFromPolygon(Vertex* vertex, Callback& callback);
    void removeVertexFromPolyhedron(Vertex* vertex, Callback& callback);
    
    class SplittingCriterion;
    class SplitByVisibilityCriterion;
    class SplitByConnectivityCriterion;
    class SplitByNormalCriterion;
    
    Seam createSeam(const SplittingCriterion& criterion);
    
    void split(const Seam& seam, Callback& callback);
    void deleteFaces(HalfEdge* current, FaceSet& visitedFaces, VertexList& verticesToDelete, Callback& callback);

    void sealWithSinglePolygon(const Seam& seam, Callback& callback);
    
    class ShiftSeamForSealing;
    void sealWithMultiplePolygons(Seam seam, Callback& callback);
    
    class ShiftSeamForWeaving;
    Vertex* weave(Seam seam, const V& position, Callback& callback);
public: // Clipping
    struct ClipResult {
        typedef enum {
            Type_ClipUnchanged,
            Type_ClipEmpty,
            Type_ClipSuccess
        } Type;
        
        const Type type;
        ClipResult(const Type i_type);
        bool unchanged() const;
        bool empty() const;
        bool success() const;
    };

    /**
     May throw a GeometryException if the polyhedron cannot be intersected with the given plane.
     */
    ClipResult clip(const Plane<T,3>& plane);
    
    /**
     May throw a GeometryException if the polyhedron cannot be intersected with the given plane.
     */
    ClipResult clip(const Plane<T,3>& plane, Callback& callback);

    /**
     Clips this polyhedron using all faces of the given polyhedron.
     */
    ClipResult clip(const Polyhedron& polyhedron);
    ClipResult clip(const Polyhedron& polyhedron, Callback& callback);
public: // Intersection
    Polyhedron intersect(const Polyhedron& other) const;
    Polyhedron intersect(Polyhedron other, const Callback& callback) const;
private:
    ClipResult checkIntersects(const Plane<T,3>& plane) const;
    
    /**
     May throw a GeometryException if the polyhedron cannot be intersected with the given plane due.
     */
    Seam intersectWithPlane(const Plane<T,3>& plane, Callback& callback);
    HalfEdge* findInitialIntersectingEdge(const Plane<T,3>& plane) const;
    HalfEdge* intersectWithPlane(HalfEdge* firstBoundaryEdge, const Plane<T,3>& plane, Callback& callback);
    void intersectWithPlane(HalfEdge* remainingFirst, HalfEdge* deletedFirst, Callback& callback);
    HalfEdge* findNextIntersectingEdge(HalfEdge* searchFrom, const Plane<T,3>& plane) const;
public: // Subtraction
    typedef std::list<Polyhedron> SubtractResult;
    
    SubtractResult subtract(const Polyhedron& subtrahend) const;
    SubtractResult subtract(const Polyhedron& subtrahend, const Callback& callback) const;
private:
    class Subtract;
    class Simplify;
    class Merge;
    
    static void addMissingFragments(Polyhedron::List& fragments, const Polyhedron& minuend, const Polyhedron& subtrahend, const Callback& callback);
    static FaceSet findUncoveredFaces(const Polyhedron::List& fragments, const Polyhedron& minuend, const Polyhedron& subtrahend, const Callback& callback);
    static typename Plane<T,3>::Set findIgnoredPlanes(const Polyhedron& minuend, const Polyhedron& subtrahend, const Callback& callback);
    static void addIgnoredPlanes(const Polyhedron& polyhedron, bool flip, const Callback& callback, typename Plane<T,3>::Set& result);
    
    static bool isCoveredByFragment(const Face* face, const Polyhedron::List& fragments, const Polyhedron& ignore);
public: // geometrical queries
    bool contains(const V& point, const Callback& callback = Callback()) const;
    bool contains(const Polyhedron& other, const Callback& callback = Callback()) const;
    bool intersects(const Polyhedron& other, const Callback& callback = Callback()) const;
private:
    static bool pointIntersectsPoint(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback = Callback());
    static bool pointIntersectsEdge(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback = Callback());
    static bool pointIntersectsPolygon(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback = Callback());
    static bool pointIntersectsPolyhedron(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback = Callback());
    
    static bool edgeIntersectsPoint(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback = Callback());
    static bool edgeIntersectsEdge(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback = Callback());
    static bool edgeIntersectsPolygon(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback = Callback());
    static bool edgeIntersectsPolyhedron(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback = Callback());
    
    static bool edgeIntersectsFace(const Edge* lhsEdge, const Face* rhsFace);
    
    static bool polygonIntersectsPoint(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback = Callback());
    static bool polygonIntersectsEdge(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback = Callback());
    static bool polygonIntersectsPolygon(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback = Callback());
    static bool polygonIntersectsPolyhedron(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback = Callback());
    
    static bool faceIntersectsFace(const Face* lhsFace, const Face* rhsFace);
    
    static bool polyhedronIntersectsPoint(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback = Callback());
    static bool polyhedronIntersectsEdge(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback = Callback());
    static bool polyhedronIntersectsPolygon(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback = Callback());
    static bool polyhedronIntersectsPolyhedron(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback = Callback());

    static bool separate(const Face* faces, const Vertex* vertices, const Callback& callback);
    static Math::PointStatus::Type pointStatus(const Plane<T,3>& plane, const Vertex* vertices);
};

#endif
