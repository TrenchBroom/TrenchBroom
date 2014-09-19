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

#ifndef __TrenchBroom__World__
#define __TrenchBroom__World__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/Attributable.h"
#include "Model/AttributableIndex.h"
#include "Model/Node.h"

namespace TrenchBroom {
    namespace Model {
        class World : public Attributable {
        private:
            Layer* m_defaultLayer;
            AttributableIndex m_attributableIndex;
        public:
            World();

        public: // factory methods
            Layer* createLayer(const String& name) const;
            Group* createGroup(const String& name) const;
            Entity* createEntity() const;
            Brush* createBrush(const BBox3& worldBounds, const BrushFaceList& faces) const;
        public: // layer management
            Layer* defaultLayer() const;
        private:
            void createDefaultLayer();
        private: // implement Node interface
            bool doCanAddChild(const Node* child) const;
            bool doCanRemoveChild(const Node* child) const;
            void doAccept(NodeVisitor& visitor);
            void doAccept(ConstNodeVisitor& visitor) const;
            void doFindAttributablesWithAttribute(const AttributeName& name, const AttributeValue& value, AttributableList& result) const;
            void doFindAttributablesWithNumberedAttribute(const AttributeName& prefix, const AttributeValue& value, AttributableList& result) const;
            void doAddToIndex(Attributable* attributable, const AttributeName& name, const AttributeValue& value);
            void doRemoveFromIndex(Attributable* attributable, const AttributeName& name, const AttributeValue& value);
        private: // implement Attributable interface
            void doAttributesDidChange();
            bool doCanAddOrUpdateAttribute(const AttributeName& name, const AttributeValue& value) const;
            bool doCanRenameAttribute(const AttributeName& name, const AttributeName& newName) const;
            bool doCanRemoveAttribute(const AttributeName& name) const;
        private:
            World(const World&);
            World& operator=(const World&);
        };
    }
}

#endif /* defined(__TrenchBroom__World__) */
