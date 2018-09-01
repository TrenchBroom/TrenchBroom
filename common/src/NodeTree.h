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

#ifndef NodeTree_h
#define NodeTree_h

#include "bbox.h"
#include "Ray.h"

#include <functional>
#include <list>

template <typename T, size_t S, typename U, typename Cmp = std::less<U>>
class NodeTree {
public:
    using List = std::list<U>;
    using Array = std::vector<U>;
    using Box = bbox<T,S>;
    using DataType = U;
    using FloatType = T;
    static const size_t Components = S;
    
    using GetBounds = std::function<Box(const U& data)>;
public:
    virtual ~NodeTree() {}

    /**
     * Indicates whether a node with the given bounds and data exists in this tree.
     *
     * @param bounds the bounds to find
     * @param data the data to find
     * @return true if a node with the given bounds and data exists and false otherwise
     */
    virtual bool contains(const Box& bounds, const U& data) const = 0;

    /**
     * Clears this tree and rebuilds it by inserting given objects.
     *
     * @param objects the objects to insert
     * @param getBounds a function to compute the bounds from each object
     */
    virtual void clearAndBuild(const List& objects, const GetBounds& getBounds) {
        clear();
        for (const auto& object : objects) {
            insert(getBounds(object), object);
        }
    }

    /**
     * Clears this tree and rebuilds it by inserting given objects.
     *
     * @param objects the objects to insert
     * @param getBounds a function to compute the bounds from each object
     */
    virtual void clearAndBuild(const Array& objects, const GetBounds& getBounds) {
        clear();
        for (const auto& object : objects) {
            insert(getBounds(object), object);
        }
    }

    /**
     * Insert a node with the given bounds and data into this tree.
     *
     * @param bounds the bounds to insert
     * @param data the data to insert
     */
    virtual void insert(const Box& bounds, const U& data) = 0;

    /**
     * Removes the node with the given bounds and data into this tree.
     *
     * @param bounds the bounds to remove
     * @param data the data to remove
     * @return true if a node with the given bounds and data was removed, and false otherwise
     */
    virtual bool remove(const Box& bounds, const U& data) = 0;

    /**
     * Updates the node with the given bounds and data with the given new bounds.
     *
     * @param oldBounds the old bounds of the node to update
     * @param newBounds the new bounds of the node
     * @param data the node data
     *
     * @throws NodeTreeException if no node with the given bounds and data can be found in this tree
     */
    virtual void update(const Box& oldBounds, const Box& newBounds, const U& data) = 0;

    /**
     * Clears this node tree.
     */
    virtual void clear() = 0;

    /**
     * Indicates whether this tree is empty.
     *
     * @return true if this tree is empty and false otherwise
     */
    virtual bool empty() const  = 0;

    /**
     * Returns the bounds of all nodes in this tree.
     *
     * @return the bounds of all nodes in this tree, or a bounding box made up of NaN values if this tree is empty
     */
    virtual const Box& bounds() const = 0;

    /**
     * Finds every data item in this tree whose bounding box intersects with the given ray and retuns a list of those items.
     *
     * @param ray the ray to test
     * @return a list containing all found data items
     */
    virtual List findIntersectors(const Ray<T,S>& ray) const = 0;

    /**
     * Finds every data item in this tree whose bounding box contains the given point and returns a list of those items.
     *
     * @param point the point to test
     * @return a list containing all found data items
     */
    virtual List findContainers(const vec<T,S>& point) const = 0;
};

#endif /* NodeTree_h */
