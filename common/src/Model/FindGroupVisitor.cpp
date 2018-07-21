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

#include "FindGroupVisitor.h"

#include "Model/Group.h"
#include "Model/Node.h"

namespace TrenchBroom {
    namespace Model {
        FindGroupVisitor::~FindGroupVisitor() {}

        void FindGroupVisitor::doVisit(World* world) {}
        void FindGroupVisitor::doVisit(Layer* layer) {}
        
        void FindGroupVisitor::doVisit(Group* group) {
            setResult(group);
            if (!shouldContinue(group)) {
                cancel();
            }
        }
        
        void FindGroupVisitor::doVisit(Entity* entity) {}
        void FindGroupVisitor::doVisit(Brush* brush) {}

        bool FindContainingGroupVisitor::shouldContinue(const Group* group) const {
            return false;
        }

        bool FindTopGroupVisitor::shouldContinue(const Group* group) const {
            return true;
        }

        bool FindTopGroupWithOpenParentVisitor::shouldContinue(const Group* group) const {
            const Group* container = group->group();
            return container != nullptr && !container->opened();
        }

        Model::Group* findContainingGroup(Model::Node* node) {
            FindContainingGroupVisitor visitor;
            node->escalate(visitor);
            if (!visitor.hasResult()) {
                return nullptr;
            }
            return visitor.result();
        }

        Model::Group* findTopContainingGroup(Model::Node* node) {
            FindTopGroupVisitor visitor;
            node->escalate(visitor);
            if (!visitor.hasResult()) {
                return nullptr;
            }
            return visitor.result();
        }

        Model::Group* findContainingGroupWithOpenParent(Model::Node* node) {
            FindTopGroupWithOpenParentVisitor visitor;
            node->escalate(visitor);
            if (!visitor.hasResult()) {
                return nullptr;
            }
            return visitor.result();
        }
    }
}
