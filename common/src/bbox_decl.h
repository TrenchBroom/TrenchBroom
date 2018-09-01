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

#include "mat_decl.h"
#include "Plane.h" // TODO: 2201: replace with plane_decl.h
#include "Quat.h" // TODO: 2201: replace with quat_decl.h
#include "vec_decl.h"

#include <array>
#include <iostream>

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
class bbox {
public:
    vec<T,S> min;
    vec<T,S> max;
public:
    /**
     * Creates a new bounding box at the origin with size 0.
     */
    bbox();
    
    // Copy and move constructors
    bbox(const bbox<T,S>& other) = default;
    bbox(bbox<T,S>&& other) noexcept = default;
    
    // Assignment operators
    bbox<T,S>& operator=(const bbox<T,S>& other) = default;
    bbox<T,S>& operator=(bbox<T,S>&& other) noexcept = default;

    /**
     * Creates a new bounding box by copying the values from the given bounding box. If the given box has a different
     * component type, the values are converted by calling the appropriate conversion constructor of the two vectors.
     *
     * @tparam U the component type of the given bounding box
     * @param other the bounding box to convert
     */
    template <typename U>
    explicit bbox(const bbox<U,S>& other) :
    min(other.min),
    max(other.max) {
        assert(valid());
    }

    /**
     * Creates a new bounding box with the given min and max values. The values are assumed to be correct, that is, for
     * each component, the corresponding value of the min point is smaller than or equal to the corresponding value of
     * the max point.
     *
     * @param i_min the min point of the bounding box
     * @param i_max the max point of the bounding box
     */
    bbox(const vec<T,S>& i_min, const vec<T,S>& i_max);

    /**
     * Creates a new bounding box with the coordinate system origin at its center by setting the min point to
     * the negated given value, and the max point to the given value.
     *
     * The value is assumed to be correct, that is, none of its components must have a negative value.
     *
     * @param i_minMax the min and max point
     */
    bbox(T i_minMax);

    /**
     * Creates a new bounding box by setting each component of the min point to the given min value, and each component of
     * the max point to the given max value. This constructor assumes that the given min value does not exceed the given
     * max value.
     *
     * @param i_min the min point of the bounding box
     * @param i_max the max point of the bounding box
     */
    bbox(T i_min, T i_max);
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
    static bbox<T,S> mergeAll(I cur, I end, const G& get = G()) {
        assert(cur != end);
        const auto first = get(*cur++);
        bbox<T,S> result(first, first);
        while (cur != end) {
            result = merge(result, get(*cur++));
        }
        return result;
    }
public:
    /**
     * Checks whether a bounding box with the given min and max points satisfies its invariant. The invariant states that
     * for each component, the corresponding value of the min point must not exceed the corresponding value of the max
     * point.
     *
     * @return true if the bounding box with the given points is valid and false otherwise
     */
    static bool valid(const vec<T,S>& min, const vec<T,S>& max);

    /**
     * Checks whether this bounding box satisfies its invariant. The invariant states that
     * for each component, the corresponding value of the min point must not exceed the corresponding value of the max
     * point.
     *
     * @return true if this bounding box is valid and false otherwise
     */
    bool valid() const;

    /**
     * Checks whether this bounding box has an empty volume.
     *
     * @return true if this bounding box has an empty volume and false otherwise
     */
    bool empty() const;

    /**
     * Computes the center of this bounding box.
     *
     * @return the center of this bounding box
     */
    vec<T,S> center() const;

    /**
     * Computes the size of this bounding box
     *
     * @return the size of this bounding box
     */
    vec<T,S> size() const;

    /**
     * Computes the volume of this bounding box.
     *
     * @return the volumen of this bounding box
     */
    T volume() const;

    /**
     * Checks whether the given point is cointained in this bounding box.
     *
     * @param point the point
     * @param epsilon an epsilon value
     * @return true if the given point is contained in the given bounding box and false otherwise
     */
    bool contains(const vec<T,S>& point, T epsilon = static_cast<T>(0.0)) const;

    /**
     * Checks whether the given bounding box is contained in this bounding box.
     *
     * @param b the possibly contained bounding box
     * @param epsilon an epsilon value
     * @return true if the given bounding box is contained in this bounding box
     */
    bool contains(const bbox<T,S>& b, T epsilon = static_cast<T>(0.0)) const;

    /**
     * Checks whether the given bounding box is enclosed in this bounding box. This is equivalent to
     * checking whether given box is contained within this box such that the boxes don't touch at all.
     *
     * @param b the possibly enclosed bounding box
     * @param epsilon an epsilon value
     * @return true if the given bounding box is enclosed in this bounding box
     */
    bool encloses(const bbox<T,S>& b, T epsilon = static_cast<T>(0.0)) const;

    /**
     * Checks whether the given bounding box intersects with this bounding box.

     * @param b the second bounding box
     * @param epsilon an epsilon value
     * @return true if the given bounding box intersects with this bounding box and false otherwise
     */
    bool intersects(const bbox<T,S>& b, T epsilon = static_cast<T>(0.0)) const;

    /**
     * Constrains the given point to the volume covered by this bounding box.
     *
     * @param point the point to constrain
     * @return the constrained point
     */
    vec<T,S> constrain(const vec<T,S>& point) const;

    enum class Corner { min, max };

    /**
     * Returns the position of a corner of this bounding box according to the given spec.
     *
     * @param c the corner to return
     * @return the position of the given corner
     */
    vec<T,S> corner(const Corner c[S]) const;

    /**
     * Returns the position of a corner of this bounding box according to the given spec.
     *
     * @param x the X position of the corner
     * @param y the Y position of the corner
     * @param z the Z position of the corner
     * @return the position of the given corner
     */
    vec<T,3> corner(Corner x, Corner y, Corner z) const;

    enum class Range { less, within, greater };

    /**
     * Returns the relative position of the given point. For each component, the returned array contains a
     * value of the Range enum which indicates one of the following three cases:
     *
     * - the component of the point is less than the corresponding component of the min point
     * - the component of the point is greater than the corresponding component of the max point
     * - the component of the point is in the range defined by the corresponding components of the min and max point (inclusive)
     *
     * @param point the point to check
     * @return the relative position
     */
    std::array<Range, S> relativePosition(const vec<T,S>& point) const;

    /**
     * Expands this bounding box by the given delta.
     *
     * @param f the value by which to expand this bounding box
     * @return the expanded bounding box
     */
    bbox<T,S> expand(const T f) const;

    /**
     * Translates this bounding box by the given delta.
     *
     * @param delta the delta by which to translate
     * @return the translated bounding box
     */
    bbox<T,S> translate(const vec<T,S>& delta) const;

    /**
     * Transforms this bounding box by applying the given transformation to each corner vertex. The result is the
     * smallest bounding box that contains the transformed vertices.
     *
     * @param transform the transformation
     * @return the transformed bounding box
     */
    bbox<T,S> transform(const mat<T,S+1,S+1>& transform) const;

    /**
     * Executes the given operation on every face of this bounding box. For each face, its four vertices
     * are passed to the given operation in a clock wise manner.
     * 
     * @tparam Op the type of the operation 
     * @param op the operation
     */
    template <typename Op>
    void forEachFace(Op&& op) const {
        const vec<T,3> boxSize = size();
        const vec<T,3> x(boxSize.x(), static_cast<T>(0.0), static_cast<T>(0.0));
        const vec<T,3> y(static_cast<T>(0.0), boxSize.y(), static_cast<T>(0.0));
        const vec<T,3> z(static_cast<T>(0.0), static_cast<T>(0.0), boxSize.z());

        op(max, max - y, max - y - x, max - x, vec<T,3>( 0.0,  0.0, +1.0)); // top
        op(min, min + x, min + x + y, min + y, vec<T,3>( 0.0,  0.0, -1.0)); // bottom
        op(min, min + z, min + z + x, min + x, vec<T,3>( 0.0, -1.0,  0.0)); // front
        op(max, max - x, max - x - z, max - z, vec<T,3>( 0.0, +1.0,  0.0)); // back
        op(min, min + y, min + y + z, min + z, vec<T,3>(-1.0,  0.0,  0.0)); // left
        op(max, max - z, max - z - y, max - y, vec<T,3>(+1.0,  0.0,  0.0)); // right
    }

    /**
     * Executes the given operation for each edge of this bounding box. For each edge, the two vertices
     * which are connected by that edge are passed to the operation.
     *
     * @tparam Op the type of the operation
     * @param op the operation
     */
    template <typename Op>
    void forEachEdge(Op&& op) const {
        const vec<T,3> boxSize = size();
        const vec<T,3> x(boxSize.x(), static_cast<T>(0.0), static_cast<T>(0.0));
        const vec<T,3> y(static_cast<T>(0.0), boxSize.y(), static_cast<T>(0.0));
        const vec<T,3> z(static_cast<T>(0.0), static_cast<T>(0.0), boxSize.z());

        // top edges clockwise (viewed from above)
        op(max,         max - y    );
        op(max - y,     max - y - x);
        op(max - y - x, max - x    );
        op(max - x,     max        );

        // bottom edges clockwise (viewed from below)
        op(min,         min + x    );
        op(min + x,     min + x + y);
        op(min + x + y, min + y    );
        op(min + y,     min        );

        // side edges clockwise (viewed from above)
        op(min,         min + z        );
        op(min + y,     min + y + z    );
        op(min + x + y, min + x + y + z);
        op(min + x,     min + x + z    );
    }

    /**
     * Executes the given operation for each vertex of this bounding box.
     *
     * @tparam Op the type of the operation
     * @param op the operation
     */
    template <class Op>
    void forEachVertex(Op&& op) const {
        const vec<T,3> boxSize = size();
        const vec<T,3> x(boxSize.x(),         static_cast<T>(0.0), static_cast<T>(0.0));
        const vec<T,3> y(static_cast<T>(0.0), boxSize.y(),         static_cast<T>(0.0));
        const vec<T,3> z(static_cast<T>(0.0), static_cast<T>(0.0), boxSize.z());

        // top vertices clockwise (viewed from above)
        op(max);
        op(max-y);
        op(min+z);
        op(max-x);

        // bottom vertices clockwise (viewed from below)
        op(min);
        op(min+x);
        op(max-z);
        op(min+y);
    }

    /**
     * Returns a list containing all 8 corner vertices of this bounding box.
     *
     * @return a list of vertices
     */
    typename vec<T,S>::List vertices() const;
};

/**
 * Prints a textual representation of the given bounding box onto the given stream.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param stream the stream to print to
 * @param bbox the bounding box to print
 * @return the given stream
 */
template <typename T, size_t S>
std::ostream& operator<<(std::ostream& stream, const bbox<T,S>& bbox);

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
bool operator==(const bbox<T,S>& lhs, const bbox<T,S>& rhs);

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
bool operator!=(const bbox<T,S>& lhs, const bbox<T,S>& rhs);

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
bbox<T,S> merge(const bbox<T,S>& lhs, const bbox<T,S>& rhs);

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
bbox<T,S> merge(const bbox<T,S>& lhs, const vec<T,S>& rhs);

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
bbox<T,S> intersect(const bbox<T,S>& lhs, const bbox<T,S>& rhs);


/**
 * Computes the point of intersection between the given ray and the given bounding box, and returns the distance
 * on the given ray from the ray's origin to that point.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param r the ray
 * @param b the bounding box
 * @return the distance to the intersection point, or NaN if the ray does not intersect the bounding box
 */
template <typename T, size_t S>
T intersect(const Ray<T,S>& r, const bbox<T,S>& b);

template <typename T>
mat<T,4,4> scaleBBoxMatrix(const bbox<T,3>& oldBBox, const bbox<T,3>& newBBox);

template <typename T>
mat<T,4,4> scaleBBoxMatrixWithAnchor(const bbox<T,3>& oldBBox, const vec<T,3>& newSize, const vec<T,3>& anchorPoint);

template <typename T>
mat<T,4,4> shearBBoxMatrix(const bbox<T,3>& box, const vec<T,3>& sideToShear, const vec<T,3>& delta);

typedef bbox<float,1> BBox1f;
typedef bbox<double,1> BBox1d;
typedef bbox<float,2> BBox2f;
typedef bbox<double,2> BBox2d;
typedef bbox<float,3> BBox3f;
typedef bbox<double,3> BBox3d;

#endif
