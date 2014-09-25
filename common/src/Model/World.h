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
#include "Hit.h"
#include "Model/Attributable.h"
#include "Model/AttributableIndex.h"
#include "Model/IssueGeneratorRegistry.h"
#include "Model/MapFormat.h"
#include "Model/ModelFactory.h"
#include "Model/ModelFactoryImpl.h"
#include "Model/Node.h"
#include "Model/Picker.h"

namespace TrenchBroom {
    namespace Model {
        class BrushContentTypeBuilder;
        
        class World : public Attributable, public ModelFactory {
        private:
            ModelFactoryImpl m_factory;
            Layer* m_defaultLayer;
            Picker m_picker;
            AttributableIndex m_attributableIndex;
            IssueGeneratorRegistry m_issueGeneratorRegistry;
        public:
            World(MapFormat::Type mapFormat, BrushContentTypeBuilder* brushContentTypeBuilder);
        public: // layer management
            Layer* defaultLayer() const;
        private:
            void createDefaultLayer();
        public:
            // issue generator registration
            void registerIssueGenerators(const IssueGeneratorList& generators);
            void unregisterAllIssueGenerators();
        public: // picking
            Hits pick(const Ray3& ray) const;
        private: // implement Node interface
            Node* doClone(const BBox3& worldBounds) const;
            bool doCanAddChild(const Node* child) const;
            bool doCanRemoveChild(const Node* child) const;
            void doDescendantWasAdded(Node* node);
            void doDescendantWasRemoved(Node* node);
            void doDescendantWillChange(Node* node);
            void doDescendantDidChange(Node* node);
            bool doSelectable() const;
            void doUpdateIssues(Node* node);
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
        private: // implement ModelFactory interface
            World* doCreateWorld() const;
            Layer* doCreateLayer(const String& name) const;
            Group* doCreateGroup(const String& name) const;
            Entity* doCreateEntity() const;
            Brush* doCreateBrush(const BBox3& worldBounds, const BrushFaceList& faces) const;
            BrushFace* doCreateFace(const Vec3& point1, const Vec3& point2, const Vec3& point3, const String& textureName) const;
            BrushFace* doCreateFace(const Vec3& point1, const Vec3& point2, const Vec3& point3, const String& textureName, const Vec3& texAxisX, const Vec3& texAxisY) const;
        private:
            World(const World&);
            World& operator=(const World&);
        };
    }
}

#endif /* defined(__TrenchBroom__World__) */
