/*
 Copyright (C) 2021 Kristian Duske

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

#pragma once

#include "Model/BezierPatch.h"
#include "Model/HitType.h"
#include "Model/Node.h"
#include "Model/Object.h"

#include <iosfwd>
#include <optional>

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }

    namespace Model {
        class EntityNodeBase;

        struct PatchGrid {
            struct Point {
                vm::vec3 position;
                vm::vec2 texCoords;
                vm::vec3 normal;
            };

            size_t pointRowCount;
            size_t pointColumnCount;
            std::vector<Point> points;
            vm::bbox3 bounds;

            const Point& point(size_t row, size_t col) const;
            
            size_t quadRowCount() const;
            size_t quadColumnCount() const;
        };

        // public for testing
        std::vector<vm::vec3> computeGridNormals(const std::vector<BezierPatch::Point> patchGrid, const size_t pointRowCount, const size_t pointColumnCount);

        // public for testing
        PatchGrid makePatchGrid(const BezierPatch& patch, size_t subdivisionsPerSurface);

        bool operator==(const PatchGrid::Point& lhs, const PatchGrid::Point& rhs);
        bool operator!=(const PatchGrid::Point& lhs, const PatchGrid::Point& rhs);
        std::ostream& operator<<(std::ostream& str, const PatchGrid::Point& p);

        class PatchNode : public Node, public Object {
        public:
            static const HitType::Type PatchHitType;
        private:
            BezierPatch m_patch;
            PatchGrid m_grid;
        public:
            explicit PatchNode(BezierPatch patch);

            EntityNodeBase* entity();
            const EntityNodeBase* entity() const;

            const BezierPatch& patch() const;
            BezierPatch setPatch(BezierPatch patch);

            void setTexture(Assets::Texture* texture);

            const PatchGrid& grid() const;
        private: // implement Node interface
            const std::string& doGetName() const override;
            const vm::bbox3& doGetLogicalBounds() const override;
            const vm::bbox3& doGetPhysicalBounds() const override;

            FloatType doGetProjectedArea(vm::axis::type axis) const override;

            Node* doClone(const vm::bbox3& worldBounds) const override;

            bool doCanAddChild(const Node* child) const override;
            bool doCanRemoveChild(const Node* child) const override;
            bool doRemoveIfEmpty() const override;

            bool doShouldAddToSpacialIndex() const override;

            bool doSelectable() const override;

            void doPick(const vm::ray3& ray, PickResult& pickResult) override;
            void doFindNodesContaining(const vm::vec3& point, std::vector<Node*>& result) override;

            void doGenerateIssues(const IssueGenerator* generator, std::vector<Issue*>& issues) override;

            void doAccept(NodeVisitor& visitor) override;
            void doAccept(ConstNodeVisitor& visitor) const override;
        private: // implement Object interface
            Node* doGetContainer() override;
            LayerNode* doGetContainingLayer() override;
            GroupNode* doGetContainingGroup() override;
        private: // implement Taggable interface
            void doAcceptTagVisitor(TagVisitor& visitor) override;
            void doAcceptTagVisitor(ConstTagVisitor& visitor) const override;
        };
    }
}
