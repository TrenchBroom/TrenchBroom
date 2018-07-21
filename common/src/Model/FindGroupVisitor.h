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

#ifndef TrenchBroom_FindGroupVisitor
#define TrenchBroom_FindGroupVisitor

#include "Model/ModelTypes.h"
#include "Model/NodeVisitor.h"

namespace TrenchBroom {
    namespace Model {
        class FindGroupVisitor : public NodeVisitor, public NodeQuery<Group*> {
        public:
            virtual ~FindGroupVisitor() override;
        private:
            void doVisit(World* world) override;
            void doVisit(Layer* layer) override;
            void doVisit(Group* group) override;
            void doVisit(Entity* entity) override;
            void doVisit(Brush* brush) override;

            virtual bool shouldContinue(const Group* group) const = 0;
        };

        class FindContainingGroupVisitor : public FindGroupVisitor {
        private:
            bool shouldContinue(const Group* group) const override;
        };

        class FindTopGroupVisitor : public FindGroupVisitor {
        private:
            bool shouldContinue(const Group* group) const override;
        };

        class FindTopGroupWithOpenParentVisitor : public FindGroupVisitor {
        private:
            bool shouldContinue(const Group* group) const override;
        };

        Model::Group* findContainingGroup(Model::Node* node);
        Model::Group* findTopContainingGroup(Model::Node* node);
        Model::Group* findContainingGroupWithOpenParent(Model::Node* node);
    }
}

#endif /* defined(TrenchBroom_FindGroupVisitor) */
