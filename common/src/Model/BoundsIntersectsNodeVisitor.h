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

#ifndef TrenchBroom_BoundsIntersectsNodeVisitor
#define TrenchBroom_BoundsIntersectsNodeVisitor

#include "FloatType.h"
#include "Model/NodeVisitor.h"

#include <vecmath/bbox.h>

namespace TrenchBroom {
    namespace Model {
        class BoundsIntersectsNodeVisitor : public ConstNodeVisitor, public NodeQuery<bool> {
        private:
            const vm::bbox3 m_bounds;
        public:
            explicit BoundsIntersectsNodeVisitor(const vm::bbox3& bounds);
        private:
            void doVisit(const World* world) override;
            void doVisit(const Layer* layer) override;
            void doVisit(const Group* group) override;
            void doVisit(const Entity* entity) override;
            void doVisit(const BrushNode* brush) override;
        };
    }
}

#endif /* defined(TrenchBroom_BoundsIntersectsNodeVisitor) */
