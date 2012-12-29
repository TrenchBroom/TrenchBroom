/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "RemoveObjectsCommand.h"

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/EntityDefinition.h"
#include "Model/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        bool RemoveObjectsCommand::performDo() {
            assert(!m_entities.empty() || !m_brushes.empty());

            m_removedEntities = m_entities;
            m_removedBrushes.clear();
            m_removedBrushParents.clear();
            
            Model::BrushList::iterator brushIt, brushEnd;
            for (brushIt = m_brushes.begin(), brushEnd = m_brushes.end(); brushIt != brushEnd; ++brushIt) {
                Model::Brush* brush = *brushIt;
                Model::Entity* entity = brush->entity();

                document().removeBrush(*brush);
                
                if (entity != NULL &&
                    !entity->worldspawn() &&
                    entity->brushes().empty() &&
                    std::find(m_removedEntities.begin(), m_removedEntities.end(), entity) == m_removedEntities.end()) {
                    m_removedEntities.push_back(entity);
                }
                m_removedBrushes.push_back(brush);
                m_removedBrushParents.insert(Model::BrushParentMapEntry(brush, entity));
            }
            
            Model::EntityList::iterator entityIt, entityEnd;
            for (entityIt = m_removedEntities.begin(), entityEnd = m_removedEntities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity* entity = *entityIt;
                document().removeEntity(*entity);
            }
            
            return true;
        }
        
        bool RemoveObjectsCommand::performUndo() {
            assert(!m_removedEntities.empty() || !m_removedBrushes.empty());
            
            Model::EntityList::iterator entityIt, entityEnd;
            for (entityIt = m_removedEntities.begin(), entityEnd = m_removedEntities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity* entity = *entityIt;
                document().addEntity(*entity);
            }
            
            Model::BrushParentMap::iterator brushIt, brushEnd;
            for (brushIt = m_removedBrushParents.begin(), brushEnd = m_removedBrushParents.end(); brushIt != brushEnd; ++brushIt) {
                Model::Brush* brush = brushIt->first;
                Model::Entity* entity = brushIt->second;
                document().addBrush(*entity, *brush);
            }

            return true;
        }
        
        RemoveObjectsCommand::RemoveObjectsCommand(Type type, Model::MapDocument& document, const wxString& name, const Model::EntityList& entities, const Model::BrushList& brushes) :
        DocumentCommand(type, document, true, name, true),
        m_entities(entities),
        m_brushes(brushes) {}
        
        RemoveObjectsCommand::~RemoveObjectsCommand() {
            if (state() == Done) {
                while (!m_removedEntities.empty()) delete m_removedEntities.back(), m_removedEntities.pop_back();
                while (!m_removedBrushes.empty()) delete m_removedBrushes.back(), m_removedBrushes.pop_back();
            } else {
                m_removedEntities.clear();
                m_removedBrushes.clear();
            }
            m_removedBrushParents.clear();
            m_entities.clear();
            m_brushes.clear();
        }

        RemoveObjectsCommand* RemoveObjectsCommand::removeObjects(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes) {
            assert(!entities.empty() || !brushes.empty());
            return new RemoveObjectsCommand(RemoveObjects, document, makeObjectActionName(wxT("Remove"), entities, brushes), entities, brushes);
        }
    }
}
