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

#ifndef MatchSelectedNodes_h
#define MatchSelectedNodes_h

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Group.h"

namespace TrenchBroom {
    namespace Model {
        class Brush;
        class Entity;
        class Group;
        class Layer;
        class World;
        
        template <bool MatchSelected>
        class MatchSelectedNodes {
        public:
            bool operator()(const Model::World* world) const   { return false; }
            bool operator()(const Model::Layer* layer) const   { return false; }
            bool operator()(const Model::Group* group) const   { return MatchSelected == group->selected(); }
            bool operator()(const Model::Entity* entity) const { return MatchSelected == entity->selected(); }
            bool operator()(const Model::Brush* brush) const   { return MatchSelected == brush->selected(); }
        };
    }
}

#endif /* MatchSelectedNodes_h */
