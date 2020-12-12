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

#pragma once

#include "FloatType.h"

namespace TrenchBroom {
    namespace Model {
        class GroupNode;
        class LayerNode;
        class Node;

        class Object {
        protected:
            Object();
        public:
            virtual ~Object();

            Node* container();
            const Node* container() const;

            LayerNode* containingLayer();
            const LayerNode* containingLayer() const;

            GroupNode* group();
            const GroupNode* group() const;

            bool grouped() const;
            bool groupOpened() const;

            bool contains(const Node* object) const;
            bool intersects(const Node* object) const;
        private: // subclassing interface
            virtual Node* doGetContainer() = 0;
            virtual LayerNode* doGetContainingLayer() = 0;
            virtual GroupNode* doGetGroup() = 0;

            virtual bool doContains(const Node* node) const = 0;
            virtual bool doIntersects(const Node* node) const = 0;
        };
    }
}

