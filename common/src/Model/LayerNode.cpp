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
        LayerNode::LayerNode(const bool defaultLayer, std::string name) :
        m_layer(defaultLayer, std::move(name)),
        m_boundsValid(false) {}

        LayerNode::LayerNode(std::string name) :
        LayerNode(false, std::move(name)) {}

        const Layer& LayerNode::layer() const {
            return m_layer;
        }

        Layer LayerNode::setLayer(Layer layer) {
            ensure(layer.defaultLayer() == m_layer.defaultLayer(), "Set same layer type");

            using std::swap;
            swap(m_layer, layer);
            return layer;
        }

        bool LayerNode::isDefaultLayer() const {
            return m_layer.defaultLayer();
        }

        void LayerNode::sortLayers(std::vector<LayerNode*>& layers)  {
            std::stable_sort(layers.begin(), layers.end(), [](LayerNode* a, LayerNode* b) {
                return a->layer().sortIndex() < b->layer().sortIndex();
            });
        }

        const std::string& LayerNode::doGetName() const {
            return layer().name();
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

        Node* LayerNode::doClone(const vm::bbox3&) const {
            LayerNode* layer = new LayerNode(isDefaultLayer(), doGetName());
            cloneAttributes(layer);
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
