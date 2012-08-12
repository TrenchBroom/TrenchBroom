/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Grid.h"

#include "Model/Map/Brush.h"
#include "Model/Map/BrushGeometry.h"
#include "Model/Map/Entity.h"
#include "Model/Map/Face.h"
#include "Utilities/VecMath.h"
#include "Utilities/Console.h"

#include <algorithm>
#include <limits>

namespace TrenchBroom {
    namespace Controller {
        unsigned int Grid::size() const {
            return m_size;
        }
        
        void Grid::setSize(unsigned int size) {
            if (m_size == size)
                return;
            
            if (size > MaxSize)
                m_size = MaxSize;
            else
                m_size = size;
            gridDidChange(*this);
        }

        unsigned int Grid::actualSize() const {
            if (m_snap)
                return 1 << m_size;
            return 1;
        }

        void Grid::toggleVisible() {
            m_visible = !m_visible;
            gridDidChange(*this);
        }
        
        bool Grid::visible() const {
            return m_visible;
        }
        
        void Grid::toggleSnap() {
            m_snap = !m_snap;
            gridDidChange(*this);
        }
        
        bool Grid::snap() const {
            return m_snap;
        }

        float Grid::snap(float f) {
            int actSize = actualSize();
            return actSize * Math::fround(f / actSize);
        }

        float Grid::snapUp(float f, bool skip) {
            int actSize = actualSize();
            float s = actSize * ceil(f / actSize);
            if (skip && s == f)
                s += actualSize();
            return s;
        }

        float Grid::snapDown(float f, bool skip) {
            int actSize = actualSize();
            float s = actSize * floor(f / actSize);
            if (skip && s == f)
                s -= actualSize();
            return s;
        }
        
        float Grid::offset(float f) {
            return f - snap(f);
        }

        Vec3f Grid::snap(const Vec3f& p) {
            return Vec3f(snap(p.x), snap(p.y), snap(p.z));
        }
        
        Vec3f Grid::snapUp(const Vec3f& p, bool skip) {
            return Vec3f(snapUp(p.x, skip), snapUp(p.y, skip), snapUp(p.z, skip));
        }
        
        Vec3f Grid::snapDown(const Vec3f& p, bool skip) {
            return Vec3f(snapDown(p.x, skip), snapDown(p.y, skip), snapDown(p.z, skip));
        }
        
        Vec3f Grid::snapTowards(const Vec3f& p, const Vec3f& d, bool skip) {
            Vec3f result;
            if (Math::fpos(d.x))        result.x = snapUp(p.x, skip);
            else if(Math::fneg(d.x))    result.x = snapDown(p.x, skip);
            else                        result.x = snap(p.x);
            if (Math::fpos(d.y))        result.y = snapUp(p.y, skip);
            else if(Math::fneg(d.y))    result.y = snapDown(p.y, skip);
            else                        result.y = snap(p.y);
            if (Math::fpos(d.z))        result.z = snapUp(p.z, skip);
            else if(Math::fneg(d.z))    result.z = snapDown(p.z, skip);
            else                        result.z = snap(p.z);
            return result;
        }

        Vec3f Grid::offset(const Vec3f& p) {
            return p - snap(p);
        }

        
        float Grid::intersectWithRay(const Ray& ray, unsigned int skip) {
            Vec3f planeAnchor;
            
            planeAnchor.x = ray.direction.x > 0 ? snapUp(ray.origin.x, true) + skip * actualSize() : snapDown(ray.origin.x, true) - skip * actualSize();
            planeAnchor.y = ray.direction.y > 0 ? snapUp(ray.origin.y, true) + skip * actualSize() : snapDown(ray.origin.y, true) - skip * actualSize();
            planeAnchor.z = ray.direction.z > 0 ? snapUp(ray.origin.z, true) + skip * actualSize() : snapDown(ray.origin.z, true) - skip * actualSize();

            Plane plane(Vec3f::PosX, planeAnchor);
            float distX = plane.intersectWithRay(ray);
            
            plane = Plane(Vec3f::PosY, planeAnchor);
            float distY = plane.intersectWithRay(ray);
            
            plane = Plane(Vec3f::PosZ, planeAnchor);
            float distZ = plane.intersectWithRay(ray);
            
            float dist = distX;
            if (!Math::isnan(distY) && (Math::isnan(dist) || fabsf(distY) < fabsf(dist)))
                dist = distY;
            if (!Math::isnan(distZ) && (Math::isnan(dist) || fabsf(distZ) < fabsf(dist)))
                dist = distZ;
            
            return dist;
        }

        Vec3f Grid::moveDeltaForEntity(const Vec3f& origin, const BBox& worldBounds, const Vec3f& delta) {
            Vec3f newOrigin = snap(origin + delta);
            Vec3f actualDelta = newOrigin - origin;
            
            for (unsigned int i = 0; i < 3; i++)
                if (actualDelta[i] > 0 != delta[i] > 0)
                    actualDelta[i] = 0;
            return actualDelta;
        }

        Vec3f Grid::moveDelta(const BBox& bounds, const BBox& worldBounds, const Vec3f& delta) {
            Vec3f actualDelta = Vec3f::Null;
            for (int i = 0; i < 3; i++) {
                if (!Math::fzero(delta[i])) {
                    float low  = snap(bounds.min[i] + delta[i]) - bounds.min[i];
                    float high = snap(bounds.max[i] + delta[i]) - bounds.max[i];
                    
                    if (low != 0 && high != 0)
                        actualDelta[i] = fabsf(high) < fabsf(low) ? high : low;
                    else if (low != 0)
                        actualDelta[i] = low;
                    else if (high != 0)
                        actualDelta[i] = high;
                    else
                        actualDelta[i] = 0;
                }
            }
            
            if (delta.lengthSquared() < (delta - actualDelta).lengthSquared())
                actualDelta = Vec3f::Null;
            
            return actualDelta;
        }
        
        Vec3f Grid::moveDelta(const Vec3f& point, const BBox& worldBounds, const Vec3f& delta) {
            Vec3f actualDelta = Vec3f::Null;
            for (int i = 0; i < 3; i++)
                if (!Math::fzero(delta[i]))
                    actualDelta[i] = snap(point[i] + delta[i]) - point[i];
            
            if (delta.lengthSquared() < (delta - actualDelta).lengthSquared())
                actualDelta = Vec3f::Null;
            
            return actualDelta;
        }

        Vec3f Grid::moveDelta(const Vec3f& delta) {
            Vec3f actualDelta = Vec3f::Null;
            for (int i = 0; i < 3; i++)
                if (!Math::fzero(delta[i]))
                    actualDelta[i] = snap(delta[i]);
            
            if (delta.lengthSquared() < (delta - actualDelta).lengthSquared())
                actualDelta = Vec3f::Null;
            
            return actualDelta;
        }

        Vec3f Grid::combineDeltas(const Vec3f& delta1, const Vec3f& delta2) {
            if (delta1.lengthSquared() < delta2.lengthSquared())
                return delta1;
            return delta2;
        }

        float Grid::moveDistance(const Model::Face& face, Vec3f& delta) {
            float dist = delta | face.boundary.normal;
            if (Math::fzero(dist))
                return Math::nan();
            
            const Model::EdgeList& brushEdges = face.brush()->geometry->edges;
            const Model::VertexList& faceVertices = face.side->vertices;
            
            // the edge rays indicate the direction into which each vertex of the given face moves if the face is dragged
            std::vector<Ray> edgeRays;
            for (unsigned int i = 0; i < brushEdges.size(); i++) {
                const Model::Edge* edge = brushEdges[i];
                unsigned int c = 0;
                bool invert = false;
                
                if (Model::indexOf(faceVertices, edge->start) < faceVertices.size()) {
                    c++;
                } 
                
                if (Model::indexOf(faceVertices, edge->end) < faceVertices.size()) {
                    c++;
                    invert = true;
                }
                
                if (c == 1) {
                    Ray ray;
                    if (invert) {
                        ray.origin = edge->end->position;
                        ray.direction = (edge->start->position - edge->end->position).normalize();
                    } else {
                        ray.origin = edge->start->position;
                        ray.direction = (edge->end->position - edge->start->position).normalize();
                    }
                    
                    // depending on the direction of the drag vector, the rays must be inverted to reflect the
                    // actual movement of the vertices
                    if (dist > 0.0f)
                        ray.direction *= -1.0f;
                    
                    edgeRays.push_back(ray);
                }
            }
            
            Vec3f normDelta = face.boundary.normal * dist;
            int gridSkip = std::max<int>(0, static_cast<int>(normDelta | normDelta.firstAxis()) / actualSize() - 1);
            float actualDist = std::numeric_limits<float>::max();
            
            do {
                // Find the smallest drag distance at which the face boundary is actually moved
                // by intersecting the edge rays with the grid planes.
                // The distance of the ray origin to the closest grid plane is then multiplied by the ray
                // direction to yield the vector by which the vertex would be moved if the face was dragged
                // and the drag would snap the vertex onto the previously selected grid plane.
                // This vector is then projected onto the face normal to yield the distance by which the face
                // must be dragged so that the vertex snaps to its closest grid plane.
                // Then, test if the resulting drag distance is smaller than the current candidate and if it is, see if
                // it is large enough so that the face boundary changes when the drag is applied.
                
                for (unsigned int i = 0; i < edgeRays.size(); i++) {
                    const Ray& ray = edgeRays[i];
                    float vertexDist = intersectWithRay(ray, gridSkip);
                    Vec3f vertexDelta = ray.direction * vertexDist;
                    float vertexNormDist = vertexDelta | face.boundary.normal;
                    
                    if (fabsf(vertexNormDist) < fabsf(actualDist)) {
                        Model::Face testFace(face.worldBounds, face);
                        testFace.move(vertexNormDist, false);
                        if (!testFace.boundary.equals(face.boundary))
                            actualDist = vertexNormDist;
                    }
                    
                    gridSkip++;
                }
            } while (actualDist == std::numeric_limits<float>::max());
            
            if (fabsf(actualDist) > fabsf(dist))
                return Math::nan();

            normDelta = face.boundary.normal * actualDist;
            Vec3f deltaNormalized = delta.normalize();
            delta = deltaNormalized * (normDelta | deltaNormalized);
            
            return actualDist;
        }
    }
}

