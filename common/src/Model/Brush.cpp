/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "Brush.h"

#include "Model/NodeVisitor.h"

namespace TrenchBroom {
    namespace Model {
        Brush::Brush(const BBox3& worldBounds, const BrushFaceList& faces) {}

        bool Brush::doCanAddChild(Node* child) const {
            return false;
        }
        
        bool Brush::doCanRemoveChild(Node* child) const {
            return false;
        }
        
        void Brush::doAncestorDidChange() {}
        
        void Brush::doAccept(NodeVisitor& visitor) {
            visitor.visit(this);
        }
        
        void Brush::doAccept(ConstNodeVisitor& visitor) const {
            visitor.visit(this);
        }

        const BBox3& Brush::doGetBounds() const {}
        void Brush::doTransform(const Mat4x4& transformation, bool lockTextures, const BBox3& worldBounds) {}
        bool Brush::doContains(const Node* node) const {}
        bool Brush::doIntersects(const Node* node) const {}
    }
}
