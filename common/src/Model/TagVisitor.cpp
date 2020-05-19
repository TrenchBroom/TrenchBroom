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
        void TagVisitor::visit(World&) {}
        void TagVisitor::visit(Layer&) {}
        void TagVisitor::visit(Group&) {}
        void TagVisitor::visit(Entity&) {}
        void TagVisitor::visit(BrushNode&) {}
        void TagVisitor::visit(BrushFace&) {}

        ConstTagVisitor::~ConstTagVisitor() = default;
        void ConstTagVisitor::visit(const World&) {}
        void ConstTagVisitor::visit(const Layer&) {}
        void ConstTagVisitor::visit(const Group&) {}
        void ConstTagVisitor::visit(const Entity&) {}
        void ConstTagVisitor::visit(const BrushNode&) {}
        void ConstTagVisitor::visit(const BrushFace&) {}
    }
}
