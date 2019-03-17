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

#include "AssortNodesVisitor.h"

namespace TrenchBroom {
    namespace Model {
        const LayerList& CollectLayersStrategy::layers() const { return m_layers; }
        void CollectLayersStrategy::addLayer(TrenchBroom::Model::Layer* layer) { m_layers.push_back(layer); }

        const LayerList& SkipLayersStrategy::layers() const { return EmptyLayerList; }
        void SkipLayersStrategy::addLayer(TrenchBroom::Model::Layer* layer) {}


        const GroupList& CollectGroupsStrategy::groups() const { return m_groups; }
        void CollectGroupsStrategy::addGroup(Group* group) { m_groups.push_back(group); }

        const GroupList& SkipGroupsStrategy::groups() const { return EmptyGroupList; }
        void SkipGroupsStrategy::addGroup(Group* group) {}


        const EntityList& CollectEntitiesStrategy::entities() const { return m_entities; }
        void CollectEntitiesStrategy::addEntity(Entity* entity) { m_entities.push_back(entity); }

        const EntityList& SkipEntitiesStrategy::entities() const { return EmptyEntityList; }
        void SkipEntitiesStrategy::addEntity(Entity* entity) {}


        const BrushList& CollectBrushesStrategy::brushes() const { return m_brushes; }
        void CollectBrushesStrategy::addBrush(Brush* brush) { m_brushes.push_back(brush); }


        const BrushList& SkipBrushesStrategy::brushes() const { return EmptyBrushList; }
        void SkipBrushesStrategy::addBrush(Brush* brush) {}
    }
}
