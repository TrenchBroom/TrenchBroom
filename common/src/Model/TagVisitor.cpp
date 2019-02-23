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

#include "TagVisitor.h"

namespace TrenchBroom {
    namespace Model {
        TagVisitor::~TagVisitor() = default;
        void TagVisitor::visit(World& world) {}
        void TagVisitor::visit(Layer& layer) {}
        void TagVisitor::visit(Group& group) {}
        void TagVisitor::visit(Entity& entity) {}
        void TagVisitor::visit(Brush& brush) {}
        void TagVisitor::visit(BrushFace& face) {}

        ConstTagVisitor::~ConstTagVisitor() = default;
        void ConstTagVisitor::visit(const World& world) {}
        void ConstTagVisitor::visit(const Layer& layer) {}
        void ConstTagVisitor::visit(const Group& group) {}
        void ConstTagVisitor::visit(const Entity& entity) {}
        void ConstTagVisitor::visit(const Brush& brush) {}
        void ConstTagVisitor::visit(const BrushFace& face) {}
    }
}
