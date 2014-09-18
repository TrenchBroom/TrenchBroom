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

#ifndef __TrenchBroom__Entity__
#define __TrenchBroom__Entity__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/Attributable.h"
#include "Model/Object.h"
#include "Model/EntityRotationPolicy.h"

namespace TrenchBroom {
    namespace Model {
        class Entity : public Attributable, public Object, private EntityRotationPolicy {
        private:
            static const BBox3 DefaultBounds;
            mutable BBox3 m_bounds;
            mutable bool m_boundsValid;
        public:
            Entity();
            
            bool pointEntity() const;
            const Vec3 origin() const;
        private:
            void setOrigin(const Vec3& origin);
            void applyRotation(const Mat4x4& transformation);
        private: // implement Node interface
            bool doCanAddChild(Node* child) const;
            bool doCanRemoveChild(Node* child) const;
            void doAccept(NodeVisitor& visitor);
            void doAccept(ConstNodeVisitor& visitor) const;
        private: // implement Attributable interface
            void doAttributesDidChange();
            bool doCanAddOrUpdateAttribute(const AttributeName& name, const AttributeValue& value) const;
            bool doCanRenameAttribute(const AttributeName& name, const AttributeName& newName) const;
            bool doCanRemoveAttribute(const AttributeName& name) const;
        private: // implement Object interface
            const BBox3& doGetBounds() const;
            void doTransform(const Mat4x4& transformation, bool lockTextures, const BBox3& worldBounds);
            bool doContains(const Node* node) const;
            bool doIntersects(const Node* node) const;
        private:
            void invalidateBounds();
            void validateBounds() const;
        private:
            Entity(const Entity&);
            Entity& operator=(const Entity&);
        };
    }
}

#endif /* defined(__TrenchBroom__Entity__) */
