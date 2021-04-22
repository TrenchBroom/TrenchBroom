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

        Node* Object::container() {
            return doGetContainer();
        }

        const Node* Object::container() const {
            return const_cast<Object*>(this)->container();
        }

        LayerNode* Object::containingLayer() {
            return doGetContainingLayer();
        }

        const LayerNode* Object::containingLayer() const {
            return const_cast<Object*>(this)->containingLayer();
        }

        GroupNode* Object::containingGroup() {
            return doGetContainingGroup();
        }

        const GroupNode* Object::containingGroup() const {
            return const_cast<Object*>(this)->containingGroup();
        }

        bool Object::containedInGroup() const {
            return containingGroup() != nullptr;
        }

        bool Object::containingGroupOpened() const {
            const auto* group = containingGroup();
            return group == nullptr || group->opened();
        }
    }
}
