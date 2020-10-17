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

#ifndef TrenchBroom_ComputeNodeBoundsVisitor
#define TrenchBroom_ComputeNodeBoundsVisitor

#include "FloatType.h"
#include "Model/NodeVisitor.h"
#include "Model/Node.h"

#include <vecmath/bbox.h>

#include <vector>

namespace TrenchBroom {
    namespace Model {
        enum class BoundsType {
            /**
             * See Node::logicalBounds()
             */
            Logical,
            /**
             * See Node::physicalBounds()
             */
            Physical
        };

        class ComputeNodeBoundsVisitor : public ConstNodeVisitor {
        private:
            bool m_initialized;
            BoundsType m_boundsType;
            vm::bbox3 m_defaultBounds;
            vm::bbox3::builder m_builder;
        public:
            explicit ComputeNodeBoundsVisitor(BoundsType type, const vm::bbox3& defaultBounds = vm::bbox3());
            const vm::bbox3& bounds() const;
        private:
            void doVisit(const WorldNode* world) override;
            void doVisit(const LayerNode* layer) override;
            void doVisit(const GroupNode* group) override;
            void doVisit(const EntityNode* entity) override;
            void doVisit(const BrushNode* brush) override;
            void mergeWith(const vm::bbox3& bounds);
        };

        vm::bbox3 computeLogicalBounds(const std::vector<Node*>& nodes, const vm::bbox3& defaultBounds = vm::bbox3());

        template <typename I>
        vm::bbox3 computeLogicalBounds(I cur, I end, const vm::bbox3& defaultBounds = vm::bbox3()) {
            auto visitor = ComputeNodeBoundsVisitor(BoundsType::Logical, defaultBounds);
            Node::accept(cur, end, visitor);
            return visitor.bounds();
        }

        vm::bbox3 computePhysicalBounds(const std::vector<Node*>& nodes, const vm::bbox3& defaultBounds = vm::bbox3());

        template <typename I>
        vm::bbox3 computePhysicalBounds(I cur, I end, const vm::bbox3& defaultBounds = vm::bbox3()) {
            auto visitor = ComputeNodeBoundsVisitor(BoundsType::Physical, defaultBounds);
            Node::accept(cur, end, visitor);
            return visitor.bounds();
        }
    }
}


#endif /* defined(TrenchBroom_ComputeNodeBoundsVisitor) */
