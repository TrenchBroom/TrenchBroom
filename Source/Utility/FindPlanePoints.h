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

#ifndef TrenchBroom_FindPlanePoints_h
#define TrenchBroom_FindPlanePoints_h

#include "Utility/CoordinatePlane.h"
#include "Utility/VecMath.h"

#include <algorithm>
#include <limits>

namespace TrenchBroom {
    namespace Math {
        typedef Vec3f PlanePoints[3];

        class SearchCursor {
        private:
            typedef unsigned int Type;
            static const Type Center        = 0;
            static const Type TopLeft       = 1;
            static const Type Top           = 2;
            static const Type TopRight      = 3;
            static const Type Left          = 4;
            static const Type Right         = 5;
            static const Type BottomLeft    = 6;
            static const Type Bottom        = 7;
            static const Type BottomRight   = 8;

            static const Vec2f MoveOffsets[9];

            const Plane& m_plane;
            const float m_frequency;

            Vec2f m_position;
            float m_errors[9];

            inline float error(const Vec2f& point) {
                const float z = m_plane.z(point.x, point.y);
                return std::abs(z - Math::round(z));
            }

            inline void updateError(const Type point) {
                m_errors[point] = error(m_position + MoveOffsets[point]);
            }

            inline Type move(const Type direction) {
                m_position += MoveOffsets[direction];
                Type bestPoint = Center;
                switch (direction) {
                    case Top:
                        m_errors[BottomLeft] = m_errors[Left];
                        m_errors[Bottom] = m_errors[Center];
                        m_errors[BottomRight] = m_errors[Right];
                        m_errors[Left] = m_errors[TopLeft];
                        m_errors[Center] = m_errors[Top];
                        m_errors[Right] = m_errors[TopRight];
                        updateError(TopLeft);
                        updateError(Top);
                        updateError(TopRight);

                        if (m_errors[TopLeft] < m_errors[bestPoint])
                            bestPoint = TopLeft;
                        if (m_errors[Top] < m_errors[bestPoint])
                            bestPoint = Top;
                        if (m_errors[TopRight] < m_errors[bestPoint])
                            bestPoint = TopRight;
                        break;
                    case Right:
                        m_errors[TopLeft] = m_errors[Top];
                        m_errors[Left] = m_errors[Center];
                        m_errors[BottomLeft] = m_errors[Bottom];
                        m_errors[Top] = m_errors[TopRight];
                        m_errors[Center] = m_errors[Right];
                        m_errors[Bottom] = m_errors[BottomRight];
                        updateError(TopRight);
                        updateError(Right);
                        updateError(BottomRight);

                        if (m_errors[TopRight] < m_errors[bestPoint])
                            bestPoint = TopRight;
                        if (m_errors[Right] < m_errors[bestPoint])
                            bestPoint = Right;
                        if (m_errors[BottomRight] < m_errors[bestPoint])
                            bestPoint = BottomRight;
                        break;
                    case Bottom:
                        m_errors[TopLeft] = m_errors[Left];
                        m_errors[Top] = m_errors[Center];
                        m_errors[TopRight] = m_errors[Right];
                        m_errors[Left] = m_errors[BottomLeft];
                        m_errors[Center] = m_errors[Bottom];
                        m_errors[Right] = m_errors[BottomRight];
                        updateError(BottomLeft);
                        updateError(Bottom);
                        updateError(BottomRight);

                        if (m_errors[BottomLeft] < m_errors[bestPoint])
                            bestPoint = BottomLeft;
                        if (m_errors[Bottom] < m_errors[bestPoint])
                            bestPoint = Bottom;
                        if (m_errors[BottomRight] < m_errors[bestPoint])
                            bestPoint = BottomRight;
                        break;
                    case Left:
                        m_errors[TopRight] = m_errors[Top];
                        m_errors[Right] = m_errors[Center];
                        m_errors[BottomRight] = m_errors[Bottom];
                        m_errors[Top] = m_errors[TopLeft];
                        m_errors[Center] = m_errors[Left];
                        m_errors[Bottom] = m_errors[BottomLeft];
                        updateError(TopLeft);
                        updateError(Left);
                        updateError(BottomLeft);

                        if (m_errors[TopLeft] < m_errors[bestPoint])
                            bestPoint = TopLeft;
                        if (m_errors[Left] < m_errors[bestPoint])
                            bestPoint = Left;
                        if (m_errors[BottomLeft] < m_errors[bestPoint])
                            bestPoint = BottomLeft;
                        break;
                    case TopLeft:
                        m_errors[BottomRight] = m_errors[Center];
                        m_errors[Bottom] = m_errors[Left];
                        m_errors[Right] = m_errors[Top];
                        m_errors[Center] = m_errors[TopLeft];
                        updateError(BottomLeft);
                        updateError(Left);
                        updateError(TopLeft);
                        updateError(Top);
                        updateError(TopRight);

                        if (m_errors[BottomLeft] < m_errors[bestPoint])
                            bestPoint = BottomLeft;
                        if (m_errors[Left] < m_errors[bestPoint])
                            bestPoint = Left;
                        if (m_errors[TopLeft] < m_errors[bestPoint])
                            bestPoint = TopLeft;
                        if (m_errors[Top] < m_errors[bestPoint])
                            bestPoint = Top;
                        if (m_errors[TopRight] < m_errors[bestPoint])
                            bestPoint = TopRight;
                        break;
                    case TopRight:
                        m_errors[BottomLeft] = m_errors[Center];
                        m_errors[Left] = m_errors[Top];
                        m_errors[Bottom] = m_errors[Right];
                        m_errors[Center] = m_errors[TopRight];
                        updateError(TopLeft);
                        updateError(Top);
                        updateError(TopRight);
                        updateError(Right);
                        updateError(BottomRight);

                        if (m_errors[TopLeft] < m_errors[bestPoint])
                            bestPoint = TopLeft;
                        if (m_errors[Top] < m_errors[bestPoint])
                            bestPoint = Top;
                        if (m_errors[TopRight] < m_errors[bestPoint])
                            bestPoint = TopRight;
                        if (m_errors[Right] < m_errors[bestPoint])
                            bestPoint = Right;
                        if (m_errors[BottomRight] < m_errors[bestPoint])
                            bestPoint = BottomRight;
                        break;
                    case BottomRight:
                        m_errors[TopLeft] = m_errors[Center];
                        m_errors[Left] = m_errors[Bottom];
                        m_errors[Top] = m_errors[Right];
                        m_errors[Center] = m_errors[BottomRight];
                        updateError(TopRight);
                        updateError(Right);
                        updateError(BottomRight);
                        updateError(Bottom);
                        updateError(BottomLeft);

                        if (m_errors[TopRight] < m_errors[bestPoint])
                            bestPoint = TopRight;
                        if (m_errors[Right] < m_errors[bestPoint])
                            bestPoint = Right;
                        if (m_errors[BottomRight] < m_errors[bestPoint])
                            bestPoint = BottomRight;
                        if (m_errors[Bottom] < m_errors[bestPoint])
                            bestPoint = Bottom;
                        if (m_errors[BottomLeft] < m_errors[bestPoint])
                            bestPoint = BottomLeft;
                        break;
                    case BottomLeft:
                        m_errors[TopRight] = m_errors[Center];
                        m_errors[Top] = m_errors[Left];
                        m_errors[Right] = m_errors[Bottom];
                        m_errors[Center] = m_errors[BottomLeft];
                        updateError(BottomRight);
                        updateError(Bottom);
                        updateError(BottomLeft);
                        updateError(Left);
                        updateError(TopLeft);

                        if (m_errors[BottomRight] < m_errors[bestPoint])
                            bestPoint = BottomRight;
                        if (m_errors[Bottom] < m_errors[bestPoint])
                            bestPoint = Bottom;
                        if (m_errors[BottomLeft] < m_errors[bestPoint])
                            bestPoint = BottomLeft;
                        if (m_errors[Left] < m_errors[bestPoint])
                            bestPoint = Left;
                        if (m_errors[TopLeft] < m_errors[bestPoint])
                            bestPoint = TopLeft;
                        break;
                }

                return bestPoint;
            }

            inline const void findLocalMinimum() {
                for (Type i = 0; i < 9; i++)
                    updateError(i);

                // find the initial best point
                Type bestPoint = Center;
                for (Type i = 0; i < 9; i++)
                    if (m_errors[i] < m_errors[bestPoint])
                        bestPoint = i;

                while (bestPoint != Center)
                    bestPoint = move(bestPoint);
            }

            inline const Vec3f doFindMinimum() {
                findLocalMinimum();
                const Vec2f localMinimumPosition = m_position;
                const float localMinimumError = m_errors[Center];

                Vec2f globalMinimumPosition = localMinimumPosition;
                float globalMinimumError = localMinimumError;

                if (globalMinimumError > 0.0f) {
                    // To escape local minima, let's search some adjacent quadrants
                    // The number of extra quadrants should depend on the frequency: The higher the frequency, the
                    // more quadrants should be searched.
                    unsigned int numQuadrants = static_cast<unsigned int>(std::ceil(m_frequency * m_frequency * 3.0f));
                    for (unsigned int i = 1; i < numQuadrants && globalMinimumError > 0.0f; i++) {
                        for (Type q = 1; q < 9 && globalMinimumError > 0.0f; q++) {
                            m_position = localMinimumPosition + i * 3.0f * MoveOffsets[q];
                            findLocalMinimum();
                            const float newError = m_errors[Center];
                            if (newError < globalMinimumError) {
                                globalMinimumPosition = m_position;
                                globalMinimumError = newError;
                            }
                        }
                    }
                }

                return Vec3f(globalMinimumPosition.x,
                             globalMinimumPosition.y,
                             Math::round(m_plane.z(globalMinimumPosition.x,
                                                   globalMinimumPosition.y)));
            }
        public:
            SearchCursor(const Plane& plane, const float frequency) :
            m_plane(plane),
            m_frequency(frequency) {}

            inline const Vec3f findMinimum(const Vec3f& initialPosition) {
                m_position.x = Math::round(initialPosition.x);
                m_position.y = Math::round(initialPosition.y);
                return doFindMinimum();
            }
        };

        class FindPlanePoints {
        protected:
            virtual void doFindPlanePoints(const Plane& plane, PlanePoints& points, size_t numPoints) const = 0;
        public:
            inline void operator()(const Plane& plane, PlanePoints& points, size_t numPoints = 0) const {
                assert(numPoints <= 3);
                doFindPlanePoints(plane, points, numPoints);
            }

            virtual ~FindPlanePoints() {}
        };

        class FindFloatPlanePoints : public FindPlanePoints {
        protected:
            inline void doFindPlanePoints(const Plane& plane, PlanePoints& points, size_t numPoints) const {
                if (numPoints == 0) {
                    points[0] = plane.anchor();
                    numPoints++;
                }
                if (numPoints == 1) {
                    const Vec3f dir = plane.normal.thirdAxis();
                    points[1] = plane.project(dir * 128.0f);
                    numPoints++;
                }
                if (numPoints == 2) {
                    const Vec3f dir = (points[1] - points[0]).crossed(plane.normal);
                    points[3] = dir * 128.0f * 128.0f / dir.lengthSquared();
                }
            }
        };

        /**
         * \brief This algorithm will find three integer points that describe a given plane as closely as possible.
         */
        class FindIntegerPlanePoints : public FindPlanePoints {
        private:
            inline float planeFrequency(const Plane& plane) const {
                static const float c = 1.0f - std::sin(Math::Pi / 4.0f);

                const Vec3f& axis = plane.normal.firstAxis();
                const float d = plane.normal.dot(axis);
                assert(d != 0.0f);

                return (1.0f - d) / c;
            }

            inline void setDefaultPlanePoints(const Plane& plane, PlanePoints& points) const {
                points[0] = plane.anchor().rounded();
                const Axis::Type axis = plane.normal.firstComponent();
                switch (axis) {
                    case Axis::AX:
                        if (plane.normal.x > 0.0f) {
                            points[1] = points[0] + 64.0f * Vec3f::PosZ;
                            points[2] = points[0] + 64.0f * Vec3f::PosY;
                        } else {
                            points[1] = points[0] + 64.0f * Vec3f::PosY;
                            points[2] = points[0] + 64.0f * Vec3f::PosZ;
                        }
                        break;
                    case Axis::AY:
                        if (plane.normal.y > 0.0f) {
                            points[1] = points[0] + 64.0f * Vec3f::PosX;
                            points[2] = points[0] + 64.0f * Vec3f::PosZ;
                        } else {
                            points[1] = points[0] + 64.0f * Vec3f::PosZ;
                            points[2] = points[0] + 64.0f * Vec3f::PosX;
                        }
                        break;
                    default:
                        if  (plane.normal.z > 0.0f) {
                            points[1] = points[0] + 64.0f * Vec3f::PosY;
                            points[2] = points[0] + 64.0f * Vec3f::PosX;
                        } else {
                            points[1] = points[0] + 64.0f * Vec3f::PosX;
                            points[2] = points[0] + 64.0f * Vec3f::PosY;
                        }
                        break;
                }
            }
        protected:
            inline void doFindPlanePoints(const Plane& plane, PlanePoints& points, size_t numPoints) const {
                if (numPoints == 3 && points[0].isInteger() && points[1].isInteger() && points[2].isInteger())
                    return;

                const float frequency = planeFrequency(plane);
                if (Math::zero(frequency, 1.0f / 7084.0f)) {
                    setDefaultPlanePoints(plane, points);
                } else {
                    const CoordinatePlane& coordPlane = CoordinatePlane::plane(plane.normal);
                    const Plane swizzledPlane(coordPlane.swizzle(plane.normal), plane.distance);
                    const float waveLength = 1.0f / frequency;
                    const float pointDistance = std::max(64.0f, waveLength);

                    float multiplier = 10.0f;
                    SearchCursor cursor(swizzledPlane, frequency);

                    if (numPoints == 0)
                        points[0] = cursor.findMinimum(swizzledPlane.anchor());
                    else if (!points[0].isInteger())
                        points[0] = cursor.findMinimum(points[0]);

                    Vec3f v1, v2;
                    float cos;
                    size_t count = 0;
                    do {
                        if (numPoints < 2 || !points[1].isInteger())
                            points[1] = cursor.findMinimum(points[0] + 0.33f * multiplier * pointDistance * Vec3f::PosX);
                        points[2] = cursor.findMinimum(points[0] + multiplier * (pointDistance * Vec3f::PosY - 0.5f * pointDistance * Vec3f::PosX));
                        v1 = points[2] - points[0];
                        v2 = points[1] - points[0];
                        cos = v1.normalized().dot(v2.normalized());
                        multiplier *= 1.5f;
                        count++;
                    } while (Math::isnan(cos) || std::abs(cos) > 0.9f);

                    v1.cross(v2);
                    if ((v1.z > 0.0f) != (swizzledPlane.normal.z > 0.0f))
                        std::swap(points[0], points[2]);

                    for (unsigned int i = 0; i < 3; i++)
                        points[i] = coordPlane.unswizzle(points[i]);
                }
            }
        };
    }
}

#endif
