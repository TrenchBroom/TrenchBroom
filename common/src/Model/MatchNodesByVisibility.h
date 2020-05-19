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

#ifndef TrenchBroom_MatchNodesByVisibility
#define TrenchBroom_MatchNodesByVisibility

namespace TrenchBroom {
    namespace Model {
        class BrushNode;
        class Entity;
        class Group;
        class LayerNode;
        class Node;
        enum class VisibilityState;
        class World;

        class MatchNodesByVisibility {
        private:
            VisibilityState m_visibility;
        public:
            explicit MatchNodesByVisibility(VisibilityState visibility);

            bool operator()(const World* world) const;
            bool operator()(const LayerNode* layer) const;
            bool operator()(const Group* group) const;
            bool operator()(const Entity* entity) const;
            bool operator()(const BrushNode* brush) const;
        private:
            bool match(const Node* node) const;
        };
    }
}

#endif /* defined(TrenchBroom_MatchNodesByVisibility) */
