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

#include "AssortNodesVisitor.h"

namespace TrenchBroom {
    namespace Model {
        const LayerList& AssortNodesVisitor::layers() const {
            return m_layers;
        }
        
        const GroupList& AssortNodesVisitor::groups() const {
            return m_groups;
        }
        
        const EntityList& AssortNodesVisitor::entities() const {
            return m_entities;
        }
        
        const BrushList& AssortNodesVisitor::brushes() const {
            return m_brushes;
        }

        void AssortNodesVisitor::doVisit(World* world) {}
        
        void AssortNodesVisitor::doVisit(Layer* layer) {
            m_layers.push_back(layer);
        }
        
        void AssortNodesVisitor::doVisit(Group* group) {
            m_groups.push_back(group);
        }
        
        void AssortNodesVisitor::doVisit(Entity* entity) {
            m_entities.push_back(entity);
        }
        
        void AssortNodesVisitor::doVisit(Brush* brush) {
            m_brushes.push_back(brush);
        }
    }
}
