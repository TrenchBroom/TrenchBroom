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

#include "Grid.h"

#include <cmath>

#include "CollectionUtils.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"

namespace TrenchBroom {
    namespace View {
        Grid::Grid(const int size) :
        m_size(size),
        m_snap(true),
        m_visible(true) {}
        
        int Grid::size() const {
            return m_size;
        }
        
        void Grid::setSize(const int size) {
            assert(size <= MaxSize);
            assert(size >= MinSize);
            m_size = size;
            gridDidChangeNotifier();
        }
        
        void Grid::incSize() {
            if (m_size < MaxSize) {
                ++m_size;
                gridDidChangeNotifier();
            }
        }
        
        void Grid::decSize() {
            if (m_size > MinSize) {
                --m_size;
                gridDidChangeNotifier();
            }
        }
        
        FloatType Grid::actualSize() const {
            if (snap()) {
                return std::exp2(m_size);
            }
            return 1;
        }
        
        FloatType Grid::angle() const {
            return Math::radians(static_cast<FloatType>(15.0));
        }
        
        bool Grid::visible() const {
            return m_visible;
        }
        
        void Grid::toggleVisible() {
            m_visible = !m_visible;
            gridDidChangeNotifier();
        }
        
        bool Grid::snap() const {
            return m_snap;
        }
        
        void Grid::toggleSnap() {
            m_snap = !m_snap;
            gridDidChangeNotifier();
        }
        
        FloatType Grid::intersectWithRay(const Ray3& ray, const size_t skip) const {
            Vec3 planeAnchor;
            
            for (size_t i = 0; i < 3; ++i)
                planeAnchor[i] = ray.direction[i] > 0.0 ? snapUp(ray.origin[i], true) + skip * actualSize() : snapDown(ray.origin[i], true) - skip * actualSize();
            
            const FloatType distX = Plane3(planeAnchor, Vec3::PosX).intersectWithRay(ray);
            const FloatType distY = Plane3(planeAnchor, Vec3::PosY).intersectWithRay(ray);
            const FloatType distZ = Plane3(planeAnchor, Vec3::PosZ).intersectWithRay(ray);
            
            FloatType dist = distX;
            if (!Math::isnan(distY) && (Math::isnan(dist) || std::abs(distY) < std::abs(dist)))
                dist = distY;
            if (!Math::isnan(distZ) && (Math::isnan(dist) || std::abs(distZ) < std::abs(dist)))
                dist = distZ;
            return dist;
        }
        
        Vec3 Grid::moveDeltaForPoint(const Vec3& point, const BBox3& worldBounds, const Vec3& delta) const {
            const Vec3 newPoint = snap(point + delta);
            Vec3 actualDelta = newPoint - point;
            
            for (size_t i = 0; i < 3; ++i)
                if ((actualDelta[i] > 0.0) != (delta[i] > 0.0))
                    actualDelta[i] = 0.0;
            return actualDelta;
        }
        
        Vec3 Grid::moveDeltaForBounds(const Plane3& dragPlane, const BBox3& bounds, const BBox3& worldBounds, const Ray3& ray, const Vec3& position) const {
            
            // First, compute the snapped position under the mouse:
            const FloatType dist = dragPlane.intersectWithRay(ray);
            const Vec3 hitPoint = ray.pointAtDistance(dist);
            const Vec3 newPos = snapTowards(hitPoint, dragPlane, -ray.direction);
            const Vec3 offset = newPos - hitPoint;
            
            const Vec3 normal = dragPlane.normal;
            const Vec3 size = bounds.size();
            
            Vec3 newMinPos = newPos;
            for (size_t i = 0; i < 3; ++i) {
                if (Math::zero(offset[i])) {
                    if (normal[i] < 0.0)
                        newMinPos[i] -= size[i];
                } else {
                    if ((size[i] >= 0.0) != (ray.direction[i] >= 0.0))
                        newMinPos[i] -= size[i];
                }
            }

            return newMinPos - bounds.min;
        }
        
        Vec3 Grid::moveDelta(const BBox3& bounds, const BBox3& worldBounds, const Vec3& delta) const {
            Vec3 actualDelta = Vec3::Null;
            for (size_t i = 0; i < 3; ++i) {
                if (!Math::zero(delta[i])) {
                    const FloatType low  = snap(bounds.min[i] + delta[i]) - bounds.min[i];
                    const FloatType high = snap(bounds.max[i] + delta[i]) - bounds.max[i];
                    
                    if (low != 0.0 && high != 0.0)
                        actualDelta[i] = std::abs(high) < std::abs(low) ? high : low;
                    else if (low != 0.0)
                        actualDelta[i] = low;
                    else if (high != 0.0)
                        actualDelta[i] = high;
                    else
                        actualDelta[i] = 0.0;
                }
            }
            
            if (delta.squaredLength() < (delta - actualDelta).squaredLength())
                actualDelta = Vec3::Null;
            return actualDelta;
        }
        
        Vec3 Grid::moveDelta(const Vec3& point, const BBox3& worldBounds, const Vec3& delta) const {
            Vec3 actualDelta = Vec3::Null;
            for (size_t i = 0; i < 3; ++i)
                if (!Math::zero(delta[i]))
                    actualDelta[i] = snap(point[i] + delta[i]) - point[i];
            
            if (delta.squaredLength() < (delta - actualDelta).squaredLength())
                actualDelta = Vec3::Null;
            
            return actualDelta;
        }
        
        Vec3 Grid::moveDelta(const Vec3& delta) const {
            Vec3 actualDelta = Vec3::Null;
            for (unsigned int i = 0; i < 3; i++)
                if (!Math::zero(delta[i]))
                    actualDelta[i] = snap(delta[i]);
            
            if (delta.squaredLength() < (delta - actualDelta).squaredLength())
                actualDelta = Vec3::Null;
            
            return actualDelta;
        }
        
        Vec3 Grid::moveDelta(const Model::BrushFace* face, const Vec3& delta) const {
            const FloatType dist = delta.dot(face->boundary().normal);
            if (Math::zero(dist))
                return Vec3::Null;
            
            const Model::Brush* brush = face->brush();
            const Model::Brush::EdgeList brushEdges = brush->edges();
            const Model::BrushFace::VertexList faceVertices = face->vertices();
            
            // the edge rays indicate the direction into which each vertex of the given face moves if the face is dragged
            std::vector<Ray3> edgeRays;
            
            for (const Model::BrushEdge* edge : brushEdges) {
                size_t c = 0;
                bool originAtStart = true;

                bool startFound = false;
                bool endFound = false;
                
                for (const Model::BrushVertex* vertex : faceVertices) {
                    startFound |= (vertex->position() == edge->firstVertex()->position());
                    endFound |= (vertex->position() == edge->secondVertex()->position());
                    if (startFound && endFound)
                        break;
                }
                
                if (startFound)
                    c++;
                if (endFound) {
                    c++;
                    originAtStart = false;
                }
                
                if (c == 1) {
                    Ray3 ray;
                    if (originAtStart) {
                        ray.origin = edge->firstVertex()->position();
                        ray.direction = edge->vector().normalized();
                    } else {
                        ray.origin = edge->secondVertex()->position();
                        ray.direction = -edge->vector().normalized();
                    }
                    
                    // depending on the direction of the drag vector, the rays must be inverted to reflect the
                    // actual movement of the vertices
                    if (delta.dot(ray.direction) < 0.0)
                        ray.direction = -ray.direction;
                    
                    edgeRays.push_back(ray);
                }
            }
            
            Vec3 normDelta = face->boundary().normal * dist;
            size_t gridSkip = static_cast<size_t>(static_cast<size_t>(normDelta.dot(normDelta.firstAxis())) / actualSize());
            if (gridSkip > 0)
                --gridSkip;
            FloatType actualDist = std::numeric_limits<FloatType>::max();
            FloatType minDistDelta = std::numeric_limits<FloatType>::max();
            
            do {
                // Find the smallest drag distance at which the face boundary is actually moved
                // by intersecting the edge rays with the grid planes.
                // The distance of the ray origin to the closest grid plane is then multiplied by the ray
                // direction to yield the vector by which the vertex would be moved if the face was dragged
                // and the drag would snap the vertex onto the previously selected grid plane.
                // This vector is then projected onto the face normal to yield the distance by which the face
                // must be dragged so that the vertex snaps to its closest grid plane.
                // Then, test if the resulting drag distance is smaller than the current candidate.
                
                for (size_t i = 0; i < edgeRays.size(); ++i) {
                    const Ray3& ray = edgeRays[i];
                    const FloatType vertexDist = intersectWithRay(ray, gridSkip);
                    const Vec3 vertexDelta = ray.direction * vertexDist;
                    const FloatType vertexNormDist = vertexDelta.dot(face->boundary().normal);
                    
                    const FloatType normDistDelta = std::abs(vertexNormDist - dist);
                    if (normDistDelta < minDistDelta) {
                        actualDist = vertexNormDist;
                        minDistDelta = normDistDelta;
                    }
                }
                ++gridSkip;
            } while (actualDist == std::numeric_limits<FloatType>::max());
            
            normDelta = face->boundary().normal * actualDist;
            const Vec3 deltaNormalized = delta.normalized();
            return deltaNormalized * normDelta.dot(deltaNormalized);
        }
        
        Vec3 Grid::combineDeltas(const Vec3& delta1, const Vec3& delta2) const {
            if (delta1.squaredLength() < delta2.squaredLength())
                return delta1;
            return delta2;
        }
        
        Vec3 Grid::referencePoint(const BBox3& bounds) const {
            return snap(bounds.center());
        }
    }
}
