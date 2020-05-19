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

#include "MatchNodesByVisibility.h"

#include "Model/BrushNode.h"
#include "Model/GroupNode.h"
#include "Model/Entity.h"
#include "Model/LayerNode.h"
#include "Model/Node.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace Model {
        MatchNodesByVisibility::MatchNodesByVisibility(const VisibilityState visibility) :
        m_visibility(visibility) {}

        bool MatchNodesByVisibility::operator()(const Model::World* world) const   { return match(world);  }
        bool MatchNodesByVisibility::operator()(const Model::LayerNode* layer) const   { return match(layer);  }
        bool MatchNodesByVisibility::operator()(const Model::GroupNode* group) const   { return match(group);  }
        bool MatchNodesByVisibility::operator()(const Model::Entity* entity) const { return match(entity); }
        bool MatchNodesByVisibility::operator()(const Model::BrushNode* brush) const   { return match(brush);  }

        bool MatchNodesByVisibility::match(const Model::Node* node) const {
            return node->visibilityState() == m_visibility;
        }
    }
}
