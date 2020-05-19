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

namespace TrenchBroom {
    namespace Model {
        Object::Object() {}
        Object::~Object() {}

        Node* Object::container() const {
            return doGetContainer();
        }

        LayerNode* Object::layer() const {
            return doGetLayer();
        }

        GroupNode* Object::group() const {
            return doGetGroup();
        }

        bool Object::grouped() const {
            return group() != nullptr;
        }

        bool Object::groupOpened() const {
            const auto* containingGroup = group();
            return containingGroup == nullptr || containingGroup->opened();
        }

        void Object::transform(const vm::mat4x4& transformation, bool lockTextures, const vm::bbox3& worldBounds) {
            doTransform(transformation, lockTextures, worldBounds);
        }

        bool Object::contains(const Node* node) const {
            return doContains(node);
        }

        bool Object::intersects(const Node* node) const {
            return doIntersects(node);
        }
    }
}
