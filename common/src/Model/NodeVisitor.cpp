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

#include "NodeVisitor.h"

namespace TrenchBroom {
    namespace Model {
        NodeVisitor::NodeVisitor() {}

        NodeVisitor::~NodeVisitor() {}

        void NodeVisitor::visit(WorldNode* world) {
            doVisit(world);
        }

        void NodeVisitor::visit(LayerNode* layer) {
            doVisit(layer);
        }

        void NodeVisitor::visit(GroupNode* group) {
            doVisit(group);
        }

        void NodeVisitor::visit(EntityNode* entity) {
            doVisit(entity);
        }

        void NodeVisitor::visit(BrushNode* brush) {
            doVisit(brush);
        }

        ConstNodeVisitor::ConstNodeVisitor() {}

        ConstNodeVisitor::~ConstNodeVisitor() {}

        void ConstNodeVisitor::visit(const WorldNode* world) {
            doVisit(world);
        }

        void ConstNodeVisitor::visit(const LayerNode* layer) {
            doVisit(layer);
        }

        void ConstNodeVisitor::visit(const GroupNode* group) {
            doVisit(group);
        }

        void ConstNodeVisitor::visit(const EntityNode* entity) {
            doVisit(entity);
        }

        void ConstNodeVisitor::visit(const BrushNode* brush) {
            doVisit(brush);
        }
    }
}
