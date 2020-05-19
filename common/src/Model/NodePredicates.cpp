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

#include "NodePredicates.h"

#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/WorldNode.h"

namespace TrenchBroom {
    namespace Model {
        namespace NodePredicates {
            bool True::operator()(const Node*) const  { return true;  }
            bool False::operator()(const Node*) const { return false; }

            bool EqualsNode::operator()(const WorldNode* world) const   { return world  == m_node; }
            bool EqualsNode::operator()(WorldNode* world) const         { return world  == m_node; }
            bool EqualsNode::operator()(const LayerNode* layer) const   { return layer  == m_node; }
            bool EqualsNode::operator()(LayerNode* layer) const         { return layer  == m_node; }
            bool EqualsNode::operator()(const GroupNode* group) const   { return group  == m_node; }
            bool EqualsNode::operator()(GroupNode* group) const         { return group  == m_node; }
            bool EqualsNode::operator()(const Entity* entity) const { return entity == m_node; }
            bool EqualsNode::operator()(Entity* entity) const       { return entity == m_node; }
            bool EqualsNode::operator()(const BrushNode* brush) const   { return brush  == m_node; }
            bool EqualsNode::operator()(BrushNode* brush) const         { return brush  == m_node; }

            bool EqualsObject::operator()(const WorldNode*) const         { return false; }
            bool EqualsObject::operator()(WorldNode*) const               { return false; }
            bool EqualsObject::operator()(const LayerNode*) const         { return false; }
            bool EqualsObject::operator()(LayerNode*) const               { return false; }
            bool EqualsObject::operator()(const GroupNode* group) const   { return group  == m_object; }
            bool EqualsObject::operator()(GroupNode* group) const         { return group  == m_object; }
            bool EqualsObject::operator()(const Entity* entity) const { return entity == m_object; }
            bool EqualsObject::operator()(Entity* entity) const       { return entity == m_object; }
            bool EqualsObject::operator()(const BrushNode* brush) const   { return brush  == m_object; }
            bool EqualsObject::operator()(BrushNode* brush) const         { return brush  == m_object; }
        }
    }
}
