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

#ifndef TrenchBroom_FindGroupVisitor
#define TrenchBroom_FindGroupVisitor

#include "Model/ModelTypes.h"
#include "Model/NodeVisitor.h"

namespace TrenchBroom {
    namespace Model {
        class FindGroupVisitor : public NodeVisitor, public NodeQuery<Group*> {
        private:
            bool m_findTopGroup;
        public:
            FindGroupVisitor(bool findTopGroup);
        private:
            void doVisit(World* world);
            void doVisit(Layer* layer);
            void doVisit(Group* group);
            void doVisit(Entity* entity);
            void doVisit(Brush* brush);
        };
        
        Model::Group* findGroup(Model::Node* node);
        Model::Group* findTopGroup(Model::Node* node);
    }
}

#endif /* defined(TrenchBroom_FindGroupVisitor) */
