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

#include "LayerNode.h"

#include "Model/BrushNode.h"
#include "Model/ComputeNodeBoundsVisitor.h"
#include "Model/GroupNode.h"
#include "Model/EntityNode.h"
#include "Model/EntityAttributes.h"
#include "Model/IssueGenerator.h"
#include "Model/NodeVisitor.h"
#include "Model/TagVisitor.h"
#include "Model/WorldNode.h"

#include <limits>
#include <string>
#include <algorithm>
#include <stdexcept>

#include "Ensure.h"

namespace TrenchBroom {
    namespace Model {
        LayerNode::LayerNode(const std::string& name) :
        m_boundsValid(false) {
            setName(name);
        }

        void LayerNode::setName(const std::string& name) {
            addOrUpdateAttribute(AttributeNames::LayerName, name);
        }

        bool LayerNode::isDefaultLayer() const {
            Model::Node* parentNode = parent();
            if (parentNode == nullptr) {
                return false;
            }
            Model::WorldNode* world = dynamic_cast<Model::WorldNode*>(parentNode);
            ensure(world != nullptr, "layer parent must be WorldNode");
            return world->defaultLayer() == this;
        }

        int LayerNode::invalidSortIndex() {
            return std::numeric_limits<int>::max();
        }

        int LayerNode::defaultLayerSortIndex() {
            return -1;
        }

        int LayerNode::sortIndex() const {
            if (isDefaultLayer()) {
                return defaultLayerSortIndex();
            }

            const std::string& indexString = attribute(AttributeNames::LayerSortIndex);
            if (indexString.empty()) {
                return invalidSortIndex();
            }

            try {
                return std::stoi(indexString);
            } catch (const std::invalid_argument&) {
                return invalidSortIndex();
            } catch (const std::out_of_range&) {
                return invalidSortIndex();
            }
        }

        std::optional<Color> LayerNode::layerColor() const {
            const std::string& string = attribute(AttributeNames::LayerColor);
            if (string.empty() || !Color::canParse(string)) {
                return std::nullopt;
            }
            return { Color::parse(string) };
        }

        void LayerNode::setGroupColor(const Color& color) {
            addOrUpdateAttribute(AttributeNames::LayerColor, color.toString());
        }

        void LayerNode::setSortIndex(int index) {
            if (isDefaultLayer()) {
                return;
            }
            addOrUpdateAttribute(AttributeNames::LayerSortIndex, std::to_string(index));
        }

        void LayerNode::sortLayers(std::vector<LayerNode*>& layers)  {
            std::stable_sort(layers.begin(), layers.end(), [](LayerNode* a, LayerNode* b) {
                return a->sortIndex() < b->sortIndex();
            });
        }

        const std::string& LayerNode::doGetName() const {
            return attribute(AttributeNames::LayerName);
        }

        const vm::bbox3& LayerNode::doGetLogicalBounds() const {
            if (!m_boundsValid) {
                validateBounds();
            }
            return m_logicalBounds;
        }

        const vm::bbox3& LayerNode::doGetPhysicalBounds() const {
            if (!m_boundsValid) {
                validateBounds();
            }
            return m_physicalBounds;
        }

        Node* LayerNode::doClone(const vm::bbox3& worldBounds) const {
            LayerNode* layer = new LayerNode(doGetName());
            cloneAttributes(layer);
            layer->addChildren(clone(worldBounds, children()));
            return layer;
        }

        class CanAddChildToLayer : public ConstNodeVisitor, public NodeQuery<bool> {
        private:
            void doVisit(const WorldNode*)  override { setResult(false); }
            void doVisit(const LayerNode*)  override { setResult(false); }
            void doVisit(const GroupNode*)  override { setResult(true); }
            void doVisit(const EntityNode*) override { setResult(true); }
            void doVisit(const BrushNode*)  override { setResult(true); }
        };

        bool LayerNode::doCanAddChild(const Node* child) const {
            CanAddChildToLayer visitor;
            child->accept(visitor);
            return visitor.result();
        }

        bool LayerNode::doCanRemoveChild(const Node* /* child */) const {
            return true;
        }

        bool LayerNode::doRemoveIfEmpty() const {
            return false;
        }

        bool LayerNode::doShouldAddToSpacialIndex() const {
            return false;
        }

        void LayerNode::doNodePhysicalBoundsDidChange() {
            invalidateBounds();
        }

        bool LayerNode::doSelectable() const {
            return false;
        }

        void LayerNode::doPick(const vm::ray3& /* ray */, PickResult&) {}

        void LayerNode::doFindNodesContaining(const vm::vec3& point, std::vector<Node*>& result) {
            for (Node* child : Node::children())
                child->findNodesContaining(point, result);
        }

        void LayerNode::doGenerateIssues(const IssueGenerator* generator, std::vector<Issue*>& issues) {
            generator->generate(this, issues);
        }

        void LayerNode::doAccept(NodeVisitor& visitor) {
            visitor.visit(this);
        }

        void LayerNode::doAccept(ConstNodeVisitor& visitor) const {
            visitor.visit(this);
        }

        void LayerNode::doAttributesDidChange(const vm::bbox3& /* oldBounds */) {
        }

        bool LayerNode::doIsAttributeNameMutable(const std::string& /* name */) const {
            return false;
        }

        bool LayerNode::doIsAttributeValueMutable(const std::string& /* name */) const {
            return false;
        }

        vm::vec3 LayerNode::doGetLinkSourceAnchor() const {
            return vm::vec3::zero();
        }

        vm::vec3 LayerNode::doGetLinkTargetAnchor() const {
            return vm::vec3::zero();
        }

        void LayerNode::invalidateBounds() {
            m_boundsValid = false;
        }

        void LayerNode::validateBounds() const {
            ComputeNodeBoundsVisitor visitor(BoundsType::Logical, vm::bbox3(0.0));
            iterate(visitor);
            m_logicalBounds = visitor.bounds();

            ComputeNodeBoundsVisitor physicalBoundsVisitor(BoundsType::Physical, vm::bbox3(0.0));
            iterate(physicalBoundsVisitor);
            m_physicalBounds = physicalBoundsVisitor.bounds();

            m_boundsValid = true;
        }

        void LayerNode::doAcceptTagVisitor(TagVisitor& visitor) {
            visitor.visit(*this);
        }

        void LayerNode::doAcceptTagVisitor(ConstTagVisitor& visitor) const {
            visitor.visit(*this);
        }
    }
}
