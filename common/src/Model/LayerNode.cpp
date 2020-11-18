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

#include "Ensure.h"
#include "Model/BrushNode.h"
#include "Model/GroupNode.h"
#include "Model/EntityNode.h"
#include "Model/EntityAttributes.h"
#include "Model/IssueGenerator.h"
#include "Model/ModelUtils.h"
#include "Model/TagVisitor.h"
#include "Model/WorldNode.h"

#include <kdl/overload.h>
#include <kdl/string_utils.h>

#include <limits>
#include <string>
#include <algorithm>
#include <stdexcept>

namespace TrenchBroom {
    namespace Model {
        LayerNode::LayerNode(const std::string& name) :
        m_boundsValid(false) {
            setName(name);
        }

        void LayerNode::setName(const std::string& name) {
            auto entity = m_entity;
            entity.addOrUpdateAttribute(AttributeNames::LayerName, name);
            setEntity(std::move(entity));
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

            if (const auto* indexString = entity().attribute(AttributeNames::LayerSortIndex)) {
                return kdl::str_to_int(*indexString).value_or(invalidSortIndex());
            } else {
                return invalidSortIndex();
            }
        }

        std::optional<Color> LayerNode::layerColor() const {
            if (const auto* string = entity().attribute(AttributeNames::LayerColor); string && Color::canParse(*string)) {
                return { Color::parse(*string) };
            } else {
                return std::nullopt;
            }
        }

        void LayerNode::setLayerColor(const Color& color) {
            auto entity = m_entity;
            entity.addOrUpdateAttribute(AttributeNames::LayerColor, color.toString());
            setEntity(std::move(entity));
        }

        bool LayerNode::omitFromExport() const {
            return entity().hasAttribute(AttributeNames::LayerOmitFromExport, AttributeValues::LayerOmitFromExportValue);
        }

        void LayerNode::setOmitFromExport(const bool omitFromExport) {
            auto entity = m_entity;
            if (omitFromExport) {
                entity.addOrUpdateAttribute(AttributeNames::LayerOmitFromExport, AttributeValues::LayerOmitFromExportValue);
            } else {
                entity.removeAttribute(AttributeNames::LayerOmitFromExport);
            }
            setEntity(std::move(entity));
        }

        void LayerNode::setSortIndex(int index) {
            if (isDefaultLayer()) {
                return;
            }
            
            auto entity = m_entity;
            entity.addOrUpdateAttribute(AttributeNames::LayerSortIndex, std::to_string(index));
            setEntity(std::move(entity));
        }

        void LayerNode::sortLayers(std::vector<LayerNode*>& layers)  {
            std::stable_sort(layers.begin(), layers.end(), [](LayerNode* a, LayerNode* b) {
                return a->sortIndex() < b->sortIndex();
            });
        }

        const std::string& LayerNode::doGetName() const {
            static const auto NoName = std::string("");
            const auto* value = entity().attribute(AttributeNames::LayerName);
            return value ? *value : NoName;
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

        bool LayerNode::doCanAddChild(const Node* child) const {
            return child->accept(kdl::overload(
                [](const WorldNode*)  { return false; },
                [](const LayerNode*)  { return false; },
                [](const GroupNode*)  { return true; },
                [](const EntityNode*) { return true; },
                [](const BrushNode*)  { return true; }
            ));
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
            m_logicalBounds = computeLogicalBounds(children(), vm::bbox3(0.0));
            m_physicalBounds = computePhysicalBounds(children(), vm::bbox3(0.0));
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
