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

#include "Object.h"

#include "Model/GroupNode.h"

#include <kdl/result.h>

#include <iostream>

namespace TrenchBroom {
    namespace Model {
        std::ostream& operator<<(std::ostream& str, const TransformError& e) {
            str << e.msg;
            return str;
        }

        Object::Object() {}
        Object::~Object() {}

        Node* Object::container() {
            return doGetContainer();
        }

        const Node* Object::container() const {
            return const_cast<Object*>(this)->container();
        }

        LayerNode* Object::layer() {
            return doGetLayer();
        }

        const LayerNode* Object::layer() const {
            return const_cast<Object*>(this)->layer();
        }

        GroupNode* Object::group() {
            return doGetGroup();
        }

        const GroupNode* Object::group() const {
            return const_cast<Object*>(this)->group();
        }

        bool Object::grouped() const {
            return group() != nullptr;
        }

        bool Object::groupOpened() const {
            const auto* containingGroup = group();
            return containingGroup == nullptr || containingGroup->opened();
        }

        kdl::result<void, TransformError> Object::transform(const vm::bbox3& worldBounds, const vm::mat4x4& transformation, bool lockTextures) {
            return doTransform(worldBounds, transformation, lockTextures);
        }

        bool Object::contains(const Node* node) const {
            return doContains(node);
        }

        bool Object::intersects(const Node* node) const {
            return doIntersects(node);
        }
    }
}
