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

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Utility {
        const float Grid::SnapAngle = Math<float>::radians(15.0f);

        float Grid::snap(float f) const {
            if (!snap())
                return f;
            unsigned int actSize = actualSize();
            return actSize * Math<float>::round(f / actSize);
        }

        float Grid::snapAngle(float a) const {
            if (!snap())
                return a;
            return SnapAngle * Math<float>::round(a / SnapAngle);
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
            return Vec3f(snap(p.x()), snap(p.y()), snap(p.z()));
        }
        
        Vec3f Grid::snapUp(const Vec3f& p, bool skip) const {
            if (!snap())
                return p;
            return Vec3f(snapUp(p.x(), skip), snapUp(p.y(), skip), snapUp(p.z(), skip));
        }
        
        Vec3f Grid::snapDown(const Vec3f& p, bool skip) const {
            if (!snap())
                return p;
            return Vec3f(snapDown(p.x(), skip), snapDown(p.y(), skip), snapDown(p.z(), skip));
        }
        
        Vec3f Grid::snapTowards(const Vec3f& p, const Vec3f& d, bool skip) const {
            if (!snap())
                return p;
            Vec3f result;
            if (Math<float>::pos(d.x()))        result[0] = snapUp(p.x(), skip);
            else if(Math<float>::neg(d.x()))    result[0] = snapDown(p.x(), skip);
            else                                result[0] = snap(p.x());
            if (Math<float>::pos(d.y()))        result[1] = snapUp(p.y(), skip);
            else if(Math<float>::neg(d.y()))    result[1] = snapDown(p.y(), skip);
            else                                result[1] = snap(p.y());
            if (Math<float>::pos(d.z()))        result[2] = snapUp(p.z(), skip);
            else if(Math<float>::neg(d.z()))    result[2] = snapDown(p.z(), skip);
            else                                result[2] = snap(p.z());
            return result;
        }

        Vec3f Grid::offset(const Vec3f& p) const {
            if (!snap())
                return Vec3f::Null;
            return p - snap(p);
        }

        Vec3f Grid::snap(const Vec3f& p, const Planef& onPlane) const {
            Vec3f result;
            switch(onPlane.normal.firstComponent()) {
                case Axis::AX:
                    result[1] = snap(p.y());
                    result[2] = snap(p.z());
                    result[0] = onPlane.x(result.y(), result.z());
                    break;
                case Axis::AY:
                    result[0] = snap(p.x());
                    result[2] = snap(p.z());
                    result[1] = onPlane.y(result.x(), result.z());
                    break;
                case Axis::AZ:
                    result[0] = snap(p.x());
                    result[1] = snap(p.y());
                    result[2] = onPlane.z(result.x(), result.y());
                    break;
            }
            return result;
        }
        
        float Grid::intersectWithRay(const Rayf& ray, const size_t skip) const {
            Vec3f planeAnchor;
            
            for (size_t i = 0; i < 3; ++i)
                planeAnchor[i] = (ray.direction[i] > 0.0f
                                  ? snapUp(ray.origin[i], true) + skip * actualSize()
                                  : snapDown(ray.origin[i], true) - skip * actualSize());

            const float distX = Planef(Vec3f::PosX, planeAnchor).intersectWithRay(ray);
            const float distY = Planef(Vec3f::PosY, planeAnchor).intersectWithRay(ray);
            const float distZ = Planef(Vec3f::PosZ, planeAnchor).intersectWithRay(ray);
            
            float dist = distX;
            if (!Math<float>::isnan(distY) && (Math<float>::isnan(dist) || std::abs(distY) < std::abs(dist)))
                dist = distY;
            if (!Math<float>::isnan(distZ) && (Math<float>::isnan(dist) || std::abs(distZ) < std::abs(dist)))
                dist = distZ;
            
            return dist;
        }

        Vec3f Grid::moveDeltaForPoint(const Vec3f& point, const BBoxf& worldBounds, const Vec3f& delta) const {
            Vec3f newPoint = snap(point + delta);
            Vec3f actualDelta = newPoint - point;
            
            for (unsigned int i = 0; i < 3; i++)
                if ((actualDelta[i] > 0.0f) != (delta[i] > 0.0f))
                    actualDelta[i] = 0.0f;
            return actualDelta;
        }
        
        Vec3f Grid::moveDeltaForBounds(const Model::Face& face, const BBoxf& bounds, const BBoxf& worldBounds, const Rayf& ray, const Vec3f& position) const {
            const Planef dragPlane = Planef::alignedOrthogonalDragPlane(position, face.boundary().normal);
            
            const Vec3f halfSize = bounds.size() * 0.5f;
            float offsetLength = halfSize.dot(dragPlane.normal);
            if (offsetLength < 0.0f)
                offsetLength *= -1.0f;
            const Vec3f offset = dragPlane.normal * offsetLength;
            
            const float dist = dragPlane.intersectWithRay(ray);
            const Vec3f newPos = ray.pointAtDistance(dist);
            Vec3f delta = moveDeltaForPoint(bounds.center(), worldBounds, newPos - (bounds.center() - offset));
            
            Axis::Type a = dragPlane.normal.firstComponent();
            if (dragPlane.normal[a] > 0.0f) delta[a] = position[a] - bounds.min[a];
            else delta[a] = position[a] - bounds.max[a];
            
            return delta;
        }

        Vec3f Grid::moveDelta(const BBoxf& bounds, const BBoxf& worldBounds, const Vec3f& delta) const {
            Vec3f actualDelta = Vec3f::Null;
            for (unsigned int i = 0; i < 3; i++) {
                if (!Math<float>::zero(delta[i])) {
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
        
        Vec3f Grid::moveDelta(const Vec3f& point, const BBoxf& worldBounds, const Vec3f& delta) const {
            Vec3f actualDelta = Vec3f::Null;
            for (unsigned int i = 0; i < 3; i++)
                if (!Math<float>::zero(delta[i]))
                    actualDelta[i] = snap(point[i] + delta[i]) - point[i];
            
            if (delta.lengthSquared() < (delta - actualDelta).lengthSquared())
                actualDelta = Vec3f::Null;
            
            return actualDelta;
        }

        Vec3f Grid::moveDelta(const Vec3f& delta) const {
            Vec3f actualDelta = Vec3f::Null;
            for (unsigned int i = 0; i < 3; i++)
                if (!Math<float>::zero(delta[i]))
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

        Vec3f Grid::referencePoint(const BBoxf& bounds) {
            return snap(bounds.center());
        }

        Vec3f Grid::moveDelta(const Model::Face& face, const Vec3f delta) {
            const float dist = delta.dot(face.boundary().normal);
            if (Math<float>::zero(dist))
                return Vec3f::Null;
            
            const Model::EdgeList& brushEdges = face.brush()->edges();
            const Model::VertexList& faceVertices = face.vertices();
            
            // the edge rays indicate the direction into which each vertex of the given face moves if the face is dragged
            std::vector<Rayf> edgeRays;
            for (size_t i = 0; i < brushEdges.size(); ++i) {
                const Model::Edge* edge = brushEdges[i];
                size_t c = 0;
                bool originAtStart = true;
                
                if (Model::findElement(faceVertices, edge->start) < faceVertices.size())
                    c++;
                if (Model::findElement(faceVertices, edge->end) < faceVertices.size()) {
                    c++;
                    originAtStart = false;
                }
                
                if (c == 1) {
                    Rayf ray;
                    if (originAtStart) {
                        ray.origin = edge->start->position;
                        ray.direction = (edge->end->position - edge->start->position).normalized();
                    } else {
                        ray.origin = edge->end->position;
                        ray.direction = (edge->start->position - edge->end->position).normalized();
                    }
                    
                    // depending on the direction of the drag vector, the rays must be inverted to reflect the
                    // actual movement of the vertices
                    if (delta.dot(ray.direction) < 0.0f)
                        ray.direction = -ray.direction;
                    
                    edgeRays.push_back(ray);
                }
            }
            
            Vec3f normDelta = face.boundary().normal * dist;
            size_t gridSkip = static_cast<size_t>(normDelta.dot(normDelta.firstAxis())) / actualSize();
            if (gridSkip > 0)
                --gridSkip;
            float actualDist = std::numeric_limits<float>::max();
            float minDistDelta = std::numeric_limits<float>::max();
            
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
                    const Rayf& ray = edgeRays[i];
                    const float vertexDist = intersectWithRay(ray, gridSkip);
                    const Vec3f vertexDelta = ray.direction * vertexDist;
                    const float vertexNormDist = vertexDelta.dot(face.boundary().normal);
                    
                    const float normDistDelta = std::abs(vertexNormDist - dist);
                    if (normDistDelta < minDistDelta) {
                        actualDist = vertexNormDist;
                        minDistDelta = normDistDelta;
                    }
                }
                ++gridSkip;
            } while (actualDist == std::numeric_limits<float>::max());
            
            normDelta = face.boundary().normal * actualDist;
            const Vec3f deltaNormalized = delta.normalized();
            return deltaNormalized * normDelta.dot(deltaNormalized);
        }
    }
}

