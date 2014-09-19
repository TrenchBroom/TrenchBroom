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

#ifndef __TrenchBroom__Brush__
#define __TrenchBroom__Brush__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/BrushContentType.h"
#include "Model/Node.h"
#include "Model/Object.h"
#include "Model/Selectable.h"

namespace TrenchBroom {
    namespace Model {
        class BrushGeometry;
        class Brush : public Node, public Object, public Selectable {
        private:
            BrushFaceList m_faces;
            BrushGeometry* m_geometry;
            
            mutable BrushContentType::FlagType m_contentType;
            mutable bool m_transparent;
            mutable bool m_contentTypeValid;
        public:
            Brush(const BBox3& worldBounds, const BrushFaceList& faces);
            ~Brush();
            
            Attributable* entity() const;
        public: // face management:
            const BrushFaceList& faces() const;
            void faceDidChange();
        private:
            void invalidateContentType();
            void validateContentType() const;
        private: // implement Node interface
            bool doCanAddChild(const Node* child) const;
            bool doCanRemoveChild(const Node* child) const;
            void doParentWillChange();
            void doAccept(NodeVisitor& visitor);
            void doAccept(ConstNodeVisitor& visitor) const;
        private: // implement Object interface
            const BBox3& doGetBounds() const;
            void doTransform(const Mat4x4& transformation, bool lockTextures, const BBox3& worldBounds);
            bool doContains(const Node* node) const;
            bool doIntersects(const Node* node) const;
        private: // implement Selectable interface
            void doWasSelected();
            void doWasDeselected();
        };
    }
}

#endif /* defined(__TrenchBroom__Brush__) */
