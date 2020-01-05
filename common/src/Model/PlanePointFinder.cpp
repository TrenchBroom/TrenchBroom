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

#include "PlanePointFinder.h"

#include "FloatType.h"
#include "Model/BrushFace.h"

#include <vecmath/plane.h>
#include <vecmath/scalar.h>
#include <vecmath/vec.h>

namespace TrenchBroom {
    namespace Model {
        class GridSearchCursor {
        private:
            static const size_t Center = 4;

            static const vm::vec2 MoveOffsets[];

            const vm::plane3& m_plane;
            const FloatType m_frequency;

            vm::vec2 m_position;
            FloatType m_errors[9];
        public:
            GridSearchCursor(const vm::plane3& plane, const FloatType frequency) :
            m_plane(plane),
            m_frequency(frequency) {
                for (size_t i = 0; i < 9; ++i) {
                    m_errors[i] = 0.0;
                }
            }

            vm::vec3 findMinimum(const vm::vec3& initialPosition) {
                for (size_t i = 0; i < 2; ++i) {
                    m_position[i] = vm::round(initialPosition[i]);
                }

                findLocalMinimum();
                const auto localMinPos = m_position;
                const auto localMinErr = m_errors[Center];

                auto globalMinPos = localMinPos;
                auto globalMinErr = localMinErr;

                if (globalMinErr > 0.0) {
                    // To escape local minima, let's search some adjacent quadrants
                    // The number of extra quadrants should depend on the frequency: The higher the frequency, the
                    // more quadrants should be searched.
                    const auto numQuadrants = static_cast<size_t>(std::ceil(m_frequency * m_frequency * 3.0));
                    for (size_t i = 0; i < numQuadrants && globalMinErr > 0.0; ++i) {
                        if (i != Center) {
                            m_position = localMinPos + static_cast<FloatType>(i) * 3.0 * MoveOffsets[i];
                            findLocalMinimum();
                            const auto newError = m_errors[Center];
                            if (newError < globalMinErr) {
                                globalMinPos = m_position;
                                globalMinErr = newError;
                            }
                        }
                    }
                }

                return vm::vec3(globalMinPos.x(), globalMinPos.y(), vm::round(m_plane.zAt(globalMinPos)));
            }
        private:
            void findLocalMinimum() {
                updateErrors();

                size_t smallestError = findSmallestError();
                while (smallestError != Center) {
                    smallestError = moveCursor(smallestError);
                }
            }

            size_t moveCursor(const size_t direction) {
                m_position = m_position + MoveOffsets[direction];
                updateErrors();
                return findSmallestError();
            }

            void updateErrors() {
                for (size_t i = 0; i < 9; ++i) {
                    m_errors[i] = computeError(i);
                }
            }

            FloatType computeError(const size_t location) const {
                const auto z = m_plane.zAt(m_position + MoveOffsets[location]);
                return std::abs(z - vm::round(z));
            }

            size_t findSmallestError() {
                auto smallest = Center;
                for (size_t i = 0; i < 9; ++i) {
                    if (m_errors[i] < m_errors[smallest]) {
                        smallest = i;
                    }
                }
                return smallest;
            }
        };

        const vm::vec2 GridSearchCursor::MoveOffsets[] = {
            vm::vec2(-1.0,  1.0), vm::vec2( 0.0,  1.0), vm::vec2( 1.0,  1.0),
            vm::vec2(-1.0,  0.0), vm::vec2( 0.0,  0.0), vm::vec2( 1.0,  0.0),
            vm::vec2(-1.0, -1.0), vm::vec2( 0.0, -1.0), vm::vec2( 1.0, -1.0)
        };

        FloatType computePlaneFrequency(const vm::plane3& plane);
        void setDefaultPlanePoints(const vm::plane3& plane, BrushFace::Points& points);

        FloatType computePlaneFrequency(const vm::plane3& plane) {
            static const auto c = FloatType(1.0) - std::sin(vm::C::pi() / FloatType(4.0));

            const auto axis = vm::get_abs_max_component_axis(plane.normal);
            const auto cos = dot(plane.normal, axis);
            assert(cos != FloatType(0.0));

            return (FloatType(1.0) - cos) / c;
        }

        void setDefaultPlanePoints(const vm::plane3& plane, BrushFace::Points& points) {
            points[0] = round(plane.anchor());
            switch (vm::find_abs_max_component(plane.normal)) {
                case vm::axis::x:
                    if (plane.normal.x() > 0.0) {
                        points[1] = points[0] + 64.0 * vm::vec3::pos_z();
                        points[2] = points[0] + 64.0 * vm::vec3::pos_y();
                    } else {
                        points[1] = points[0] + 64.0 * vm::vec3::pos_y();
                        points[2] = points[0] + 64.0 * vm::vec3::pos_z();
                    }
                    break;
                case vm::axis::y:
                    if (plane.normal.y() > 0.0) {
                        points[1] = points[0] + 64.0 * vm::vec3::pos_x();
                        points[2] = points[0] + 64.0 * vm::vec3::pos_z();
                    } else {
                        points[1] = points[0] + 64.0 * vm::vec3::pos_z();
                        points[2] = points[0] + 64.0 * vm::vec3::pos_x();
                    }
                    break;
                default:
                    if  (plane.normal.z() > 0.0) {
                        points[1] = points[0] + 64.0 * vm::vec3::pos_y();
                        points[2] = points[0] + 64.0 * vm::vec3::pos_x();
                    } else {
                        points[1] = points[0] + 64.0 * vm::vec3::pos_x();
                        points[2] = points[0] + 64.0 * vm::vec3::pos_y();
                    }
                    break;
            }
        }

        void PlanePointFinder::findPoints(const vm::plane3& plane, FacePoints& points, const size_t numPoints) {
            using std::swap;

            assert(numPoints <= 3);

            if (numPoints == 3 && vm::is_integral(points[0]) && vm::is_integral(points[1]) && vm::is_integral(points[2])) {
                return;
            }

            const auto frequency = computePlaneFrequency(plane);
            if (vm::is_zero(frequency, 1.0 / 7084.0)) {
                setDefaultPlanePoints(plane, points);
                return;
            }

            const auto axis = vm::find_abs_max_component(plane.normal);
            const auto swizzledPlane = vm::plane3(plane.distance, swizzle(plane.normal, axis));
            for (size_t i = 0; i < 3; ++i) {
                points[i] = swizzle(points[i], axis);
            }

            const auto waveLength = FloatType(1.0) / frequency;
            const auto pointDistance = vm::min(FloatType(64.0), waveLength);

            auto multiplier = FloatType(10.0);
            auto cursor = GridSearchCursor(swizzledPlane, frequency);
            if (numPoints == 0) {
                points[0] = cursor.findMinimum(swizzledPlane.anchor());
            } else if (!vm::is_integral(points[0])) {
                points[0] = cursor.findMinimum(points[0]);
            }

            vm::vec3 v1, v2;
            FloatType cos;
            size_t count = 0;
            do {
                if (numPoints < 2 || !vm::is_integral(points[1])) {
                    points[1] = cursor.findMinimum(points[0] + FloatType(0.33) * multiplier * pointDistance * vm::vec3::pos_x());
                }
                points[2] = cursor.findMinimum(points[0] + multiplier * (pointDistance * vm::vec3::pos_y() - pointDistance / FloatType(2.0) * vm::vec3::pos_x()));
                v1 = normalize(points[2] - points[0]);
                v2 = normalize(points[1] - points[0]);
                cos = dot(v1, v2);
                multiplier *= FloatType(1.5);
                ++count;
            } while (vm::is_nan(cos) || std::abs(cos) > FloatType(0.9));

            v1 = cross(v1, v2);
            if ((v1.z() > 0.0) != (swizzledPlane.normal.z() > FloatType(0.0))) {
                swap(points[0], points[2]);
            }

            for (size_t i = 0; i < 3; ++i) {
                points[i] = unswizzle(points[i], axis);
            }
        }
    }
}
