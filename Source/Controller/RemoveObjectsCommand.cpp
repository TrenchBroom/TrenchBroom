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
#include "Utility/List.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        bool RemoveObjectsCommand::performDo() {
            assert(!m_entities.empty() || !m_brushes.empty());

            clearUndoInformation();
            removeBrushes();
            removeEntities();
            
            return true;
        }
        
        void RemoveObjectsCommand::removeBrushes() {
            Model::BrushList::const_iterator brushIt, brushEnd;
            for (brushIt = m_brushes.begin(), brushEnd = m_brushes.end(); brushIt != brushEnd; ++brushIt) {
                Model::Brush* brush = *brushIt;
                Model::Entity* entity = brush->entity();
                
                document().removeBrush(*brush);
                
                if (entity != NULL &&
                    !entity->worldspawn() &&
                    entity->brushes().empty() &&
                    std::find(m_entities.begin(), m_entities.end(), entity) == m_entities.end()) {
                    document().removeEntity(*entity);
                    m_removedEntities.push_back(entity);
                }
                
                m_removedBrushes.push_back(brush);
                m_removedBrushParents.insert(Model::BrushParentMapEntry(brush, entity));
            }
        }

        void RemoveObjectsCommand::removeEntities() {
            Model::EntityList::const_iterator entityIt, entityEnd;
            for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity* entity = *entityIt;
                document().removeEntity(*entity);
                m_removedEntities.push_back(entity);
            }
        }

        bool RemoveObjectsCommand::performUndo() {
            assert(!m_removedEntities.empty() || !m_removedBrushes.empty());

            restoreEntities();
            restoreBrushes();
            clearUndoInformation();

            return true;
        }
        
        void RemoveObjectsCommand::restoreEntities() {
            Model::EntityList::iterator entityIt, entityEnd;
            for (entityIt = m_removedEntities.begin(), entityEnd = m_removedEntities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity* entity = *entityIt;
                document().addEntity(*entity);
            }
        }
        
        void RemoveObjectsCommand::restoreBrushes() {
            Model::BrushParentMap::iterator brushIt, brushEnd;
            for (brushIt = m_removedBrushParents.begin(), brushEnd = m_removedBrushParents.end(); brushIt != brushEnd; ++brushIt) {
                Model::Brush* brush = brushIt->first;
                Model::Entity* entity = brushIt->second;
                document().addBrush(*entity, *brush);
            }
        }

        void RemoveObjectsCommand::clearUndoInformation() {
            m_removedEntities.clear();
            m_removedBrushes.clear();
            m_removedBrushParents.clear();
        }

        RemoveObjectsCommand::RemoveObjectsCommand(Type type, Model::MapDocument& document, const wxString& name, const Model::EntityList& entities, const Model::BrushList& brushes) :
        DocumentCommand(type, document, true, name, true),
        m_entities(entities),
        m_brushes(brushes) {}
        
        RemoveObjectsCommand::~RemoveObjectsCommand() {
            clearUndoInformation();
            Utility::deleteAll(m_removedEntities);
            Utility::deleteAll(m_removedBrushes);
        }

        RemoveObjectsCommand* RemoveObjectsCommand::removeObjects(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes) {
            assert(!entities.empty() || !brushes.empty());
            return new RemoveObjectsCommand(RemoveObjects, document, makeObjectActionName(wxT("Remove"), entities, brushes), entities, brushes);
        }

        RemoveObjectsCommand* RemoveObjectsCommand::removeBrush(Model::MapDocument& document, Model::Brush& brush) {
            Model::BrushList brushList;
            brushList.push_back(&brush);
            return removeObjects(document, Model::EmptyEntityList, brushList);
        }
        
        RemoveObjectsCommand* RemoveObjectsCommand::removeEntities(Model::MapDocument& document, const Model::EntityList& entities) {
            return removeObjects(document, entities, Model::EmptyBrushList);
        }
    }
}
