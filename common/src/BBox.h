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

#ifndef TrenchBroom_BBox_h
#define TrenchBroom_BBox_h

#include "Algorithms.h"
#include "mat_forward.h"
#include "mat_type.h"
#include "mat_functions.h"
#include "Plane.h"
#include "Quat.h"
#include "vec_forward.h"
#include "vec_type.h"
#include "vec_functions.h"

#include "MathUtils.h"

#include <algorithm>
#include <array>
#include <string>
#include <sstream>

/**
 * An axis aligned bounding box that is represented by a min point and a max point. The min and max point are
 * constrained by the following invariant:
 *
 * For each component i < S, it holds that min[i] <= max[i].
 *
 * @tparam T the component type
 * @tparam S the number of components of the min and max points
 */
template <typename T, size_t S>
class BBox {
public:
    class RelativePosition {
    public:
        typedef enum {
            Range_Less,
            Range_Within,
            Range_Greater
        } Range;
    private:
        Range m_positions[S];
    public:
        RelativePosition(const Range positions[S]) {
            for (size_t i = 0; i < S; ++i)
                m_positions[i] = positions[i];
        }
        
        const Range& operator[] (const size_t index) const {
            assert(index >= 0 && index < S);
            return m_positions[index];
        }
    };
    
    vec<T,S> min;
    vec<T,S> max;
public:
    /**
     * Creates a new bounding box at the origin with size 0.
     */
    BBox() :
    min(vec<T,S>::zero),
    max(vec<T,S>::zero) {}
    
    // Copy and move constructors
    BBox(const BBox<T,S>& other) = default;
    BBox(BBox<T,S>&& other) noexcept = default;
    
    // Assignment operators
    BBox<T,S>& operator=(const BBox<T,S>& other) = default;
    BBox<T,S>& operator=(BBox<T,S>&& other) noexcept = default;

    /**
     * Creates a new bounding box by copying the values from the given bounding box. If the given box has a different
     * component type, the values are converted by calling the appropriate conversion constructor of the two vectors.
     *
     * @tparam U the component type of the given bounding box
     * @param other the bounding box to convert
     */
    template <typename U>
    explicit BBox(const BBox<U,S>& other) :
    min(other.min),
    max(other.max) {
        assert(isValid(*this));
    }

    /**
     * Creates a new bounding box with the given min and max values. The values are assumed to be correct, that is, for
     * each component, the corresponding value of the min point is smaller than or equal to the corresponding value of
     * the max point.
     *
     * @param i_min the min point of the bounding box
     * @param i_max the max point of the bounding box
     */
    BBox(const vec<T,S>& i_min, const vec<T,S>& i_max) :
    min(i_min),
    max(i_max) {
        assert(isValid(*this));
    }

    /**
     * Creates a new bounding box with the coordinate system origin at its center by setting the min point to
     * the negated given value, and the max point to the given value.
     *
     * The value is assumed to be correct, that is, none of its components must have a negative value.
     *
     * @param i_minMax the min and max point
     */
    BBox(const T i_minMax) :
    min(vec<T,S>::fill(-i_minMax)),
    max(vec<T,S>::fill(+i_minMax)) {
        assert(isValid(*this));
    }

    /**
     * Creates a new bounding box by setting each component of the min point to the given min value, and each component of
     * the max point to the given max value. This constructor assumes that the given min value does not exceed the given
     * max value.
     *
     * @param i_min the min point of the bounding box
     * @param i_max the max point of the bounding box
     */
    BBox(const T i_min, const T i_max) :
    min(vec<T,S>::fill(i_min)),
    max(vec<T,S>::fill(i_max)) {
        assert(isValid(*this));
    }
public:
    /**
     * Creates the smallest bounding box that contains all points in the given range. Optionally accepts a transformation
     * that is applied to each element of the range. The given range must not be empty.
     *
     * @tparam I the range iterator type
     * @tparam G type of the transformation
     * @param cur the start of the range
     * @param end the end of the range
     * @param get the transformation
     * @return the bounding box
     */
    template <typename I, typename G = Math::Identity>
    static BBox<T,S> mergeAll(I cur, I end, const G& get = G()) {
        assert(cur != end);
        const auto first = get(*cur++);
        BBox<T,S> result(first, first);
        while (cur != end) {
            result = merge(result, get(*cur++));
        }
        return result;
    }
public:
    BBox<T,S>& mix(const BBox<T,S>& box, const vec<T,S>& factor) {
        min = ::mix(min, box.min, factor);
        max = ::mix(max, box.max, factor);
        return *this;
    }
    
    BBox<T,S> mixed(const BBox<T,S>& box, const vec<T,S>& factor) const {
        return BBox<T,S>(*this).mix(box, factor);
    }
    
    BBox<T,S>& translateToOrigin() {
        const vec<T,S> c = center(*this);
        min -= c;
        max -= c;
        return *this;
    }
    
    BBox<T,S> translatedToOrigin() const {
        return BBox<T,S>(*this).translateToOrigin();
    }
    
    BBox<T,S>& repair() {
        using std::swap;
        for (size_t i = 0; i < S; ++i)
            if (min[i] > max[i])
                swap(min[i], max[i]);
        return *this;
    }
    
    BBox<T,S> repaired() const {
        return BBox<T,S>(*this).repair();
    }

    BBox<T,S> rounded() const {
        return BBox<T,S>(min.rounded(), max.rounded());
    }

    template <typename U>
    BBox<U,S> makeIntegral() const {
        return BBox<U,S>(min.template makeIntegral<U>(), max.template makeIntegral<U>());
    }

    bool contains(const vec<T,S>& point, const T epsilon = static_cast<T>(0.0)) const {
        for (size_t i = 0; i < S; ++i) {
            if (Math::lt(point[i], min[i], epsilon) ||
                Math::gt(point[i], max[i], epsilon)) {
                return false;
            }
        }
        return true;
    }
    
    RelativePosition relativePosition(const vec<T,S>& point) const {
        typename RelativePosition::Range p[S];
        for (size_t i = 0; i < S; ++i) {
            if (point[i] < min[i])
                p[i] = RelativePosition::Range_Less;
            else if (point[i] > max[i])
                p[i] = RelativePosition::Range_Greater;
            else
                p[i] = RelativePosition::Range_Within;
        }
        
        return RelativePosition(p);
    }

    bool contains(const BBox<T,S>& bounds, const T epsilon = static_cast<T>(0.0)) const {
        for (size_t i = 0; i < S; ++i) {
            if (Math::lt(bounds.min[i], min[i], epsilon) ||
                Math::gt(bounds.max[i], max[i], epsilon)) {
                return false;
            }
        }
        return true;
    }

    /**
     * Checks whether the given bounding box is contained within this bounding box, but without touching it.
     *
     * @param bounds the bounds to check
     * @param epsilon the epsilon value
     * @return true if the given bounding box is enclosed within this bounding box and false otherwise
     */
    bool encloses(const BBox<T,S>& bounds, const T epsilon = static_cast<T>(0.0)) const {
        for (size_t i = 0; i < S; ++i) {
            if (Math::lte(bounds.min[i], min[i], epsilon) ||
                Math::gte(bounds.max[i], max[i], epsilon)) {
                return false;
            }
        }
        return true;
    }

    /**
     * Constrains the given point to the volume covered by this bounding box.
     *
     * @param point the point to constrain
     * @return the constrained point
     */
    vec<T,S> constrain(const vec<T,S>& point) const {
        vec<T,S> result(point);
        for (size_t i = 0; i < S; ++i) {
            result[i] = Math::min(max[i], Math::max(min[i], result[i]));
        }
        return result;
    }

    bool intersects(const BBox<T,S>& bounds, const T epsilon = static_cast<T>(0.0)) const {
        for (size_t i = 0; i < S; ++i) {
            if (Math::lt(bounds.max[i], min[i], epsilon) ||
                Math::gt(bounds.min[i], max[i], epsilon)) {
                return false;
            }
        }
        return true;
    }
    
    bool touches(const vec<T,S>& start, const vec<T,S>& end, const T epsilon = static_cast<T>(0.0)) const {
        if (contains(start, epsilon) || contains(end, epsilon))
            return true;

        const Ray<T,S> ray(start, normalize(end - start));
        return !Math::isnan(intersectWithRay(ray));
    }

    T intersectWithRay(const Ray<T,S>& ray) const {
        // Compute candidate planes
        std::array<T, S> origins;
        std::array<bool, S> inside;
        bool allInside = true;
        for (size_t i = 0; i < S; ++i) {
            if (ray.origin[i] < min[i]) {
                origins[i] = min[i];
                allInside = inside[i] = false;
            } else if (ray.origin[i] > max[i]) {
                origins[i] = max[i];
                allInside = inside[i] = false;
            } else {
                if (ray.direction[i] < static_cast<T>(0.0)) {
                    origins[i] = min[i];
                } else {
                    origins[i] = max[i];
                }
                inside[i] = true;
            }
        }

        // Intersect candidate planes with ray
        std::array<T, S> distances;
        for (size_t i = 0; i < S; ++i) {
            if (ray.direction[i] != static_cast<T>(0.0)) {
                distances[i] = (origins[i] - ray.origin[i]) / ray.direction[i];
            } else {
                distances[i] = static_cast<T>(-1.0);
            }
        }

        size_t bestPlane = 0;
        if (allInside) {
            // find the closest plane that was hit
            for (size_t i = 1; i < S; ++i) {
                if (distances[i] < distances[bestPlane]) {
                    bestPlane = i;
                }
            }
        } else {
            // find the farthest plane that was hit
            for (size_t i = 0; i < S; ++i) {
                if (!inside[i]) {
                    bestPlane = i;
                    break;
                }
            }
            for (size_t i = bestPlane + 1; i < S; ++i) {
                if (!inside[i] && distances[i] > distances[bestPlane]) {
                    bestPlane = i;
                }
            }
        }

        // Check if the final candidate actually hits the box
        if (distances[bestPlane] < static_cast<T>(0.0)) {
            return std::numeric_limits<T>::quiet_NaN();
        }

        for (size_t i = 0; i < S; ++i) {
            if (bestPlane != i) {
                const T coord = ray.origin[i] + distances[bestPlane] * ray.direction[i];
                if (coord < min[i] || coord > max[i]) {
                    return std::numeric_limits<T>::quiet_NaN();
                }
            }
        }

        return distances[bestPlane];
    }
    
    BBox<T,S>& expand(const T f) {
        for (size_t i = 0; i < S; ++i) {
            min[i] -= f;
            max[i] += f;
        }
        return *this;
    }
    
    BBox<T,S> expanded(const T f) const {
        return BBox<T,S>(*this).expand(f);
    }
    
    BBox<T,S>& translate(const vec<T,S>& delta) {
        min += delta;
        max += delta;
        return *this;
    }
    
    BBox<T,S> translated(const vec<T,S>& delta) const {
        return BBox<T,S>(*this).translate(delta);
    }

    std::string asString() const {
        std::stringstream result;
        result << "[ (" << min << ") - (" << max << ") ]";
        return result.str();
    }
};

/**
 * Checks whether the two given bounding boxes are identical.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first bounding box
 * @param rhs the second bounding box
 * @return true if the two bounding boxes are identical, and false otherwise
 */
template <typename T, size_t S>
bool operator==(const BBox<T,S>& lhs, const BBox<T,S>& rhs) {
    return lhs.min == rhs.min && lhs.max == rhs.max;
}

/**
 * Checks whether the two given bounding boxes are identical.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first bounding box
 * @param rhs the second bounding box
 * @return false if the two bounding boxes are identical, and true otherwise
 */
template <typename T, size_t S>
bool operator!=(const BBox<T,S>& lhs, const BBox<T,S>& rhs) {
    return lhs.min != rhs.min || lhs.max != rhs.max;
}

/**
 * Checks whether a bounding box with the given min and max points satisfies its invariant. The invariant states that
 * for each component, the corresponding value of the min point must not exceed the corresponding value of the max
 * point.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param min the min point of the bounding box to check
 * @param max the max point of the bounding box to check
 * @return true if the given bounding box is valid and false otherwise
 */
template <typename T, size_t S>
bool isValidBBox(const vec<T,S>& min, const vec<T,S>& max) {
    for (size_t i = 0; i < S; ++i) {
        if (min[i] > max[i]) {
            return false;
        }
    }
    return true;
}

/**
 * Checks whether the given bounding box satisfies its invariant. The invariant states that for each component,
 * the corresponding value of the min point must not exceed the corresponding value of the max point.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param b the bounding box to check
 * @return true if the given bounding box is valid and false otherwise
 */
template <typename T, size_t S>
bool isValid(const BBox<T,S>& b) {
    return isValidBBox(b.min, b.max);
}

/**
 * Checks whether the given bounding box has an empty volume.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param b the bounding box to check
 * @return true if the given bounding box has an empty volume and false otherwise
 */
template <typename T, size_t S>
bool isEmpty(const BBox<T,S>& b) {
    for (size_t i = 0; i < S; ++i) {
        if (b.min[i] >= b.max[i]) {
            return true;
        }
    }
    return false;
}

/**
 * Computes the center of the given bounding box.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param b the bounding box
 * @return the center of the bounding box
 */
template <typename T, size_t S>
vec<T,S> center(const BBox<T,S>& b) {
    return (b.min + b.max) / static_cast<T>(2.0);
}

/**
 * Computes the size of the given bounding box.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param b the bounding box
 * @return the size of the bounding box
 */
template <typename T, size_t S>
vec<T,S> size(const BBox<T,S>& b) {
    return b.max - b.min;
}

/**
 * Computes the volume of the given bounding box.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @return the volumen of the bounding box
 */
template <typename T, size_t S>
T volume(const BBox<T,S>& b) {
    const auto boxSize = size(b);
    T result = boxSize[0];
    for (size_t i = 1; i < S; ++i) {
        result *= boxSize[i];
    }
    return result;
}

enum class BBoxCorner { min, max };

/**
 * Returns the position of a corner of the given bounding box according to the given spec.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param b the bounding box
 * @param c the corner to return
 * @return the position of the given corner
 */
template <typename T, size_t S>
vec<T,S> corner(const BBox<T,S>& b, const BBoxCorner c[S]) {
    vec<T,S> result;
    for (size_t i = 0; i < S; ++i) {
        result[i] = c[i] == BBoxCorner::min ? b.min[i] : b.max[i];
    }
    return result;
}

/**
 * Returns the position of a corner of the given bounding box according to the given spec.
 *
 * @tparam T the component type
 * @param b the bounding box
 * @param x the X position of the corner
 * @param y the Y position of the corner
 * @param z the Z position of the corner
 * @return the position of the given corner
 */
template <typename T>
vec<T,3> corner(const BBox<T,3>& b, const BBoxCorner x, const BBoxCorner y, const BBoxCorner z) {
    BBoxCorner c[] = { x, y, z };
    return corner(b, c);
}

/**
 * Returns the smallest bounding box that contains the two given bounding boxes.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first bounding box
 * @param rhs the second bounding box
 * @return the smallest bounding box that contains the given bounding boxes
 */
template <typename T, size_t S>
BBox<T,S> merge(const BBox<T,S>& lhs, const BBox<T,S>& rhs) {
    return BBox<T,S>(min(lhs.min, rhs.min), max(lhs.max, rhs.max));
}

/**
 * Returns the smallest bounding box that contains the given bounding box and the given point.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the bounding box
 * @param rhs the point
 * @return the smallest bounding box that contains the given bounding box and point
 */
template <typename T, size_t S>
BBox<T,S> merge(const BBox<T,S>& lhs, const vec<T,S>& rhs) {
    return BBox<T,S>(min(lhs.min, rhs), max(lhs.max, rhs));
}

/**
 * Returns the smallest bounding box that contains the intersection of the given bounding boxes.
 * If the intersection is empty, then an empty bounding box at the origin is returned.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first bounding box
 * @param rhs the second bounding box
 * @return the smallest bounding box that contains the intersection of the given bounding boxes
 */
template <typename T, size_t S>
BBox<T,S> intersect(const BBox<T,S>& lhs, const BBox<T,S>& rhs) {
    const auto min = ::max(lhs.min, rhs.min);
    const auto max = ::min(lhs.max, rhs.max);
    if (isValidBBox(min, max)) {
        return BBox<T,S>(min, max);
    } else {
        return BBox<T,S>(vec<T,S>::zero, vec<T,S>::zero);
    }
}

template <typename T, class Op>
void eachBBoxFace(const BBox<T,3>& bbox, Op& op) {
    const vec<T,3> boxSize = size(bbox);
    const vec<T,3> x(boxSize.x(), static_cast<T>(0.0), static_cast<T>(0.0));
    const vec<T,3> y(static_cast<T>(0.0), boxSize.y(), static_cast<T>(0.0));
    const vec<T,3> z(static_cast<T>(0.0), static_cast<T>(0.0), boxSize.z());
    
    op(bbox.max, bbox.max - y, bbox.max - y - x, bbox.max - x, vec<T,3>( 0.0,  0.0, +1.0)); // top
    op(bbox.min, bbox.min + x, bbox.min + x + y, bbox.min + y, vec<T,3>( 0.0,  0.0, -1.0)); // bottom
    op(bbox.min, bbox.min + z, bbox.min + z + x, bbox.min + x, vec<T,3>( 0.0, -1.0,  0.0)); // front
    op(bbox.max, bbox.max - x, bbox.max - x - z, bbox.max - z, vec<T,3>( 0.0, +1.0,  0.0)); // back
    op(bbox.min, bbox.min + y, bbox.min + y + z, bbox.min + z, vec<T,3>(-1.0,  0.0,  0.0)); // left
    op(bbox.max, bbox.max - z, bbox.max - z - y, bbox.max - y, vec<T,3>(+1.0,  0.0,  0.0)); // right
}

template <typename T, class Op>
void eachBBoxEdge(const BBox<T,3>& bbox, Op& op) {
    const vec<T,3> boxSize = size(bbox);
    const vec<T,3> x(boxSize.x(), static_cast<T>(0.0), static_cast<T>(0.0));
    const vec<T,3> y(static_cast<T>(0.0), boxSize.y(), static_cast<T>(0.0));
    const vec<T,3> z(static_cast<T>(0.0), static_cast<T>(0.0), boxSize.z());
    
    vec<T,3> v1, v2;
    
    // top edges clockwise (viewed from above)
    op(bbox.max,         bbox.max - y    );
    op(bbox.max - y,     bbox.max - y - x);
    op(bbox.max - y - x, bbox.max - x    );
    op(bbox.max - x,     bbox.max        );
    
    // bottom edges clockwise (viewed from below)
    op(bbox.min,         bbox.min + x    );
    op(bbox.min + x,     bbox.min + x + y);
    op(bbox.min + x + y, bbox.min + y    );
    op(bbox.min + y,     bbox.min        );
    
    // side edges clockwise (viewed from above)
    op(bbox.min,         bbox.min + z        );
    op(bbox.min + y,     bbox.min + y + z    );
    op(bbox.min + x + y, bbox.min + x + y + z);
    op(bbox.min + x,     bbox.min + x + z    );
}

template <typename T>
typename vec<T,3>::List bBoxVertices(const BBox<T,3>& bbox) {
    const vec<T,3> boxSize = size(bbox);
    const vec<T,3> x(boxSize.x(), static_cast<T>(0.0), static_cast<T>(0.0));
    const vec<T,3> y(static_cast<T>(0.0), boxSize.y(), static_cast<T>(0.0));
    const vec<T,3> z(static_cast<T>(0.0), static_cast<T>(0.0), boxSize.z());

    typename vec<T,3>::List vertices(8);
    
    // top vertices clockwise (viewed from above)
    vertices[0] = bbox.max;
    vertices[1] = bbox.max-y;
    vertices[2] = bbox.min+z;
    vertices[3] = bbox.max-x;
    
    // bottom vertices clockwise (viewed from below)
    vertices[4] = bbox.min;
    vertices[5] = bbox.min+x;
    vertices[6] = bbox.max-z;
    vertices[7] = bbox.min+y;
    return vertices;
}

template <typename T, class Op>
void eachBBoxVertex(const BBox<T,3>& bbox, Op& op) {
    const vec<T,3> boxSize = size(bbox);
    const vec<T,3> x(boxSize.x(),         static_cast<T>(0.0), static_cast<T>(0.0));
    const vec<T,3> y(static_cast<T>(0.0), boxSize.y(),         static_cast<T>(0.0));
    const vec<T,3> z(static_cast<T>(0.0), static_cast<T>(0.0), boxSize.z());
    
    // top vertices clockwise (viewed from above)
    op(bbox.max);
    op(bbox.max-y);
    op(bbox.min+z);
    op(bbox.max-x);
    
    // bottom vertices clockwise (viewed from below)
    op(bbox.min);
    op(bbox.min+x);
    op(bbox.max-z);
    op(bbox.min+y);
}

template <typename T>
struct RotateBBox {
    Quat<T> rotation;
    bool first;
    BBox<T,3> bbox;
    
    RotateBBox(const Quat<T>& i_rotation) :
    rotation(i_rotation),
    first(true) {}
    
    void operator()(const vec<T,3>& vertex) {
        if (first) {
            bbox.min = bbox.max = rotation * vertex;
            first = false;
        } else {
            bbox = merge(bbox, rotation * vertex);
        }
    }
};

template <typename T>
BBox<T,3> rotateBBox(const BBox<T,3>& bbox, const Quat<T>& rotation, const vec<T,3>& center = vec<T,3>::zero) {
    RotateBBox<T> rotator(rotation);
    eachBBoxVertex(bbox.translated(-center), rotator);
    return rotator.bbox.translated(center);
}

template <typename T>
struct TransformBBox {
    mat<T,4,4> transformation;
    bool first;
    BBox<T,3> bbox;
    
    TransformBBox(const mat<T,4,4>& i_transformation) :
    transformation(i_transformation),
    first(true) {}
    
    void operator()(const vec<T,3>& vertex) {
        if (first) {
            bbox.min = bbox.max = transformation * vertex;
            first = false;
        } else {
            bbox = merge(bbox, transformation * vertex);
        }
    }
};

template <typename T>
BBox<T,3> rotateBBox(const BBox<T,3>& bbox, const mat<T,4,4>& transformation) {
    TransformBBox<T> transformator(transformation);
    eachBBoxVertex(bbox, transformator);
    return transformator.bbox;
}

template <typename I, typename Get = Identity>
auto mergeBounds(I cur, I end, const Get& getBounds = Get()) {
    assert(cur != end);
    auto result = getBounds(*cur); ++cur;
    while (cur != end) {
        result = merge(result, getBounds(*cur));
        ++cur;
    };
    return result;
}

template <typename T>
mat<T,4,4> scaleBBoxMatrix(const BBox<T,3>& oldBBox, const BBox<T,3>& newBBox) {
    const vec<T,3>& oldSize = size(oldBBox);
    const vec<T,3>& newSize = size(newBBox);
    const vec<T,3> scaleFactors = newSize / oldSize;
    
    const mat<T,4,4> transform = translationMatrix(newBBox.min) * scalingMatrix(scaleFactors) * translationMatrix(-oldBBox.min);
    return transform;
}

template <typename T>
mat<T,4,4> scaleBBoxMatrixWithAnchor(const BBox<T,3>& oldBBox, const vec<T,3>& newSize, const vec<T,3>& anchorPoint) {
    const vec<T,3>& oldSize = size(oldBBox);
    const vec<T,3> scaleFactors = newSize / oldSize;

    const mat<T,4,4> transform = translationMatrix(anchorPoint) * scalingMatrix(scaleFactors) * translationMatrix(-anchorPoint);
    return transform;
}

template <typename T>
mat<T,4,4> shearBBoxMatrix(const BBox<T,3>& box, const vec<T,3>& sideToShear, const vec<T,3>& delta) {
    const auto oldSize = size(box);
    
    // shearMatrix(const T Sxy, const T Sxz, const T Syx, const T Syz, const T Szx, const T Szy) {
    mat<T,4,4> shearMat;
    if (sideToShear == vec<T,3>::pos_x) {
        const auto relativeDelta = delta / oldSize.x();
        shearMat = shearMatrix(relativeDelta.y(), relativeDelta.z(), 0., 0., 0., 0.);
    }
    if (sideToShear == vec<T,3>::neg_x) {
        const auto relativeDelta = delta / oldSize.x();
        shearMat = shearMatrix(-relativeDelta.y(), -relativeDelta.z(), 0., 0., 0., 0.);
    }
    if (sideToShear == vec<T,3>::pos_y) {
        const auto relativeDelta = delta / oldSize.y();
        shearMat = shearMatrix(0., 0., relativeDelta.x(), relativeDelta.z(), 0., 0.);
    }
    if (sideToShear == vec<T,3>::neg_y) {
        const auto relativeDelta = delta / oldSize.y();
        shearMat = shearMatrix(0., 0., -relativeDelta.x(), -relativeDelta.z(), 0., 0.);
    }
    if (sideToShear == vec<T,3>::pos_z) {
        const auto relativeDelta = delta / oldSize.z();
        shearMat = shearMatrix(0., 0., 0., 0., relativeDelta.x(), relativeDelta.y());
    }
    if (sideToShear == vec<T,3>::neg_z) {
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
    eachBBoxFace(box, visitor);
    assert(didGrab);
    
    const mat<T,4,4> transform = translationMatrix(vertOnOppositeSide) * shearMat * translationMatrix(-vertOnOppositeSide);
    return transform;
}

template <typename T, size_t S>
std::ostream& operator<<(std::ostream& stream, const BBox<T,S>& bbox) {
    stream << "{min:" << bbox.min << " max:" << bbox.max << "}";
    return stream;
}

typedef BBox<float,1> BBox1f;
typedef BBox<double,1> BBox1d;
typedef BBox<float,2> BBox2f;
typedef BBox<double,2> BBox2d;
typedef BBox<float,3> BBox3f;
typedef BBox<double,3> BBox3d;

#endif
