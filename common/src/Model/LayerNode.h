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

#ifndef TrenchBroom_LayerNode
#define TrenchBroom_LayerNode

#include "Color.h"
#include "FloatType.h"
#include "Macros.h"
#include "Model/AttributableNode.h"

#include <vecmath/bbox.h>

#include <string>
#include <vector>
#include <optional>

namespace TrenchBroom {
    namespace Model {
        class LayerNode : public AttributableNode {
        private:
            mutable vm::bbox3 m_logicalBounds;
            mutable vm::bbox3 m_physicalBounds;
            mutable bool m_boundsValid;
        public:
            LayerNode(const std::string& name);

            void setName(const std::string& name);

            bool isDefaultLayer() const;

            static int invalidSortIndex();
            static int defaultLayerSortIndex();

            int sortIndex() const;
            void setSortIndex(int index);

            /**
             * Stable sort the given vector using `sortIndex()` as the sort key.
             */
            static void sortLayers(std::vector<LayerNode*>& layers);

            std::optional<Color> layerColor() const;
            void setLayerColor(const Color& color);

            bool omitFromExport() const;
            void setOmitFromExport(bool omitFromExport);
        private: // implement Node interface
            const std::string& doGetName() const override;
            const vm::bbox3& doGetLogicalBounds() const override;
            const vm::bbox3& doGetPhysicalBounds() const override;

            Node* doClone(const vm::bbox3& worldBounds) const override;
            bool doCanAddChild(const Node* child) const override;
            bool doCanRemoveChild(const Node* child) const override;
            bool doRemoveIfEmpty() const override;
            bool doShouldAddToSpacialIndex() const override;
            void doNodePhysicalBoundsDidChange() override;
            bool doSelectable() const override;

            void doPick(const vm::ray3& ray, PickResult& pickResult) override;
            void doFindNodesContaining(const vm::vec3& point, std::vector<Node*>& result) override;

            void doGenerateIssues(const IssueGenerator* generator, std::vector<Issue*>& issues) override;
            void doAccept(NodeVisitor& visitor) override;
            void doAccept(ConstNodeVisitor& visitor) const override;
        private: // implement AttributableNode interface
            void doAttributesDidChange(const vm::bbox3& oldBounds) override;
            vm::vec3 doGetLinkSourceAnchor() const override;
            vm::vec3 doGetLinkTargetAnchor() const override;
        private:
            void invalidateBounds();
            void validateBounds() const;
        private: // implement Taggable interface
            void doAcceptTagVisitor(TagVisitor& visitor) override;
            void doAcceptTagVisitor(ConstTagVisitor& visitor) const override;
        private:
            deleteCopyAndMove(LayerNode)
        };
    }
}

#endif /* defined(TrenchBroom_LayerNode) */
