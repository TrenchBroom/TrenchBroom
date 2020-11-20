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

#ifndef TrenchBroom_EntityNode
#define TrenchBroom_EntityNode

#include "FloatType.h"
#include "Macros.h"
#include "Model/AttributableNode.h"
#include "Model/HitType.h"
#include "Model/Object.h"

#include <kdl/result_forward.h>

#include <vecmath/forward.h>
#include <vecmath/bbox.h>
#include <vecmath/util.h>

#include <optional>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Assets {
        enum class PitchType;
        class EntityModelFrame;
        struct ModelSpecification;
    }

    namespace Model {
        class EntityNode : public AttributableNode, public Object {
        public:
            static const HitType::Type EntityHitType;
            static const vm::bbox3 DefaultBounds;
        private:
            struct CachedBounds {
                vm::bbox3 modelBounds;
                vm::bbox3 logicalBounds;
                vm::bbox3 physicalBounds;
            };
            mutable std::optional<CachedBounds> m_cachedBounds;
        public:
            EntityNode();
            explicit EntityNode(Entity entity);
            explicit EntityNode(std::initializer_list<EntityAttribute> attributes);

            FloatType area(vm::axis::type axis) const;
        public: // entity model
            const vm::bbox3& modelBounds() const;
            void setModelFrame(const Assets::EntityModelFrame* modelFrame);
        private: // implement Node interface
            const vm::bbox3& doGetLogicalBounds() const override;
            const vm::bbox3& doGetPhysicalBounds() const override;

            Node* doClone(const vm::bbox3& worldBounds) const override;
            NodeSnapshot* doTakeSnapshot() override;

            bool doCanAddChild(const Node* child) const override;
            bool doCanRemoveChild(const Node* child) const override;
            bool doRemoveIfEmpty() const override;

            bool doShouldAddToSpacialIndex() const override;

            void doChildWasAdded(Node* node) override;
            void doChildWasRemoved(Node* node) override;

            void doNodePhysicalBoundsDidChange() override;
            void doChildPhysicalBoundsDidChange() override;

            bool doSelectable() const override;

            void doPick(const vm::ray3& ray, PickResult& pickResult) override;
            void doFindNodesContaining(const vm::vec3& point, std::vector<Node*>& result) override;

            void doGenerateIssues(const IssueGenerator* generator, std::vector<Issue*>& issues) override;
            void doAccept(NodeVisitor& visitor) override;
            void doAccept(ConstNodeVisitor& visitor) const override;

            std::vector<Node*> nodesRequiredForViewSelection() override;
        private: // implement AttributableNode interface
            void doAttributesDidChange(const vm::bbox3& oldBounds) override;
            vm::vec3 doGetLinkSourceAnchor() const override;
            vm::vec3 doGetLinkTargetAnchor() const override;
        private: // implement Object interface
            Node* doGetContainer() override;
            LayerNode* doGetLayer() override;
            GroupNode* doGetGroup() override;

            kdl::result<void, TransformError> doTransform(const vm::bbox3& worldBounds, const vm::mat4x4& transformation, bool lockTextures) override;
            bool doContains(const Node* node) const override;
            bool doIntersects(const Node* node) const override;
        private:
            void invalidateBounds();
            void validateBounds() const;
        private: // implement Taggable interface
            void doAcceptTagVisitor(TagVisitor& visitor) override;
            void doAcceptTagVisitor(ConstTagVisitor& visitor) const override;
        private:
            deleteCopyAndMove(EntityNode)
        };
    }
}

#endif /* defined(TrenchBroom_EntityNode) */
