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

#ifndef TRENCHBROOM_CONVEX_HULL_H
#define TRENCHBROOM_CONVEX_HULL_H

#include "vec_decl.h"
#include "vec_impl.h"

#include <algorithm>
#include <iterator>

namespace vm {
    /**
     * Helper struct for computing a convex hull.
     *
     * @tparam T the component type
     */
    template <typename T>
    class ConvexHull2D {
    private:
        static int isLeft(const vec<T,3>& p1, const vec<T,3>& p2, const vec<T,3>& p3) {
            const T result = ( (p2.x() - p1.x()) * (p3.y() - p1.y())
                               -(p3.x() - p1.x()) * (p2.y() - p1.y()));
            if (result < 0.0) {
                return -1;
            } else if (result > 0.0) {
                return 1;
            } else {
                return 0;
            }
        }
    private:
        class LessThanByAngle {
        private:
            const vec<T,3>& m_anchor;
        public:
            LessThanByAngle(const vec<T,3>& anchor) : m_anchor(anchor) {}
        public:
            bool operator()(const vec<T,3>& lhs, const vec<T,3>& rhs) const {
                const auto side = isLeft(m_anchor, lhs, rhs);
                if (side > 0) {
                    return true;
                } else if (side < 0) {
                    return false;
                } else {
                    // the points are colinear, the one that is further from the anchor is considered less
                    const auto dxl = abs(lhs.x() - m_anchor.x());
                    const auto dxr = abs(rhs.x() - m_anchor.x());
                    if (dxl == dxr) {
                        const auto dyl = abs(lhs.y() - m_anchor.y());
                        const auto dyr = abs(rhs.y() - m_anchor.y());
                        return dyl > dyr;
                    }
                    return dxl > dxr;
                }
            }
        };

        typename vec<T,3>::List m_points;
        bool m_hasResult;
    public:
        ConvexHull2D(const typename vec<T,3>::List& points) :
        m_points(points),
        m_hasResult(m_points.size() > 2) {
            if (m_hasResult) {
                const auto thirdPointIndex = findLinearlyIndependentPoint();
                m_hasResult = (thirdPointIndex < m_points.size());

                if (m_hasResult) {
                    const auto axis = computeAxis(thirdPointIndex);
                    swizzle(axis);

                    findAnchor();
                    sortPoints();
                    m_hasResult = (m_points.size() > 2);

                    if (m_hasResult) {
                        buildHull();
                        unswizzle(axis);
                    }
                }
            }
        }

        bool hasResult() const {
            return m_hasResult;
        }

        const typename vec<T,3>::List& result() const {
            assert(m_hasResult);
            return m_points;
        }
    private:
        size_t findLinearlyIndependentPoint() const {
            size_t index = 2;
            while (index < m_points.size() && colinear(m_points[0], m_points[1], m_points[index])) {
                ++index;
            }
            return index;
        }

        axis::type computeAxis(const size_t thirdPointIndex) const {
            const auto axis = cross(m_points[thirdPointIndex] - m_points[0], m_points[1] - m_points[0]);
            return firstComponent(axis);
        }

        void swizzle(const axis::type axis) {
            auto points = typename vec<T,3>::List();
            points.reserve(m_points.size());

            std::transform(
                    std::begin(m_points), std::end(m_points),
                    std::back_inserter(points),
                    [&](const auto& p) { return vm::swizzle(p, axis); });
            using std::swap; swap(m_points, points);
        }

        void unswizzle(const axis::type axis) {
            auto points = typename vec<T,3>::List();
            points.reserve(m_points.size());

            std::transform(
                    std::begin(m_points), std::end(m_points),
                    std::back_inserter(points),
                    [&](const auto& p) { return vm::unswizzle(p, axis); });
            using std::swap; swap(m_points, points);
        }

        void findAnchor() {
            size_t anchor = 0;
            for (size_t i = 1; i < m_points.size(); ++i) {
                if ((m_points[i].y() < m_points[anchor].y()) ||
                    (m_points[i].y() == m_points[anchor].y() &&
                     m_points[i].x() >  m_points[anchor].x())) {
                    anchor = i;
                }
            }

            if (anchor > 0) {
                using std::swap;
                swap(m_points[0], m_points[anchor]);
            }
        }

        void sortPoints() {
            const auto& anchor = m_points[0];
            std::sort(std::next(std::begin(m_points)), std::end(m_points), LessThanByAngle(anchor));

            // now remove the duplicates
            auto i = std::begin(m_points) + 1;
            while (i != std::end(m_points)) {
                const auto& p1 = *(i++);
                while (i != std::end(m_points)) {
                    const auto& p2 = *i;
                    if (isLeft(anchor, p1, p2) == 0) {
                        i = m_points.erase(i);
                    } else {
                        break;
                    }
                };
            }
        }

        void buildHull() {
            auto stack = typename vec<T,3>::List();
            stack.reserve(m_points.size());
            stack.push_back(m_points[0]);
            stack.push_back(m_points[1]);

            for (size_t i = 2; i < m_points.size(); ++i) {
                const auto& p = m_points[i];
                popStalePoints(stack, p);
                stack.push_back(p);
            }

            using std::swap; swap(m_points, stack);
            assert(m_points.size() > 2);
        }

        void popStalePoints(typename vec<T,3>::List& stack, const vec<T,3>& p) {
            if (stack.size() > 1) {
                const auto& t1 = stack[stack.size() - 2];
                const auto& t2 = stack[stack.size() - 1];
                const auto side = isLeft(t1, t2, p);
                if (side < 0) {
                    stack.pop_back();
                    popStalePoints(stack, p);
                }
            }
        }
    };

    /**
     * Computes the convex hull of the given points. Returns the list of vertices of the polygon which is formed
     * by the convex hull. Note that if the given points are all colinear, or less than 3 points are given, then
     * no convex hull exists and the function returns an empty list.
     *
     * @tparam T the component type
     * @param points the points
     * @return the convex hull of the points, or an empty list if no convex hull exists
     */
    template <typename T>
    typename vec<T,3>::List convexHull2D(const typename vec<T,3>::List& points) {
        // see http://geomalgorithms.com/a10-_hull-1.html
        const ConvexHull2D<T> hull(points);
        if (!hull.hasResult()) {
            return vec<T,3>::EmptyList;
        } else {
            return hull.result();
        }
    }
}

#endif
