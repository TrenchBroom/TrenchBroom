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

#include "FindGroupVisitor.h"

#include "Model/Node.h"

namespace TrenchBroom {
    namespace Model {
        FindGroupVisitor::FindGroupVisitor(const bool findTopGroup) :
        m_findTopGroup(findTopGroup) {}

        void FindGroupVisitor::doVisit(World* world) {}
        void FindGroupVisitor::doVisit(Layer* layer) {}
        
        void FindGroupVisitor::doVisit(Group* group) {
            setResult(group);
            if (!m_findTopGroup)
                cancel();
        }
        
        void FindGroupVisitor::doVisit(Entity* entity) {}
        void FindGroupVisitor::doVisit(Brush* brush) {}

        Model::Group* findGroup(Model::Node* node) {
            FindGroupVisitor visitor(false);
            node->escalate(visitor);
            if (!visitor.hasResult())
                return NULL;
            return visitor.result();
        }

        Model::Group* findTopGroup(Model::Node* node) {
            FindGroupVisitor visitor(true);
            node->escalate(visitor);
            if (!visitor.hasResult())
                return NULL;
            return visitor.result();
        }
    }
}
