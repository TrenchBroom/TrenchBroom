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

#ifndef TrenchBroom_World
#define TrenchBroom_World

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/AttributableNode.h"
#include "Model/AttributableNodeIndex.h"
#include "Model/IssueGeneratorRegistry.h"
#include "Model/MapFormat.h"
#include "Model/ModelFactory.h"
#include "Model/ModelFactoryImpl.h"
#include "Model/Node.h"

namespace TrenchBroom {
    namespace Model {
        class BrushContentTypeBuilder;
        class PickResult;
        
        class World : public AttributableNode, public ModelFactory {
        private:
            ModelFactoryImpl m_factory;
            Layer* m_defaultLayer;
            AttributableNodeIndex m_attributableIndex;
            IssueGeneratorRegistry m_issueGeneratorRegistry;
            bool m_issuesMustBeValidated;
        public:
            World(MapFormat::Type mapFormat, const BrushContentTypeBuilder* brushContentTypeBuilder, const BBox3& worldBounds);
        public: // layer management
            Layer* defaultLayer() const;
            LayerList allLayers() const;
            LayerList customLayers() const;
        private:
            void createDefaultLayer(const BBox3& worldBounds);
        public: // selection
            // issue generator registration
            const IssueGeneratorList& registeredIssueGenerators() const;
            IssueQuickFixList quickFixes(IssueType issueTypes) const;
            void registerIssueGenerator(IssueGenerator* issueGenerator);
            void unregisterAllIssueGenerators();
        private:
            class InvalidateAllIssuesVisitor;
            void invalidateAllIssues();
        private: // implement Node interface
            const BBox3& doGetBounds() const;
            Node* doClone(const BBox3& worldBounds) const;
            Node* doCloneRecursively(const BBox3& worldBounds) const;
            bool doCanAddChild(const Node* child) const;
            bool doCanRemoveChild(const Node* child) const;
            bool doRemoveIfEmpty() const;
            bool doSelectable() const;
            void doPick(const Ray3& ray, PickResult& pickResult) const;
            FloatType doIntersectWithRay(const Ray3& ray) const;
            void doGenerateIssues(const IssueGenerator* generator, IssueList& issues);
            void doAccept(NodeVisitor& visitor);
            void doAccept(ConstNodeVisitor& visitor) const;
            void doFindAttributableNodesWithAttribute(const AttributeName& name, const AttributeValue& value, AttributableNodeList& result) const;
            void doFindAttributableNodesWithNumberedAttribute(const AttributeName& prefix, const AttributeValue& value, AttributableNodeList& result) const;
            void doAddToIndex(AttributableNode* attributable, const AttributeName& name, const AttributeValue& value);
            void doRemoveFromIndex(AttributableNode* attributable, const AttributeName& name, const AttributeValue& value);
        private: // implement AttributableNode interface
            void doAttributesDidChange();
            bool doIsAttributeNameMutable(const AttributeName& name) const;
            bool doIsAttributeValueMutable(const AttributeName& name) const;
            Vec3 doGetLinkSourceAnchor() const;
            Vec3 doGetLinkTargetAnchor() const;
        private: // implement ModelFactory interface
            MapFormat::Type doGetFormat() const;
            World* doCreateWorld(const BBox3& worldBounds) const;
            Layer* doCreateLayer(const String& name, const BBox3& worldBounds) const;
            Group* doCreateGroup(const String& name) const;
            Entity* doCreateEntity() const;
            Brush* doCreateBrush(const BBox3& worldBounds, const BrushFaceList& faces) const;
            BrushFace* doCreateFace(const Vec3& point1, const Vec3& point2, const Vec3& point3, const BrushFaceAttributes& attribs) const;
            BrushFace* doCreateFace(const Vec3& point1, const Vec3& point2, const Vec3& point3, const BrushFaceAttributes& attribs, const Vec3& texAxisX, const Vec3& texAxisY) const;
        private:
            World(const World&);
            World& operator=(const World&);
        };
    }
}

#endif /* defined(TrenchBroom_World) */
