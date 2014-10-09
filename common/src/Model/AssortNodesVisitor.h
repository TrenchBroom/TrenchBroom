/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef __TrenchBroom__AssortNodesVisitor__
#define __TrenchBroom__AssortNodesVisitor__

#include "Model/ModelTypes.h"
#include "Model/NodeVisitor.h"

namespace TrenchBroom {
    namespace Model {
        class AssortNodesVisitor : public NodeVisitor {
        private:
            LayerList m_layers;
            GroupList m_groups;
            EntityList m_entities;
            BrushList m_brushes;
        public:
            const LayerList& layers() const;
            const GroupList& groups() const;
            const EntityList& entities() const;
            const BrushList& brushes() const;
        private:
            void doVisit(World* world);
            void doVisit(Layer* layer);
            void doVisit(Group* group);
            void doVisit(Entity* entity);
            void doVisit(Brush* brush);
        };
    }
}

#endif /* defined(__TrenchBroom__AssortNodesVisitor__) */
