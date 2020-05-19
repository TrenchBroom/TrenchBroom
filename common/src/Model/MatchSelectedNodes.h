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

#ifndef MatchSelectedNodes_h
#define MatchSelectedNodes_h

#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/Group.h"

namespace TrenchBroom {
    namespace Model {
        template <bool MatchSelected>
        class MatchSelectedNodes {
        public:
            bool operator()(const Model::World*) const           { return false; }
            bool operator()(const Model::Layer*) const           { return false; }
            bool operator()(const Model::Group* group) const     { return MatchSelected == group->selected(); }
            bool operator()(const Model::Entity* entity) const   { return MatchSelected == entity->selected(); }
            bool operator()(const Model::BrushNode* brush) const { return MatchSelected == brush->selected(); }
        };

        template <bool MatchSelected>
        class MatchTransitivelySelectedNodes {
        public:
            bool operator()(const Model::World*) const           { return false; }
            bool operator()(const Model::Layer*) const           { return false; }
            bool operator()(const Model::Group* group) const     { return MatchSelected == group->transitivelySelected(); }
            bool operator()(const Model::Entity* entity) const   { return MatchSelected == entity->transitivelySelected(); }
            bool operator()(const Model::BrushNode* brush) const { return MatchSelected == brush->transitivelySelected(); }
        };

        /**
         * If MatchSelected == true, it matches nodes that have either the node itself, a parent, or a descendant selected.
         * If MatchSelected == false, it matches nodes where the node itself is unselected, no parent is selected, and no descendant is selected.
         * Used e.g. for isolating on the selection.
         */
        template <bool MatchSelected>
        class MatchTransitivelySelectedOrDescendantSelectedNodes {
        public:
            bool operator()(const Model::World*) const           { return false; }
            bool operator()(const Model::Layer*) const           { return false; }
            bool operator()(const Model::Group* group) const     { return MatchSelected == (group->transitivelySelected() || group->descendantSelected()); }
            bool operator()(const Model::Entity* entity) const   { return MatchSelected == (entity->transitivelySelected() || entity->descendantSelected()); }
            bool operator()(const Model::BrushNode* brush) const { return MatchSelected == (brush->transitivelySelected() || brush->descendantSelected()); }
        };
    }
}

#endif /* MatchSelectedNodes_h */
