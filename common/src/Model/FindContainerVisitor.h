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

#ifndef TrenchBroom_FindContainerVisitor
#define TrenchBroom_FindContainerVisitor

#include "Model/NodeVisitor.h"

namespace TrenchBroom {
    namespace Model {
        class FindContainerVisitor : public NodeVisitor, public NodeQuery<Node*> {
        private:
            void doVisit(WorldNode* world) override;
            void doVisit(LayerNode* layer) override;
            void doVisit(GroupNode* group) override;
            void doVisit(Entity* entity) override;
            void doVisit(BrushNode* brush) override;
        };
    }
}

#endif /* defined(TrenchBroom_FindContainerVisitor) */
