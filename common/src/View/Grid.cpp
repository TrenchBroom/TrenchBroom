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

#include "FloatType.h"
#include "Model/Brush.h"
#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/Polyhedron.h"

#include <vecmath/vec.h>
#include <vecmath/ray.h>
#include <vecmath/intersection.h>
#include <vecmath/scalar.h>

#include <cmath>

namespace TrenchBroom {
    namespace View {
        Grid::Grid(const int size) :
        m_size(size),
        m_snap(true),
        m_visible(true) {}

        FloatType Grid::actualSize(const int size) {
            return std::exp2(size);
        }

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
                return actualSize(m_size);
            }
            return FloatType(1);
        }

        FloatType Grid::angle() const {
            return vm::to_radians(static_cast<FloatType>(15.0));
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

        FloatType Grid::intersectWithRay(const vm::ray3& ray, const size_t skip) const {
            vm::vec3 planeAnchor;

            for (size_t i = 0; i < 3; ++i) {
                planeAnchor[i] = ray.direction[i] > 0.0 ? snapUp(ray.origin[i], true) + static_cast<FloatType>(skip) * actualSize() : snapDown(ray.origin[i], true) - static_cast<FloatType>(skip) * actualSize();
            }

            const auto distX = vm::intersect_ray_plane(ray, vm::plane3(planeAnchor, vm::vec3::pos_x()));
            const auto distY = vm::intersect_ray_plane(ray, vm::plane3(planeAnchor, vm::vec3::pos_y()));
            const auto distZ = vm::intersect_ray_plane(ray, vm::plane3(planeAnchor, vm::vec3::pos_z()));

            auto dist = distX;
            if (!vm::is_nan(distY) && (vm::is_nan(dist) || std::abs(distY) < std::abs(dist))) {
                dist = distY;
            }
            if (!vm::is_nan(distZ) && (vm::is_nan(dist) || std::abs(distZ) < std::abs(dist))) {
                dist = distZ;
            }
            return dist;
        }

        vm::vec3 Grid::moveDeltaForPoint(const vm::vec3& point, const vm::vec3& delta) const {
            const auto newPoint = snap(point + delta);
            auto actualDelta = newPoint - point;

            for (size_t i = 0; i < 3; ++i) {
                if ((actualDelta[i] > static_cast<FloatType>(0.0)) != (delta[i] > static_cast<FloatType>(0.0))) {
                    actualDelta[i] = static_cast<FloatType>(0.0);
                }
            }
            return actualDelta;
        }

        /**
         * Suggests a placement for a box of the given size following some heuristics described below.
         *
         * The placement is returned as a delta from bounds.min (which is not used, otherwise).
         * Intended to be used for placing objects (e.g. when pasting, or dragging from the entity browser)
         *
         * - One of the box corners is placed at the ray/targetPlane intersection, grid snapped
         *   (snapping towards the ray origin)
         * - Exception to the previous point: if the targetPlane is axial plane,
         *   we'll treat the plane's normal axis as "on grid" even if it's not. This allows, e.g. pasting on
         *   top of 1 unit thick floor detail on grid 8.
         * - The box is positioned so it's above the targetPlane (snapped to axial). It might
         *   clip into targetPlane.
         * - The box is positioned so it's on the opposite side of the ray/targetPlane intersection point
         *   from the pickRay source. The effect of this rule is, when dragging an entity from the entity browser
         *   onto the map, the mouse is always grabbing the edge of the entity bbox that's closest to the camera.
         */
        vm::vec3 Grid::moveDeltaForBounds(const vm::plane3& targetPlane, const vm::bbox3& bounds, const vm::bbox3& /* worldBounds */, const vm::ray3& ray) const {
            // First, find the ray/plane intersection, and snap it to grid.
            // This will become one of the corners of our resulting bbox.
            // Note that this means we might let the box clip into the plane somewhat.
            const FloatType dist = vm::intersect_ray_plane(ray, targetPlane);
            const vm::vec3 hitPoint = vm::point_at_distance(ray, dist);

            // Local axis system where Z is the largest magnitude component of targetPlane.normal, and X and Y are
            // the other two axes.
            const size_t localZ = vm::find_abs_max_component(targetPlane.normal, 0);
            const size_t localX = vm::find_abs_max_component(targetPlane.normal, 1);
            const size_t localY = vm::find_abs_max_component(targetPlane.normal, 2);

            vm::vec3 firstCorner = snapTowards(hitPoint, -ray.direction);
            if (vm::is_equal(targetPlane.normal,
                             vm::get_abs_max_component_axis(targetPlane.normal),
                             vm::C::almost_zero())) {
                // targetPlane is axial. As a special case, only snap X and Y
                firstCorner[localZ] = hitPoint[localZ];
            }

            vm::vec3 newMinPos = firstCorner;

            // The remaining task is to decide which corner of the bbox firstCorner is.
            // Start with using firstCorner as the bbox min, and for each axis,
            // we'll either subtract the box size along that axis (or not) to shift the box position.

            // 1. Look at the component of targetPlane.normal with the greatest magnitude.
            if (targetPlane.normal[localZ] < 0.0) {
                // The plane normal we're snapping to is negative in localZ (e.g. a ceiling), so
                // align the box max with the snap point on that axis
                newMinPos[localZ] -= bounds.size()[localZ];
            }
            // else, the plane normal is positive in localZ (e.g. a floor), so newMinPos
            // is already the correct box min position on that axis.

            // 2. After dealing with localZ, we'll adjust the box position on the other
            // two axes so it's furthest from the source of the ray. See moveDeltaForBounds() docs
            // for the rationale.
            if (ray.direction[localX] < 0.0) {
                newMinPos[localX] -= bounds.size()[localX];
            }
            if (ray.direction[localY] < 0.0) {
                newMinPos[localY] -= bounds.size()[localY];
            }

            return newMinPos - bounds.min;
        }

        vm::vec3 Grid::moveDelta(const vm::bbox3& bounds, const vm::vec3& delta) const {
            auto actualDelta = vm::vec3::zero();
            for (size_t i = 0; i < 3; ++i) {
                if (!vm::is_zero(delta[i], vm::C::almost_zero())) {
                    const auto low  = snap(bounds.min[i] + delta[i]) - bounds.min[i];
                    const auto high = snap(bounds.max[i] + delta[i]) - bounds.max[i];

                    if (low != static_cast<FloatType>(0.0) && high != static_cast<FloatType>(0.0)) {
                        actualDelta[i] = std::abs(high) < std::abs(low) ? high : low;
                    } else if (low != static_cast<FloatType>(0.0)) {
                        actualDelta[i] = low;
                    } else if (high != static_cast<FloatType>(0.0)) {
                        actualDelta[i] = high;
                    } else {
                        actualDelta[i] = static_cast<FloatType>(0.0);
                    }
                }
            }

            if (vm::squared_length(delta) < vm::squared_length(delta - actualDelta)) {
                actualDelta = vm::vec3::zero();
            }
            return actualDelta;
        }

        vm::vec3 Grid::moveDelta(const vm::vec3& point, const vm::vec3& delta) const {
            auto actualDelta = vm::vec3::zero();
            for (size_t i = 0; i < 3; ++i) {
                if (!vm::is_zero(delta[i], vm::C::almost_zero())) {
                    actualDelta[i] = snap(point[i] + delta[i]) - point[i];
                }
            }

            if (vm::squared_length(delta) < vm::squared_length(delta - actualDelta)) {
                actualDelta = vm::vec3::zero();
            }

            return actualDelta;
        }

        vm::vec3 Grid::moveDelta(const vm::vec3& delta) const {
            auto actualDelta = vm::vec3::zero();
            for (unsigned int i = 0; i < 3; i++) {
                if (!vm::is_zero(delta[i], vm::C::almost_zero())) {
                    actualDelta[i] = snap(delta[i]);
                }
            }

            if (vm::squared_length(delta) < vm::squared_length(delta - actualDelta)) {
                actualDelta = vm::vec3::zero();
            }

            return actualDelta;
        }

        vm::vec3 Grid::moveDelta(const Model::BrushFace& face, const vm::vec3& delta) const {
            const auto dist = dot(delta, face.boundary().normal);
            if (vm::is_zero(dist, vm::C::almost_zero())) {
                return vm::vec3::zero();
            }

            // the edge rays indicate the direction into which each vertex of the given face moves if the face is dragged
            std::vector<vm::ray3> edgeRays;

            for (const Model::BrushVertex* vertex : face.vertices()) {
                const Model::BrushHalfEdge* firstEdge = vertex->leaving();
                const Model::BrushHalfEdge* curEdge = firstEdge;
                do {
                    vm::ray3 ray(vertex->position(), vm::normalize(curEdge->vector()));

                    // depending on the direction of the drag vector, the rays must be inverted to reflect the
                    // actual movement of the vertices
                    if (dot(delta, ray.direction) < 0.0) {
                        ray.direction = -ray.direction;
                    }

                    edgeRays.push_back(ray);

                    curEdge = curEdge->twin()->next();
                } while (curEdge != firstEdge);
            }

            auto normDelta = face.boundary().normal * dist;
            /**
             * Scalar projection of normDelta onto the nearest axial normal vector.
             */
            const auto normDeltaScalarProj = dot(normDelta, vm::get_abs_max_component_axis(normDelta));

            auto gridSkip = static_cast<size_t>(normDeltaScalarProj / actualSize());
            if (gridSkip > 0) {
                --gridSkip;
            }
            auto actualDist = std::numeric_limits<FloatType>::max();
            auto minDistDelta = std::numeric_limits<FloatType>::max();

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
                    const auto& ray = edgeRays[i];
                    const auto vertexDist = intersectWithRay(ray, gridSkip);
                    const auto vertexDelta = ray.direction * vertexDist;
                    const auto vertexNormDist = dot(vertexDelta, face.boundary().normal);

                    const auto normDistDelta = vm::abs(vertexNormDist - dist);
                    if (normDistDelta < minDistDelta) {
                        actualDist = vertexNormDist;
                        minDistDelta = normDistDelta;
                    }
                }
                ++gridSkip;
            } while (actualDist == std::numeric_limits<FloatType>::max());

            normDelta = face.boundary().normal * actualDist;
            const auto deltaNormalized = normalize(delta);
            return deltaNormalized * dot(normDelta, deltaNormalized);
        }

        vm::vec3 Grid::combineDeltas(const vm::vec3& delta1, const vm::vec3& delta2) const {
            if (vm::squared_length(delta1) < vm::squared_length(delta2)) {
                return delta1;
            } else {
                return delta2;
            }
        }

        vm::vec3 Grid::referencePoint(const vm::bbox3& bounds) const {
            return snap(bounds.center());
        }
    }
}
