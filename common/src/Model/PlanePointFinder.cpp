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

#include "PlanePointFinder.h"

namespace TrenchBroom {
    namespace Model {
        class GridSearchCursor {
        private:
            static const size_t Center = 4;
            
            static const Vec2 MoveOffsets[];
            
            const Plane3& m_plane;
            const FloatType m_frequency;
            
            Vec2 m_position;
            FloatType m_errors[9];
        public:
            GridSearchCursor(const Plane3& plane, const FloatType frequency) :
            m_plane(plane),
            m_frequency(frequency) {
                for (size_t i = 0; i < 9; ++i)
                    m_errors[i] = 0.0;
            }
            
            Vec3 findMinimum(const Vec3& initialPosition) {
                for (size_t i = 0; i < 2; ++i)
                    m_position[i] = Math::round(initialPosition[i]);
                
                findLocalMinimum();
                const Vec2 localMinPos = m_position;
                const FloatType localMinErr = m_errors[Center];
                
                Vec2 globalMinPos = localMinPos;
                FloatType globalMinErr = localMinErr;
                
                if (globalMinErr > 0.0) {
                    // To escape local minima, let's search some adjacent quadrants
                    // The number of extra quadrants should depend on the frequency: The higher the frequency, the
                    // more quadrants should be searched.
                    const size_t numQuadrants = static_cast<size_t>(std::ceil(m_frequency * m_frequency * 3.0));
                    for (size_t i = 0; i < numQuadrants && globalMinErr > 0.0; ++i) {
                        if (i != Center) {
                            m_position = localMinPos + i * 3.0 * MoveOffsets[i];
                            findLocalMinimum();
                            const FloatType newError = m_errors[Center];
                            if (newError < globalMinErr) {
                                globalMinPos = m_position;
                                globalMinErr = newError;
                            }
                        }
                    }
                }
                
                return Vec3(globalMinPos.x(),
                            globalMinPos.y(),
                            Math::round(m_plane.zAt(globalMinPos)));
            }
        private:
            void findLocalMinimum() {
                updateErrors();
                
                size_t smallestError = findSmallestError();
                while (smallestError != Center)
                    smallestError = moveCursor(smallestError);
            }
            
            size_t moveCursor(const size_t direction) {
                m_position += MoveOffsets[direction];
                updateErrors();
                return findSmallestError();
            }
            
            void updateErrors() {
                for (size_t i = 0; i < 9; ++i)
                    m_errors[i] = computeError(i);
            }
            
            FloatType computeError(const size_t location) const {
                const FloatType z = m_plane.zAt(m_position + MoveOffsets[location]);
                return std::abs(z - Math::round(z));
            }
            
            size_t findSmallestError() {
                size_t smallest = Center;
                for (size_t i = 0; i < 9; ++i) {
                    if (m_errors[i] < m_errors[smallest])
                        smallest = i;
                }
                return smallest;
            }
        };

        const Vec2 GridSearchCursor::MoveOffsets[] = {
            Vec2(-1.0,  1.0), Vec2( 0.0,  1.0), Vec2( 1.0,  1.0),
            Vec2(-1.0,  0.0), Vec2( 0.0,  0.0), Vec2( 1.0,  0.0),
            Vec2(-1.0, -1.0), Vec2( 0.0, -1.0), Vec2( 1.0, -1.0)
        };

        FloatType computePlaneFrequency(const Plane3& plane);
        void setDefaultPlanePoints(const Plane3& plane, BrushFace::Points& points);

        FloatType computePlaneFrequency(const Plane3& plane) {
            static const FloatType c = 1.0 - std::sin(Math::C::pi() / 4.0);
            
            const Vec3& axis = plane.normal.firstAxis();
            const FloatType d = plane.normal.dot(axis);
            assert(d != 0.0);
            
            return (1.0 - d) / c;
        }
        
        void setDefaultPlanePoints(const Plane3& plane, BrushFace::Points& points) {
            points[0] = plane.anchor().rounded();
            const Math::Axis::Type axis = plane.normal.firstComponent();
            switch (axis) {
                case Math::Axis::AX:
                    if (plane.normal.x() > 0.0) {
                        points[1] = points[0] + 64.0 * Vec3::PosZ;
                        points[2] = points[0] + 64.0 * Vec3::PosY;
                    } else {
                        points[1] = points[0] + 64.0 * Vec3::PosY;
                        points[2] = points[0] + 64.0 * Vec3::PosZ;
                    }
                    break;
                case Math::Axis::AY:
                    if (plane.normal.y() > 0.0) {
                        points[1] = points[0] + 64.0 * Vec3::PosX;
                        points[2] = points[0] + 64.0 * Vec3::PosZ;
                    } else {
                        points[1] = points[0] + 64.0 * Vec3::PosZ;
                        points[2] = points[0] + 64.0 * Vec3::PosX;
                    }
                    break;
                default:
                    if  (plane.normal.z() > 0.0) {
                        points[1] = points[0] + 64.0 * Vec3::PosY;
                        points[2] = points[0] + 64.0 * Vec3::PosX;
                    } else {
                        points[1] = points[0] + 64.0 * Vec3::PosX;
                        points[2] = points[0] + 64.0 * Vec3::PosY;
                    }
                    break;
            }
        }

        void PlanePointFinder::findPoints(const Plane3& plane, BrushFace::Points& points, const size_t numPoints) {
            using std::swap;
            
            assert(numPoints <= 3);
            
            if (numPoints == 3 && points[0].isInteger() && points[1].isInteger() && points[2].isInteger())
                return;
            
            const FloatType frequency = computePlaneFrequency(plane);
            if (Math::zero(frequency, 1.0 / 7084.0)) {
                setDefaultPlanePoints(plane, points);
                return;
            }
            
            const Math::Axis::Type axis = plane.normal.firstComponent();
            const Plane3 swizzledPlane(plane.distance, swizzle(plane.normal, axis));
            for (size_t i = 0; i < 3; ++i)
                points[i] = swizzle(points[i], axis);
            
            const FloatType waveLength = 1.0 / frequency;
            const FloatType pointDistance = std::min(64.0, waveLength);
            
            FloatType multiplier = 10.0;
            GridSearchCursor cursor(swizzledPlane, frequency);
            if (numPoints == 0)
                points[0] = cursor.findMinimum(swizzledPlane.anchor());
            else if (!points[0].isInteger())
                points[0] = cursor.findMinimum(points[0]);
            
            Vec3 v1, v2;
            FloatType cos;
            size_t count = 0;
            do {
                if (numPoints < 2 || !points[1].isInteger())
                    points[1] = cursor.findMinimum(points[0] + 0.33 * multiplier * pointDistance * Vec3::PosX);
                points[2] = cursor.findMinimum(points[0] + multiplier * (pointDistance * Vec3::PosY - pointDistance / 2.0 * Vec3::PosX));
                v1 = points[2] - points[0];
                v2 = points[1] - points[0];
                v1.normalize();
                v2.normalize();
                cos = v1.dot(v2);
                multiplier *= 1.5f;
                ++count;
            } while (Math::isnan(cos) || std::abs(cos) > 0.9);
            
            cross(v1, v2);
            if ((v1.z() > 0.0) != (swizzledPlane.normal.z() > 0.0))
                swap(points[0], points[2]);
            
            for (size_t i = 0; i < 3; ++i)
                points[i] = unswizzle(points[i], axis);
        }
    }
}
