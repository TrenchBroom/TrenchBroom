/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "AssortNodesVisitor.h"

namespace TrenchBroom {
    namespace Model {
        const LayerArray& CollectLayersStrategy::layers() const { return m_layers; }
        void CollectLayersStrategy::addLayer(TrenchBroom::Model::Layer* layer) { m_layers.push_back(layer); }

        const LayerArray& SkipLayersStrategy::layers() const { return EmptyLayerArray; }
        void SkipLayersStrategy::addLayer(TrenchBroom::Model::Layer* layer) {}
        

        const GroupArray& CollectGroupsStrategy::groups() const { return m_groups; }
        void CollectGroupsStrategy::addGroup(Group* group) { m_groups.push_back(group); }
    
        const GroupArray& SkipGroupsStrategy::groups() const { return EmptyGroupArray; }
        void SkipGroupsStrategy::addGroup(Group* group) {}

        
        const EntityArray& CollectEntitiesStrategy::entities() const { return m_entities; }
        void CollectEntitiesStrategy::addEntity(Entity* entity) { m_entities.push_back(entity); }
        
        const EntityArray& SkipEntitiesStrategy::entities() const { return EmptyEntityArray; }
        void SkipEntitiesStrategy::addEntity(Entity* entity) {}

        
        const BrushArray& CollectBrushesStrategy::brushes() const { return m_brushes; }
        void CollectBrushesStrategy::addBrush(Brush* brush) { m_brushes.push_back(brush); }


        const BrushArray& SkipBrushesStrategy::brushes() const { return EmptyBrushArray; }
        void SkipBrushesStrategy::addBrush(Brush* brush) {}
    }
}
