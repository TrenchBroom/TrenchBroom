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

#ifndef TrenchBroom_MatchSelectableNodes
#define TrenchBroom_MatchSelectableNodes

#include <stdio.h>

namespace TrenchBroom {
    namespace Model {
        class Brush;
        class EditorContext;
        class Entity;
        class Group;
        class Layer;
        class World;
        
        class MatchSelectableNodes {
        private:
            const Model::EditorContext& m_editorContext;
        public:
            MatchSelectableNodes(const Model::EditorContext& editorContext);
            
            bool operator()(const Model::World* world) const;
            bool operator()(const Model::Layer* layer) const;
            bool operator()(const Model::Group* group) const;
            bool operator()(const Model::Entity* entity) const;
            bool operator()(const Model::Brush* brush) const;
        };
    }
}

#endif /* defined(TrenchBroom_MatchSelectableNodes) */
