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

#include "Model/Brush.h"
#include "Model/BrushGeometry.h"
#include "Model/Entity.h"
#include "Model/Face.h"
#include "Utility/VecMath.h"
#include "Utility/Console.h"

#include <algorithm>
#include <limits>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Utility {
        const float Grid::SnapAngle = Math::radians(15.0f);

        float Grid::snap(float f) const {
            if (!snap())
                return f;
            unsigned int actSize = actualSize();
            return Math::round(f / actSize);
        }

        float Grid::snapAngle(float a) const {
            if (!snap())
                return a;
            return SnapAngle * Math::round(a / SnapAngle);
        }

        float Grid::snapUp(float f, bool skip) const {
            if (!snap())
                return f;
            unsigned int actSize = actualSize();
            float s = actSize * std::ceil(f / actSize);
            if (skip && s == f)
                s += actualSize();
            return s;
        }

        float Grid::snapDown(float f, bool skip) const {
            if (!snap())
                return f;
            unsigned int actSize = actualSize();
            float s = actSize * std::floor(f / actSize);
            if (skip && s == f)
                s -= actualSize();
            return s;
        }
        
        float Grid::offset(float f) const {
            if (!snap())
                return 0.0f;
            return f - snap(f);
        }

        Vec3f Grid::snap(const Vec3f& p) const {
            if (!snap())
                return p;
            return Vec3f(snap(p.x), snap(p.y), snap(p.z));
        }
        
        Vec3f Grid::snapUp(const Vec3f& p, bool skip) const {
            if (!snap())
                return p;
            return Vec3f(snapUp(p.x, skip), snapUp(p.y, skip), snapUp(p.z, skip));
        }
        
        Vec3f Grid::snapDown(const Vec3f& p, bool skip) const {
            if (!snap())
                return p;
            return Vec3f(snapDown(p.x, skip), snapDown(p.y, skip), snapDown(p.z, skip));
        }
        
        Vec3f Grid::snapTowards(const Vec3f& p, const Vec3f& d, bool skip) const {
            if (!snap())
                return p;
            Vec3f result;
            if (Math::pos(d.x))         result.x = snapUp(p.x, skip);
            else if(Math::neg(d.x))     result.x = snapDown(p.x, skip);
            else                        result.x = snap(p.x);
            if (Math::pos(d.y))         result.y = snapUp(p.y, skip);
            else if(Math::neg(d.y))     result.y = snapDown(p.y, skip);
            else                        result.y = snap(p.y);
            if (Math::pos(d.z))         result.z = snapUp(p.z, skip);
            else if(Math::neg(d.z))     result.z = snapDown(p.z, skip);
            else                        result.z = snap(p.z);
            return result;
        }

        Vec3f Grid::offset(const Vec3f& p) const {
            if (!snap())
                return Vec3f::Null;
            return p - snap(p);
        }

        Vec3f Grid::snap(const Vec3f& p, const Plane& onPlane) const {
            Vec3f result;
            switch(onPlane.normal.firstComponent()) {
                case Axis::AX:
                    result.y = snap(p.y);
                    result.z = snap(p.z);
                    result.x = onPlane.x(result.y, result.z);
                    break;
                case Axis::AY:
                    result.x = snap(p.x);
                    result.z = snap(p.z);
                    result.y = onPlane.y(result.x, result.z);
                    break;
                case Axis::AZ:
                    result.x = snap(p.x);
                    result.y = snap(p.y);
                    result.z = onPlane.z(result.x, result.y);
                    break;
            }
            return result;
        }
        
        float Grid::intersectWithRay(const Ray& ray, unsigned int skip) const {
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
            if (!Math::isnan(distY) && (Math::isnan(dist) || std::abs(distY) < std::abs(dist)))
                dist = distY;
            if (!Math::isnan(distZ) && (Math::isnan(dist) || std::abs(distZ) < std::abs(dist)))
                dist = distZ;
            
            return dist;
        }

        Vec3f Grid::moveDeltaForEntity(const Vec3f& origin, const BBox& worldBounds, const Vec3f& delta) const {
            Vec3f newOrigin = snap(origin + delta);
            Vec3f actualDelta = newOrigin - origin;
            
            for (unsigned int i = 0; i < 3; i++)
                if ((actualDelta[i] > 0.0f) != (delta[i] > 0.0f))
                    actualDelta[i] = 0.0f;
            return actualDelta;
        }
        
        Vec3f Grid::moveDeltaForEntity(const Model::Face& face, const BBox& bounds, const BBox& worldBounds, const Ray& ray, const Vec3f& position) const {
            Plane dragPlane = Plane::alignedOrthogonalDragPlane(position, face.boundary().normal);
            
            Vec3f halfSize = bounds.size() * 0.5f;
            float offsetLength = halfSize.dot(dragPlane.normal);
            if (offsetLength < 0.0f)
                offsetLength *= -1.0f;
            Vec3f offset = dragPlane.normal * offsetLength;
            
            float dist = dragPlane.intersectWithRay(ray);
            Vec3f newPos = ray.pointAtDistance(dist);
            Vec3f delta = moveDeltaForEntity(bounds.center(), worldBounds, newPos - (bounds.center() - offset));
            
            Axis::Type a = dragPlane.normal.firstComponent();
            if (dragPlane.normal[a] > 0.0f) delta[a] = position[a] - bounds.min[a];
            else delta[a] = position[a] - bounds.max[a];
            
            return delta;
        }

        Vec3f Grid::moveDelta(const BBox& bounds, const BBox& worldBounds, const Vec3f& delta) const {
            Vec3f actualDelta = Vec3f::Null;
            for (unsigned int i = 0; i < 3; i++) {
                if (!Math::zero(delta[i])) {
                    float low  = snap(bounds.min[i] + delta[i]) - bounds.min[i];
                    float high = snap(bounds.max[i] + delta[i]) - bounds.max[i];
                    
                    if (low != 0 && high != 0)
                        actualDelta[i] = std::abs(high) < std::abs(low) ? high : low;
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
        
        Vec3f Grid::moveDelta(const Vec3f& point, const BBox& worldBounds, const Vec3f& delta) const {
            Vec3f actualDelta = Vec3f::Null;
            for (unsigned int i = 0; i < 3; i++)
                if (!Math::zero(delta[i]))
                    actualDelta[i] = snap(point[i] + delta[i]) - point[i];
            
            if (delta.lengthSquared() < (delta - actualDelta).lengthSquared())
                actualDelta = Vec3f::Null;
            
            return actualDelta;
        }

        Vec3f Grid::moveDelta(const Vec3f& delta) const {
            Vec3f actualDelta = Vec3f::Null;
            for (unsigned int i = 0; i < 3; i++)
                if (!Math::zero(delta[i]))
                    actualDelta[i] = snap(delta[i]);
            
            if (delta.lengthSquared() < (delta - actualDelta).lengthSquared())
                actualDelta = Vec3f::Null;
            
            return actualDelta;
        }

        Vec3f Grid::combineDeltas(const Vec3f& delta1, const Vec3f& delta2) const {
            if (delta1.lengthSquared() < delta2.lengthSquared())
                return delta1;
            return delta2;
        }

        Vec3f Grid::referencePoint(const BBox& bounds) {
            return snap(bounds.center());
        }

        /*
        float Grid::moveDistance(const Model::Face& face, Vec3f& delta) {
            float dist = delta.dot(face.boundary().normal);
            if (Math::zero(dist))
                return Math::nan();
            
            const Model::EdgeList& brushEdges = face.brush()->edges();
            const Model::VertexList& faceVertices = face.vertices();
            
            // the edge rays indicate the direction into which each vertex of the given face moves if the face is dragged
            std::vector<Ray> edgeRays;
            for (unsigned int i = 0; i < brushEdges.size(); i++) {
                const Model::Edge* edge = brushEdges[i];
                unsigned int c = 0;
                bool invert = false;
                
                if (Model::findElement(faceVertices, edge->start) < faceVertices.size()) {
                    c++;
                } 
                
                if (Model::findElement(faceVertices, edge->end) < faceVertices.size()) {
                    c++;
                    invert = true;
                }
                
                if (c == 1) {
                    Ray ray;
                    if (invert) {
                        ray.origin = edge->end->position;
                        ray.direction = (edge->start->position - edge->end->position).normalized();
                    } else {
                        ray.origin = edge->start->position;
                        ray.direction = (edge->end->position - edge->start->position).normalized();
                    }
                    
                    // depending on the direction of the drag vector, the rays must be inverted to reflect the
                    // actual movement of the vertices
                    if (dist > 0.0f)
                        ray.direction *= -1.0f;
                    
                    edgeRays.push_back(ray);
                }
            }
            
            Vec3f normDelta = face.boundary().normal * dist;
            unsigned int gridSkip = std::max<unsigned int>(0, static_cast<unsigned int>(normDelta.dot(normDelta.firstAxis())) / actualSize() - 1);
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
                    float vertexNormDist = vertexDelta.dot(face.boundary().normal);
                    
                    if (std::abs(vertexNormDist) < std::abs(actualDist)) {
                        Model::Face testFace(face.worldBounds(), face);
                        testFace.move(vertexNormDist, false);
                        if (!testFace.boundary().equals(face.boundary()))
                            actualDist = vertexNormDist;
                    }
                    
                    gridSkip++;
                }
            } while (actualDist == std::numeric_limits<float>::max());
            
            if (std::abs(actualDist) > std::abs(dist))
                return Math::nan();

            normDelta = face.boundary().normal * actualDist;
            Vec3f deltaNormalized = delta.normalized();
            delta = deltaNormalized * (normDelta.dot(deltaNormalized));
            
            return actualDist;
        }
         */
    }
}

