/*
 Copyright (C) 2010-2016 Kristian Duske

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

#ifndef TrenchBroom_SharedPointer_h
#define TrenchBroom_SharedPointer_h

#include <cassert>
#include <memory>

template <typename T>
std::shared_ptr<T> lock(std::shared_ptr<T> ptr) {
    return ptr;
}

template <typename T>
bool expired(std::shared_ptr<T> ptr) {
    return false;
}

template <typename T>
std::shared_ptr<T> lock(std::weak_ptr<T> ptr) {
    assert(!ptr.expired());
    return ptr.lock();
}

template <typename T>
bool expired(std::weak_ptr<T> ptr) {
    return ptr.expired();
}

template <typename T>
struct ArrayDeleter {
    void operator ()(T const* p) const {
        delete[] p;
    }
};

#endif
