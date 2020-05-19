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

#ifndef TrenchBroom_HitAdapter
#define TrenchBroom_HitAdapter

#include <optional>

namespace TrenchBroom {
    namespace Model {
        class BrushFaceHandle;
        class EntityNode;
        class Hit;
        class Node;
        class Object;

        Node* hitToNode(const Hit& hit);
        Object* hitToObject(const Hit& hit);
        EntityNode* hitToEntity(const Hit& hit);
        std::optional<BrushFaceHandle> hitToFaceHandle(const Hit& hit);
    }
}

#endif /* defined(TrenchBroom_HitAdapter) */
