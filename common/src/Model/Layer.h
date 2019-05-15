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

#ifndef TrenchBroom_Layer
#define TrenchBroom_Layer

#include "StringUtils.h"
#include "Model/ModelTypes.h"
#include "Model/Node.h"

namespace TrenchBroom {
    namespace Model {
        class Layer : public Node {
        private:
            String m_name;

            mutable vm::bbox3 m_logicalBounds;
            mutable vm::bbox3 m_physicalBounds;
            mutable bool m_boundsValid;
        public:
            Layer(const String& name, const vm::bbox3& worldBounds);

            void setName(const String& name);
        private: // implement Node interface
            const String& doGetName() const override;
            const vm::bbox3& doGetLogicalBounds() const override;
            const vm::bbox3& doGetPhysicalBounds() const override;

            Node* doClone(const vm::bbox3& worldBounds) const override;
            bool doCanAddChild(const Node* child) const override;
            bool doCanRemoveChild(const Node* child) const override;
            bool doRemoveIfEmpty() const override;
            bool doShouldAddToSpacialIndex() const override;
            void doNodePhysicalBoundsDidChange(const vm::bbox3& oldBounds) override;
            bool doSelectable() const override;

            void doPick(const vm::ray3& ray, PickResult& pickResult) const override;
            void doFindNodesContaining(const vm::vec3& point, NodeList& result) override;

            void doGenerateIssues(const IssueGenerator* generator, IssueList& issues) override;
            void doAccept(NodeVisitor& visitor) override;
            void doAccept(ConstNodeVisitor& visitor) const override;
        private:
            void invalidateBounds();
            void validateBounds() const;
        private: // implement Taggable interface
            void doAcceptTagVisitor(TagVisitor& visitor) override;
            void doAcceptTagVisitor(ConstTagVisitor& visitor) const override;
        private:
            deleteCopyAndMove(Layer)
        };
    }
}

#endif /* defined(TrenchBroom_Layer) */
