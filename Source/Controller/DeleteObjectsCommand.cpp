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

#include "DeleteObjectsCommand.h"

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/EntityDefinition.h"
#include "Model/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        bool DeleteObjectsCommand::performDo() {
            assert(!m_entities.empty() || !m_brushes.empty());

            m_deletedEntities = m_entities;
            m_deletedBrushes.clear();
            
            Model::BrushList::iterator brushIt, brushEnd;
            for (brushIt = m_brushes.begin(), brushEnd = m_brushes.end(); brushIt != brushEnd; ++brushIt) {
                Model::Brush* brush = *brushIt;
                Model::Entity* entity = brush->entity();

                document().removeBrush(*brush);
                
                if (entity != NULL && entity->brushes().empty() && std::find(m_deletedEntities.begin(), m_deletedEntities.end(), entity) == m_deletedEntities.end())
                    m_deletedEntities.push_back(entity);
                m_deletedBrushes.insert(Model::BrushParentMapEntry(brush, entity));
            }
            
            Model::EntityList::iterator entityIt, entityEnd;
            for (entityIt = m_deletedEntities.begin(), entityEnd = m_deletedEntities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity* entity = *entityIt;
                document().removeEntity(*entity);
            }
            
            document().UpdateAllViews(NULL, this);
            
            return true;
        }
        
        bool DeleteObjectsCommand::performUndo() {
            assert(!m_deletedEntities.empty() || !m_deletedBrushes.empty());
            
            Model::EntityList selectEntities;
            
            Model::EntityList::iterator entityIt, entityEnd;
            for (entityIt = m_deletedEntities.begin(), entityEnd = m_deletedEntities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity* entity = *entityIt;
                document().addEntity(*entity);
                
                if (entity->selectable())
                    selectEntities.push_back(entity);
            }
            
            Model::BrushParentMap::iterator brushIt, brushEnd;
            for (brushIt = m_deletedBrushes.begin(), brushEnd = m_deletedBrushes.end(); brushIt != brushEnd; ++brushIt) {
                Model::Brush* brush = brushIt->first;
                Model::Entity* entity = brushIt->second;
                document().addBrush(*entity, *brush);
            }
            
            document().UpdateAllViews(NULL, this);

            return true;
        }
        
        DeleteObjectsCommand::DeleteObjectsCommand(Type type, Model::MapDocument& document, const wxString& name, const Model::EntityList& entities, const Model::BrushList& brushes) :
        DocumentCommand(type, document, true, name),
        m_entities(entities),
        m_brushes(brushes) {}
        
        DeleteObjectsCommand* DeleteObjectsCommand::deleteObjects(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes) {
            wxString name;
            if (entities.empty()) {
                if (brushes.empty())
                    name = wxT("Delete Objects");
                else
                    name = brushes.size() == 1 ? wxT("Delete Brush") : wxT("Delete Brushes");
            } else if (brushes.empty()) {
                name = entities.size() == 1 ? wxT("Delete Entity") : wxT("Delete Entities");
            } else {
                name = entities.size() + brushes.size() == 1 ? wxT("Delete object") : wxT("Delete objects");
            }
            
            return new DeleteObjectsCommand(DeleteObjects, document, name, entities, brushes);
        }
    }
}