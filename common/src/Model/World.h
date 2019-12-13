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

#ifndef TrenchBroom_World
#define TrenchBroom_World

#include "Macros.h"
#include "TrenchBroom.h"
#include "Model/AttributableNode.h"
#include "Model/AttributableNodeIndex.h"
#include "Model/IssueGeneratorRegistry.h"
#include "Model/MapFormat.h"
#include "Model/ModelFactory.h"
#include "Model/ModelFactoryImpl.h"
#include "Model/Node.h"

#include <string>
#include <vector>

template <typename T, size_t S, typename U>
class AABBTree;

namespace TrenchBroom {
    namespace Model {
        class PickResult;

        class World : public AttributableNode, public ModelFactory {
        private:
            ModelFactoryImpl m_factory;
            Layer* m_defaultLayer;
            AttributableNodeIndex m_attributableIndex;
            IssueGeneratorRegistry m_issueGeneratorRegistry;

            using NodeTree = AABBTree<FloatType, 3, Node*>;
            std::unique_ptr<NodeTree> m_nodeTree;
            bool m_updateNodeTree;
        public:
            World(MapFormat mapFormat);
            ~World() override;
        public: // layer management
            Layer* defaultLayer() const;
            std::vector<Layer*> allLayers() const;
            std::vector<Layer*> customLayers() const;
        private:
            void createDefaultLayer();
        public: // index
            const AttributableNodeIndex& attributableNodeIndex() const;
        public: // selection
            // issue generator registration
            const std::vector<IssueGenerator*>& registeredIssueGenerators() const;
            std::vector<IssueQuickFix*> quickFixes(IssueType issueTypes) const;
            void registerIssueGenerator(IssueGenerator* issueGenerator);
            void unregisterAllIssueGenerators();
        private:
            class AddNodeToNodeTree;
            class RemoveNodeFromNodeTree;
            class UpdateNodeInNodeTree;
        public: // node tree bulk updating
            class MatchTreeNodes;
            void disableNodeTreeUpdates();
            void enableNodeTreeUpdates();
            void rebuildNodeTree();
        private:
            class InvalidateAllIssuesVisitor;
            void invalidateAllIssues();
        private: // implement Node interface
            const vm::bbox3& doGetLogicalBounds() const override;
            const vm::bbox3& doGetPhysicalBounds() const override;
            Node* doClone(const vm::bbox3& worldBounds) const override;
            Node* doCloneRecursively(const vm::bbox3& worldBounds) const override;
            bool doCanAddChild(const Node* child) const override;
            bool doCanRemoveChild(const Node* child) const override;
            bool doRemoveIfEmpty() const override;
            bool doShouldAddToSpacialIndex() const override;

            void doDescendantWasAdded(Node* node, size_t depth) override;
            void doDescendantWillBeRemoved(Node* node, size_t depth) override;
            void doDescendantPhysicalBoundsDidChange(Node* node) override;

            bool doSelectable() const override;
            void doPick(const vm::ray3& ray, PickResult& pickResult) const override;
            void doFindNodesContaining(const vm::vec3& point, std::vector<Node*>& result) override;
            void doGenerateIssues(const IssueGenerator* generator, std::vector<Issue*>& issues) override;
            void doAccept(NodeVisitor& visitor) override;
            void doAccept(ConstNodeVisitor& visitor) const override;
            void doFindAttributableNodesWithAttribute(const AttributeName& name, const AttributeValue& value, std::vector<AttributableNode*>& result) const override;
            void doFindAttributableNodesWithNumberedAttribute(const AttributeName& prefix, const AttributeValue& value, std::vector<AttributableNode*>& result) const override;
            void doAddToIndex(AttributableNode* attributable, const AttributeName& name, const AttributeValue& value) override;
            void doRemoveFromIndex(AttributableNode* attributable, const AttributeName& name, const AttributeValue& value) override;
        private: // implement AttributableNode interface
            void doAttributesDidChange(const vm::bbox3& oldBounds) override;
            bool doIsAttributeNameMutable(const AttributeName& name) const override;
            bool doIsAttributeValueMutable(const AttributeName& name) const override;
            vm::vec3 doGetLinkSourceAnchor() const override;
            vm::vec3 doGetLinkTargetAnchor() const override;
        private: // implement ModelFactory interface
            MapFormat doGetFormat() const override;
            World* doCreateWorld() const override;
            Layer* doCreateLayer(const std::string& name) const override;
            Group* doCreateGroup(const std::string& name) const override;
            Entity* doCreateEntity() const override;
            Brush* doCreateBrush(const vm::bbox3& worldBounds, const std::vector<BrushFace*>& faces) const override;
            BrushFace* doCreateFace(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& attribs) const override;
            BrushFace* doCreateFace(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& attribs, const vm::vec3& texAxisX, const vm::vec3& texAxisY) const override;
        private: // implement Taggable interface
            void doAcceptTagVisitor(TagVisitor& visitor) override;
            void doAcceptTagVisitor(ConstTagVisitor& visitor) const override;
        private:
            deleteCopyAndMove(World)
        };
    }
}

#endif /* defined(TrenchBroom_World) */
