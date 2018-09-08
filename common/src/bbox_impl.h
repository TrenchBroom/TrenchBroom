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

#ifndef TRENCHBROOM_BBOX_IMPL_H
#define TRENCHBROOM_BBOX_IMPL_H

#include "bbox_decl.h"

#include "vec_decl.h"
#include "vec_impl.h"
#include "mat_decl.h"
#include "mat_impl.h"
#include "quat_decl.h"
#include "quat_impl.h"
#include "MathUtils.h"

#include <algorithm>

namespace vm {
    /**
     * Creates a new bounding box at the origin with size 0.
     */
    template <typename T, size_t S>
    bbox<T,S>::bbox() :
    min(vec<T,S>::zero),
    max(vec<T,S>::zero) {}

    template <typename T, size_t S>
    bbox<T,S>::bbox(const vec<T,S>& i_min, const vec<T,S>& i_max) :
    min(i_min),
    max(i_max) {
        assert(valid());
    }

    template <typename T, size_t S>
    bbox<T,S>::bbox(const T i_minMax) :
    min(vec<T,S>::fill(-i_minMax)),
    max(vec<T,S>::fill(+i_minMax)) {
        assert(valid());
    }

    template <typename T, size_t S>
    bbox<T,S>::bbox(const T i_min, const T i_max) :
    min(vec<T,S>::fill(i_min)),
    max(vec<T,S>::fill(i_max)) {
        assert(valid());
    }

    template <typename T, size_t S>
    bool bbox<T,S>::valid(const vec<T,S>& min, const vec<T,S>& max) {
        for (size_t i = 0; i < S; ++i) {
            if (min[i] > max[i]) {
                return false;
            }
        }
        return true;
    }

    template <typename T, size_t S>
    bool bbox<T,S>::valid() const {
        return valid(min, max);
    }

    template <typename T, size_t S>
    bool bbox<T,S>::empty() const {
        assert(valid());
        for (size_t i = 0; i < S; ++i) {
            if (min[i] >= max[i]) {
                return true;
            }
        }
        return false;
    }

    template <typename T, size_t S>
    vec<T,S> bbox<T,S>::center() const {
        assert(valid());
        return (min + max) / static_cast<T>(2.0);
    }

    template <typename T, size_t S>
    vec<T,S> bbox<T,S>::size() const {
        assert(valid());
        return max - min;
    }

    template <typename T, size_t S>
    T bbox<T,S>::volume() const {
        assert(valid());
        const auto boxSize = size();
        T result = boxSize[0];
        for (size_t i = 1; i < S; ++i) {
            result *= boxSize[i];
        }
        return result;
    }

    template <typename T, size_t S>
    bool bbox<T,S>::contains(const vec<T,S>& point, const T epsilon) const {
        assert(valid());
        for (size_t i = 0; i < S; ++i) {
            if (Math::lt(point[i], min[i], epsilon) ||
                Math::gt(point[i], max[i], epsilon)) {
                return false;
            }
        }
        return true;
    }

    template <typename T, size_t S>
    bool bbox<T,S>::contains(const bbox<T,S>& b, const T epsilon) const {
        assert(valid());
        for (size_t i = 0; i < S; ++i) {
            if (Math::lt(b.min[i], min[i], epsilon) ||
                Math::gt(b.max[i], max[i], epsilon)) {
                return false;
            }
        }
        return true;
    }

    template <typename T, size_t S>
    bool bbox<T,S>::encloses(const bbox<T,S>& b, const T epsilon) const {
        assert(valid());
        for (size_t i = 0; i < S; ++i) {
            if (Math::lte(b.min[i], min[i], epsilon) ||
                Math::gte(b.max[i], max[i], epsilon)) {
                return false;
            }
        }
        return true;
    }

    template <typename T, size_t S>
    bool bbox<T,S>::intersects(const bbox<T,S>& b, const T epsilon) const {
        for (size_t i = 0; i < S; ++i) {
            if (Math::lt(b.max[i], min[i], epsilon) ||
                Math::gt(b.min[i], max[i], epsilon)) {
                return false;
            }
        }
        return true;
    }

    template <typename T, size_t S>
    vec<T,S> bbox<T,S>::constrain(const vec<T,S>& point) const {
        assert(valid());
        return vm::max(min, vm::min(max, point));
    }

    enum class Corner { min, max };

    template <typename T, size_t S>
    vec<T,S> bbox<T,S>::corner(const Corner c[S]) const {
        assert(valid());
        vec<T,S> result;
        for (size_t i = 0; i < S; ++i) {
            result[i] = c[i] == Corner::min ? min[i] : max[i];
        }
        return result;
    }

    template <typename T, size_t S>
    vec<T,3> bbox<T,S>::corner(const Corner x, const Corner y, const Corner z) const {
        Corner c[] = { x, y, z };
        return corner(c);
    }

    template <typename T, size_t S>
    std::array<typename bbox<T,S>::Range, S> bbox<T,S>::relativePosition(const vec<T,S>& point) const {
        assert(valid());

        std::array<Range, S> result;
        for (size_t i = 0; i < S; ++i) {
            if (point[i] < min[i]) {
                result[i] = Range::less;
            } else if (point[i] > max[i]) {
                result[i] = Range::greater;
            } else {
                result[i] = Range::within;
            }
        }

        return result;
    }

    template <typename T, size_t S>
    bbox<T,S> bbox<T,S>::expand(const T f) const {
        assert(valid());
        return bbox<T,S>(min - vec<T,S>::fill(f), max + vec<T,S>::fill(f));
    }

    template <typename T, size_t S>
    bbox<T,S> bbox<T,S>::translate(const vec<T,S>& delta) const {
        assert(valid());
        return bbox<T,S>(min + delta, max + delta);
    }

    template <typename T, size_t S>
    bbox<T,S> bbox<T,S>::transform(const mat<T,S+1,S+1>& transform) const {
        const auto vertices = this->vertices();
        const auto first = vertices[0] * transform;
        auto result = bbox<T,3>(first, first);
        for (size_t i = 1; i < vertices.size(); ++i) {
            result = merge(result, vertices[i] * transform);
        }
        return result;
    }

    template <typename T, size_t S>
    typename vec<T,S>::List bbox<T,S>::vertices() const {
        typename vec<T,S>::List result;
        result.reserve(8);
        forEachVertex([&](const vec<T,S>& v){ result.push_back(v); });
        return result;
    }

    template <typename T, size_t S>
    std::ostream& operator<<(std::ostream& stream, const bbox<T,S>& bbox) {
        stream << "{ min: (" << bbox.min << "), max: (" << bbox.max << ") }";
        return stream;
    }

    template <typename T, size_t S>
    bool operator==(const bbox<T,S>& lhs, const bbox<T,S>& rhs) {
        return lhs.min == rhs.min && lhs.max == rhs.max;
    }

    template <typename T, size_t S>
    bool operator!=(const bbox<T,S>& lhs, const bbox<T,S>& rhs) {
        return lhs.min != rhs.min || lhs.max != rhs.max;
    }

    template <typename T, size_t S>
    bbox<T,S> merge(const bbox<T,S>& lhs, const bbox<T,S>& rhs) {
        return bbox<T,S>(min(lhs.min, rhs.min), max(lhs.max, rhs.max));
    }

    template <typename T, size_t S>
    bbox<T,S> merge(const bbox<T,S>& lhs, const vec<T,S>& rhs) {
        return bbox<T,S>(min(lhs.min, rhs), max(lhs.max, rhs));
    }

    template <typename T, size_t S>
    bbox<T,S> intersect(const bbox<T,S>& lhs, const bbox<T,S>& rhs) {
        const auto min = vm::max(lhs.min, rhs.min);
        const auto max = vm::min(lhs.max, rhs.max);
        if (bbox<T,S>::valid(min, max)) {
            return bbox<T,S>(min, max);
        } else {
            return bbox<T,S>(vec<T,S>::zero, vec<T,S>::zero);
        }
    }

    template <typename T>
    mat<T,4,4> scaleBBoxMatrix(const bbox<T,3>& oldBBox, const bbox<T,3>& newBBox) {
        const auto scaleFactors = newBBox.size() / oldBBox.size();
        return translationMatrix(newBBox.min) * scalingMatrix(scaleFactors) * translationMatrix(-oldBBox.min);
    }

    template <typename T>
    mat<T,4,4> scaleBBoxMatrixWithAnchor(const bbox<T,3>& oldBBox, const vec<T,3>& newSize, const vec<T,3>& anchorPoint) {
        const auto scaleFactors = newSize / oldBBox.size();
        return translationMatrix(anchorPoint) * scalingMatrix(scaleFactors) * translationMatrix(-anchorPoint);
    }

    template <typename T>
    mat<T,4,4> shearBBoxMatrix(const bbox<T,3>& box, const vec<T,3>& sideToShear, const vec<T,3>& delta) {
        const auto oldSize = box.size();

        // shearMatrix(const T Sxy, const T Sxz, const T Syx, const T Syz, const T Szx, const T Szy) {
        mat<T,4,4> shearMat;
        if (sideToShear == vec<T,3>::pos_x) {
            const auto relativeDelta = delta / oldSize.x();
            shearMat = shearMatrix(relativeDelta.y(), relativeDelta.z(), 0., 0., 0., 0.);
        } else if (sideToShear == vec<T,3>::neg_x) {
            const auto relativeDelta = delta / oldSize.x();
            shearMat = shearMatrix(-relativeDelta.y(), -relativeDelta.z(), 0., 0., 0., 0.);
        } else if (sideToShear == vec<T,3>::pos_y) {
            const auto relativeDelta = delta / oldSize.y();
            shearMat = shearMatrix(0., 0., relativeDelta.x(), relativeDelta.z(), 0., 0.);
        } else if (sideToShear == vec<T,3>::neg_y) {
            const auto relativeDelta = delta / oldSize.y();
            shearMat = shearMatrix(0., 0., -relativeDelta.x(), -relativeDelta.z(), 0., 0.);
        } else if (sideToShear == vec<T,3>::pos_z) {
            const auto relativeDelta = delta / oldSize.z();
            shearMat = shearMatrix(0., 0., 0., 0., relativeDelta.x(), relativeDelta.y());
        } else if (sideToShear == vec<T,3>::neg_z) {
            const auto relativeDelta = delta / oldSize.z();
            shearMat = shearMatrix(0., 0., 0., 0., -relativeDelta.x(), -relativeDelta.y());
        }

        // grab any vertex on side that is opposite the one being sheared.
        const auto sideOppositeToShearSide = -sideToShear;
        vec<T,3> vertOnOppositeSide;
        bool didGrab = false;
        auto visitor = [&](const vec<T,3>& p0, const vec<T,3>& p1, const vec<T,3>& p2, const vec<T,3>& p3, const vec<T,3>& n){
            if (n == sideOppositeToShearSide) {
                vertOnOppositeSide = p0;
                didGrab = true;
            }
        };
        box.forEachFace(visitor);
        assert(didGrab);

        return translationMatrix(vertOnOppositeSide) * shearMat * translationMatrix(-vertOnOppositeSide);
    }
}

#endif //TRENCHBROOM_BBOX_IMPL_H
