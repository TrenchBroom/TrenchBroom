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

#ifndef TrenchBroom_WorldNode
#define TrenchBroom_WorldNode

#include "FloatType.h"
#include "Macros.h"
#include "Model/AttributableNode.h"
#include "Model/MapFormat.h"
#include "Model/ModelFactory.h"
#include "Model/Node.h"

#include <kdl/result_forward.h>

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    template <typename T, size_t S, typename U> class AABBTree;

    namespace Model {
        class AttributableNodeIndex;
        enum class BrushError;
        class BrushFace;
        class IssueGeneratorRegistry;
        class IssueQuickFix;
        class PickResult;

        class WorldNode : public AttributableNode, public ModelFactory {
        private:
            std::unique_ptr<ModelFactory> m_factory;
            LayerNode* m_defaultLayer;
            std::unique_ptr<AttributableNodeIndex> m_attributableIndex;
            std::unique_ptr<IssueGeneratorRegistry> m_issueGeneratorRegistry;

            using NodeTree = AABBTree<FloatType, 3, Node*>;
            std::unique_ptr<NodeTree> m_nodeTree;
            bool m_updateNodeTree;
        public:
            WorldNode(Entity entity, MapFormat mapFormat);
            ~WorldNode() override;
        public: // layer management
            LayerNode* defaultLayer();

            const LayerNode* defaultLayer() const;

            /**
             * Returns defaultLayer() plus customLayers()
             */
            std::vector<LayerNode*> allLayers();

            /**
             * Returns defaultLayer() plus customLayers()
             */
            std::vector<const LayerNode*> allLayers() const;

            /**
             * Returns the custom layers in file order
             */
            std::vector<LayerNode*> customLayers();

            /**
             * Returns the custom layers in file order
             */
            std::vector<const LayerNode*> customLayers() const;

            /**
             * Returns defaultLayer() plus customLayers() ordered by LayerNode::sortIndex(). The default layer is always first.
             */
            std::vector<LayerNode*> allLayersUserSorted();

            /**
             * Returns defaultLayer() plus customLayers() ordered by LayerNode::sortIndex(). The default layer is always first.
             */
            std::vector<const LayerNode*> allLayersUserSorted() const;

            /**
             * Returns customLayers() ordered by LayerNode::sortIndex()
             */
            std::vector<LayerNode*> customLayersUserSorted();

            /**
             * Returns customLayers() ordered by LayerNode::sortIndex()
             */
            std::vector<const LayerNode*> customLayersUserSorted() const;
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
        public: // node tree bulk updating
            void disableNodeTreeUpdates();
            void enableNodeTreeUpdates();
            void rebuildNodeTree();
        private:
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
            void doPick(const vm::ray3& ray, PickResult& pickResult) override;
            void doFindNodesContaining(const vm::vec3& point, std::vector<Node*>& result) override;
            void doGenerateIssues(const IssueGenerator* generator, std::vector<Issue*>& issues) override;
            void doAccept(NodeVisitor& visitor) override;
            void doAccept(ConstNodeVisitor& visitor) const override;
            void doFindAttributableNodesWithAttribute(const std::string& name, const std::string& value, std::vector<AttributableNode*>& result) const override;
            void doFindAttributableNodesWithNumberedAttribute(const std::string& prefix, const std::string& value, std::vector<AttributableNode*>& result) const override;
            void doAddToIndex(AttributableNode* attributable, const std::string& name, const std::string& value) override;
            void doRemoveFromIndex(AttributableNode* attributable, const std::string& name, const std::string& value) override;
        private: // implement AttributableNode interface
            void doAttributesDidChange(const vm::bbox3& oldBounds) override;
            vm::vec3 doGetLinkSourceAnchor() const override;
            vm::vec3 doGetLinkTargetAnchor() const override;
        private: // implement ModelFactory interface
            MapFormat doGetFormat() const override;
            WorldNode* doCreateWorld(Entity entity) const override;
            LayerNode* doCreateLayer(const std::string& name) const override;
            GroupNode* doCreateGroup(const std::string& name) const override;
            EntityNode* doCreateEntity(Entity entity) const override;
            kdl::result<BrushFace, BrushError> doCreateFace(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& attribs) const override;
            kdl::result<BrushFace, BrushError> doCreateFaceFromStandard(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& attribs) const override;
            kdl::result<BrushFace, BrushError> doCreateFaceFromValve(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& attribs, const vm::vec3& texAxisX, const vm::vec3& texAxisY) const override;
        private: // implement Taggable interface
            void doAcceptTagVisitor(TagVisitor& visitor) override;
            void doAcceptTagVisitor(ConstTagVisitor& visitor) const override;
        private:
            deleteCopyAndMove(WorldNode)
        };
    }
}

#endif /* defined(TrenchBroom_WorldNode) */
