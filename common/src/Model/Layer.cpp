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

#include "Layer.h"

#include "Model/TagMatcher.h"
#include "Model/Brush.h"
#include "Model/ComputeNodeBoundsVisitor.h"
#include "Model/Group.h"
#include "Model/Entity.h"
#include "Model/IssueGenerator.h"
#include "Model/NodeVisitor.h"
#include "Model/TagVisitor.h"

namespace TrenchBroom {
    namespace Model {
        Layer::Layer(const String& name, const vm::bbox3& worldBounds) :
        m_name(name),
        m_boundsValid(false) {}

        void Layer::setName(const String& name) {
            m_name = name;
        }

        const String& Layer::doGetName() const {
            return m_name;
        }

        const vm::bbox3& Layer::doGetBounds() const {
            if (!m_boundsValid) {
                validateBounds();
            }
            return m_bounds;
        }

        const vm::bbox3& Layer::doGetPhysicalBounds() const {
            if (!m_boundsValid) {
                validateBounds();
            }
            return m_cullingBounds;
        }

        Node* Layer::doClone(const vm::bbox3& worldBounds) const {
            Layer* layer = new Layer(m_name, worldBounds);
            cloneAttributes(layer);
            layer->addChildren(clone(worldBounds, children()));
            return layer;
        }

        class CanAddChildToLayer : public ConstNodeVisitor, public NodeQuery<bool> {
        private:
            void doVisit(const World* world) override   { setResult(false); }
            void doVisit(const Layer* layer) override   { setResult(false); }
            void doVisit(const Group* group) override   { setResult(true); }
            void doVisit(const Entity* entity) override { setResult(true); }
            void doVisit(const Brush* brush) override   { setResult(true); }
        };

        bool Layer::doCanAddChild(const Node* child) const {
            CanAddChildToLayer visitor;
            child->accept(visitor);
            return visitor.result();
        }

        bool Layer::doCanRemoveChild(const Node* child) const {
            return true;
        }

        bool Layer::doRemoveIfEmpty() const {
            return false;
        }

        bool Layer::doShouldAddToSpacialIndex() const {
            return false;
        }

        void Layer::doNodeBoundsDidChange(const vm::bbox3& oldBounds) {
            invalidateBounds();
        }

        bool Layer::doSelectable() const {
            return false;
        }

        void Layer::doPick(const vm::ray3& ray, PickResult& pickResult) const {}

        void Layer::doFindNodesContaining(const vm::vec3& point, NodeList& result) {
            for (Node* child : Node::children())
                child->findNodesContaining(point, result);
        }

        void Layer::doGenerateIssues(const IssueGenerator* generator, IssueList& issues) {
            generator->generate(this, issues);
        }

        void Layer::doAccept(NodeVisitor& visitor) {
            visitor.visit(this);
        }

        void Layer::doAccept(ConstNodeVisitor& visitor) const {
            visitor.visit(this);
        }

        void Layer::invalidateBounds() {
            m_boundsValid = false;
        }

        void Layer::validateBounds() const {
            ComputeNodeBoundsVisitor visitor(BoundsType::Regular, vm::bbox3(0.0));
            iterate(visitor);
            m_bounds = visitor.bounds();

            ComputeNodeBoundsVisitor cullingBoundsVisitor(BoundsType::Culling, vm::bbox3(0.0));
            iterate(cullingBoundsVisitor);
            m_cullingBounds = cullingBoundsVisitor.bounds();

            m_boundsValid = true;
        }

        void Layer::doAcceptTagVisitor(TagVisitor& visitor) {
            visitor.visit(*this);
        }

        void Layer::doAcceptTagVisitor(ConstTagVisitor& visitor) const {
            visitor.visit(*this);
        }
    }
}
