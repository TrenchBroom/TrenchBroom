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

#ifndef MortonTree_h
#define MortonTree_h

#include "NodeTree.h"

template <typename T, size_t S, typename U, typename Cmp = std::less<U>>
class MortonTree : public NodeTree<T,S,U,Cmp> {
public:
    MortonTree() {};

    ~MortonTree() override {
        clear();
    }

    virtual bool contains(const Box& bounds, const U& data) const = 0;

    virtual void insert(const Box& bounds, const U& data) = 0;

    virtual bool remove(const Box& bounds, const U& data) = 0;

    virtual void update(const Box& oldBounds, const Box& newBounds, const U& data) = 0;

    virtual void clear() = 0;

    virtual bool empty() const  = 0;

    virtual size_t height() const = 0;

    virtual const Box& bounds() const = 0;

    virtual List findIntersectors(const Ray<T,S>& ray) const = 0;

    virtual List findContainers(const Vec<T,S>& point) const = 0;
}

#endif /* MortonTree_h */
