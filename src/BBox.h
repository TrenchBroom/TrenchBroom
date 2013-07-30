/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef TrenchBroom_BBox_h
#define TrenchBroom_BBox_h

#include "Plane.h"
#include "Vec.h"

template <typename T, size_t S>
class BBox {
public:
    typedef enum {
        Min,
        Max
    } MinMax;

    class RelativePosition {
    public:
        typedef enum {
            Less,
            Within,
            Greater
        } Range;
    private:
        Range m_positions[S];
    public:
        RelativePosition(const Range positions[S]) {
            for (size_t i = 0; i < S; ++i)
                m_positions[i] = positions[i];
        }
        
        inline const Range& operator[] (const size_t index) const {
            assert(index >= 0 && index < S);
            return m_positions[index];
        }
    };
    
    Vec<T,S> min;
    Vec<T,S> max;
    
    BBox() :
    min(Vec<T,S>::Null),
    max(Vec<T,S>::Null) {}
    
    BBox(const Vec<T,S>& i_min, const Vec<T,S>& i_max) :
    min(i_min),
    max(i_max) {}
    
    BBox(const T i_min, const T i_max) {
        min.set(i_min);
        max.set(i_max);
    }
    
    BBox(const Vec<T,S>& center, const T size) {
        for (size_t i = 0; i < S; i++) {
            min[i] = center[i] - size;
            max[i] = center[i] + size;
        }
    }
    
    BBox(const typename Vec<T,S>::List& vertices) {
        assert(vertices.size() > 0);
        min = max = vertices[0];
        for (size_t i = 0; i < vertices.size(); ++i)
            mergeWith(vertices[i]);
    }
    
    inline bool operator== (const BBox<T,S>& right) const {
        return min == right.min && max == right.max;
    }
    
    inline const Vec<T,S> center() const {
        return (min + max) / static_cast<T>(2.0);
    }
    
    inline const Vec<T,S> size() const {
        return max - min;
    }
    
    inline const Vec<T,S> vertex(MinMax c[S]) const {
        Vec<T,S> result;
        for (size_t i = 0; i < S; i++)
            result[i] = c[i] == Min ? min[i] : max[i];
        return result;
    }
    
    inline const Vec<T,3> vertex(MinMax x, MinMax y, MinMax z) const {
        MinMax c[] = {x, y, z};
        return vertex(c);
    }
    
    inline BBox<T,S>& mergeWith(const BBox<T,S>& right) {
        for (size_t i = 0; i < S; i++) {
            min[i] = std::min(min[i], right.min[i]);
            max[i] = std::max(max[i], right.max[i]);
        }
        return *this;
    }
    
    inline const BBox<T,S> mergedWith(const BBox<T,S>& right) const {
        return BBox<T,S>(*this).mergeWith(right);
    }
    
    
    inline BBox<T,S>& mergeWith(const Vec<T,S>& right) {
        for (size_t i = 0; i < S; i++) {
            min[i] = std::min(min[i], right[i]);
            max[i] = std::max(max[i], right[i]);
        }
        return *this;
    }
    
    inline const BBox<T,S> mergedWith(const Vec<T,S>& right) const {
        return BBox<T,S>(*this).mergeWith(right);
    }
    
    inline BBox<T,S>& translateToOrigin() {
        const Vec<T,S> c = center();
        min -= c;
        max -= c;
        return *this;
    }
    
    inline const BBox<T,S> translatedToOrigin() const {
        return BBox<T,S>(*this).translateToOrigin();
    }
    
    inline BBox<T,S>& repair() {
        for (size_t i = 0; i < S; i++)
            if (min[i] > max[i])
                std::swap(min[i], max[i]);
        return *this;
    }
    
    inline const BBox<T,S> repaired() const {
        return BBox<T,S>(*this).repair();
    }

    /*
    inline const Vec<T,S> vertex(const bool x, const bool y, const bool z) const {
        Vec<T,3> vertex;
        vertex[0] = x ? min[0] : max[0];
        vertex[1] = y ? min[1] : max[1];
        vertex[2] = z ? min[2] : max[2];
        return vertex;
    }
    
    inline const Vec<T,3> vertex(const size_t i) const {
        switch (i) {
            case 0:
                return vertex(false, false, false);
            case 1:
                return vertex(false, false, true);
            case 2:
                return vertex(false, true, false);
            case 3:
                return vertex(false, true, true);
            case 4:
                return vertex(true, false, false);
            case 5:
                return vertex(true, false, true);
            case 6:
                return vertex(true, true, false);
            case 7:
                return vertex(true, true, true);
        }
        return Vec<T,3>::NaN;
    }
    
    inline void vertices(typename Vec<T,3>::List& result) const {
        result.resize(24);
        result[ 0] = result[ 7] = result[16] = vertex(false, false, false);
        result[ 1] = result[ 2] = result[20] = vertex(true , false, false);
        result[ 3] = result[ 4] = result[22] = vertex(true , true , false);
        result[ 5] = result[ 6] = result[18] = vertex(false, true , false);
        result[ 8] = result[15] = result[17] = vertex(false, false, true );
        result[ 9] = result[10] = result[21] = vertex(true , false, true );
        result[11] = result[12] = result[23] = vertex(true , true , true );
        result[13] = result[14] = result[19] = vertex(false, true , true );
    }
    */
    
    inline bool contains(const Vec<T,S>& point) const {
        for (size_t i = 0; i < S; i++)
            if (point[i] < min[i] || point[i] > max[i])
                return false;
        return true;
    }
    
    inline const RelativePosition relativePosition(const Vec<T,S>& point) const {
        typename RelativePosition::Range p[S];
        for (size_t i = 0; i < S; i++) {
            if (point[i] < min[i])
                p[i] = RelativePosition::Less;
            else if (point[i] > max[i])
                p[i] = RelativePosition::Greater;
            else
                p[i] = RelativePosition::Within;
        }
        
        return RelativePosition(p);
    }

    inline bool contains(const BBox<T,S>& bounds) const {
        for (size_t i = 0; i < S; i++)
            if (bounds.min[i] < min[i] || bounds.max[i] > max[i])
                return false;
        return true;
    }
    
    inline bool intersects(const BBox<T,S>& bounds) const {
        for (size_t i = 0; i < S; i++)
            if (bounds.max[i] < min[i] || bounds.min[i] > max[i])
                return false;
        return true;
        
    }
    
    T intersectWithRay(const Ray<T,S>& ray, Vec<T,S>* sideNormal = NULL) const {
        const bool inside = contains(ray.origin);
        
        for (size_t i = 0; i < S; ++i) {
            if (ray.direction[i] == static_cast<T>(0.0))
                continue;
            
            Vec<T,S> normal, position;
            normal[i] = ray.direction[i] < static_cast<T>(0.0) ? static_cast<T>(1.0) : static_cast<T>(-1.0);
            if (inside)
                position = ray.direction[i] < static_cast<T>(0.0) ? min : max;
            else
                position = ray.direction[i] < static_cast<T>(0.0) ? max : min;
            
            const Plane<T,S> plane(position, normal);
            const T distance = plane.intersectWithRay(ray);
            if (Math<T>::isnan(distance))
                continue;
            
            const Vec<T,S> point = ray.pointAtDistance(distance);
            for (size_t j = 0; j < S; ++j)
                if (i != j && !Math<T>::between(point[j], min[j], max[j]))
                    goto cont;
            
            if (sideNormal != NULL)
                *sideNormal = inside ? -normal : normal;
            return distance;
            
        cont:;
        }
        
        return std::numeric_limits<T>::quiet_NaN();
    }
    
    inline BBox<T,S>& expand(const T f) {
        for (size_t i = 0; i < S; i++) {
            min[i] -= f;
            max[i] += f;
        }
        return *this;
    }
    
    inline const BBox<T,S> expanded(const T f) const {
        return BBox<T,S>(*this).expand(f);
    }
    
    inline BBox<T,S>& translate(const Vec<T,S>& delta) {
        min += delta;
        max += delta;
        return *this;
    }
    
    inline const BBox<T,S> translated(const Vec<T,S>& delta) const {
        return BBox<T,S>(*this).translate(delta);
    }
    
    /*
    inline BBox<T>& rotate(const Quat<T>& rotation) {
        return *this = rotated(rotation);
    }
    
    inline const BBox<T> rotated(const Quat<T>& rotation) const {
        BBox<T> result;
        result.min = result.max = rotation * vertex(false, false, false);
        result.mergeWith(rotation * vertex(false, false, true ));
        result.mergeWith(rotation * vertex(false, true , false));
        result.mergeWith(rotation * vertex(false, true , true ));
        result.mergeWith(rotation * vertex(true , false, false));
        result.mergeWith(rotation * vertex(true , false, true ));
        result.mergeWith(rotation * vertex(true , true , false));
        result.mergeWith(rotation * vertex(true , true , true ));
        return result;
    }
    
    inline BBox<T>& rotate(const Quat<T>& rotation, const Vec<T,3>& center) {
        return *this = rotated(rotation, center);
    }
    
    inline const BBox<T> rotated(const Quat<T>& rotation, const Vec<T,3>& center) const {
        BBox<T> result;
        result.min = result.max = rotation * (vertex(false, false, false) - center) + center;
        result.mergeWith(rotation * (vertex(false, false, true ) - center) + center);
        result.mergeWith(rotation * (vertex(false, true , false) - center) + center);
        result.mergeWith(rotation * (vertex(false, true , true ) - center) + center);
        result.mergeWith(rotation * (vertex(true , false, false) - center) + center);
        result.mergeWith(rotation * (vertex(true , false, true ) - center) + center);
        result.mergeWith(rotation * (vertex(true , true , false) - center) + center);
        result.mergeWith(rotation * (vertex(true , true , true ) - center) + center);
        return result;
    }
    
    BBox<T>& transform(const Mat4f& transformation) {
        return *this = transformed(transformation);
    }
    
    const BBox<T> transformed(const Mat4f& transformation) const {
        BBox<T> result;
        result.min = result.max = transformation * vertex(false, false, false);
        mergeWith(transformation * vertex(false, false, true ));
        mergeWith(transformation * vertex(false, true , false));
        mergeWith(transformation * vertex(false, true , true ));
        mergeWith(transformation * vertex(true , false, false));
        mergeWith(transformation * vertex(true , false, true ));
        mergeWith(transformation * vertex(true , true , false));
        mergeWith(transformation * vertex(true , true , true ));
        return result;
    }
    
    inline BBox<T>& flip(const Axis::Type axis) {
        min.flip(axis);
        max.flip(axis);
        repair();
        return *this;
    }
    
    inline const BBox<T> flipped(const Axis::Type axis) const {
        return BBox<T>(*this).flip(axis);
    }
    
    inline BBox<T>& flip(const Axis::Type axis, const Vec<T,3>& center) {
        min.flip(axis, center);
        max.flip(axis, center);
        repair();
        return *this;
    }
    
    inline const BBox<T> flipped(const Axis::Type axis, const Vec<T,3>& center) const {
        return BBox<T>(*this).flip(axis, center);
    }
     */
};

template <typename T, class Op>
inline void eachBBoxEdge(const BBox<T,3>& bbox, Op& op) {
    const Vec<T,3> size = bbox.size();
    const Vec<T,3> x(size.x(), static_cast<T>(0.0), static_cast<T>(0.0));
    const Vec<T,3> y(static_cast<T>(0.0), size.y(), static_cast<T>(0.0));
    const Vec<T,3> z(static_cast<T>(0.0), static_cast<T>(0.0), size.z());
    
    Vec<T,3> v1, v2;
    
    // top edges clockwise (viewed from above)
    v1 = bbox.max; v2 = bbox.max - y;
    op(v1, v2);
    v1 = v2; v2 -= x;
    op(v1, v2);
    v1 = v2; v2 += y;
    op(v1, v2);
    v1 = v2; v2 += x;
    op(v1, v2);
    
    // bottom edges clockwise (viewed from below)
    v1 = bbox.min; v2 = bbox.min + x;
    op(v1, v2);
    v1 = v2; v2 += y;
    op(v1, v2);
    v1 = v2; v2 -= x;
    op(v1, v2);
    v1 = v2; v2 -= y;
    op(v1, v2);
    
    // side edges clockwise (viewed from above
    v1 = bbox.min; v2 = bbox.min + z;
    op(v1, v2);
    v1 += y; v2 += y;
    op(v1, v2);
    v1 += x; v2 += x;
    op(v1, v2);
    v1 -= y; v2 -= y;
    op(v1, v2);
}

typedef BBox<float,3> BBox3f;
typedef BBox<double,3> BBox3d;

#endif
