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

#include "AddObjectsCommand.h"

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        bool AddObjectsCommand::performDo() {
            m_addedEntities.clear();
            m_addedBrushes = m_brushes;
            
            Model::EntityList::iterator entityIt, entityEnd;
            for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity& entity = **entityIt;
                const Model::BrushList& entityBrushes = entity.brushes();

                if (!entity.worldspawn() || document().worldspawn(false) == NULL) {
                    document().addEntity(entity);
                    m_addedEntities.push_back(&entity);
                } else {
                    Model::Entity& worldspawn = *document().worldspawn(true);
                    Model::BrushList::const_iterator brushIt, brushEnd;
                    for (brushIt = entityBrushes.begin(), brushEnd = entityBrushes.end(); brushIt != brushEnd; ++brushIt) {
                        Model::Brush& brush = **brushIt;
                        document().addBrush(worldspawn, brush);
                    }
                }
                m_addedBrushes.insert(m_addedBrushes.end(), entityBrushes.begin(), entityBrushes.end());
            }
            
            Model::Entity& worldspawn = *document().worldspawn(true);
            Model::BrushList::iterator brushIt, brushEnd;
            for (brushIt = m_brushes.begin(), brushEnd = m_brushes.end(); brushIt != brushEnd; ++brushIt) {
                Model::Brush& brush = **brushIt;
                document().addBrush(worldspawn, brush);
            }
            
            return true;
        }
        
        bool AddObjectsCommand::performUndo() {
            Model::BrushList::iterator brushIt, brushEnd;
            for (brushIt = m_brushes.begin(), brushEnd = m_brushes.end(); brushIt != brushEnd; ++brushIt) {
                Model::Brush& brush = **brushIt;
                document().removeBrush(brush);
            }

            Model::EntityList::iterator entityIt, entityEnd;
            for (entityIt = m_addedEntities.begin(), entityEnd = m_addedEntities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity& entity = **entityIt;
                document().removeEntity(entity);
            }
            return true;
        }
        
        AddObjectsCommand::AddObjectsCommand(Model::MapDocument& document, const wxString& name, const Model::EntityList& entities, const Model::BrushList& brushes) :
        DocumentCommand(AddObjects, document, true, name),
        m_entities(entities),
        m_brushes(brushes) {}

        AddObjectsCommand* AddObjectsCommand::addObjects(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes){
            assert(!entities.empty() || !brushes.empty());
            return new AddObjectsCommand(document, makeObjectActionName(wxT("Add"), entities, brushes), entities, brushes);
        };

        AddObjectsCommand* AddObjectsCommand::addEntity(Model::MapDocument& document, Model::Entity& entity) {
            Model::EntityList entities;
            entities.push_back(&entity);
            return new AddObjectsCommand(document, wxT("Add Entity"), entities, Model::EmptyBrushList);
        }

        AddObjectsCommand::~AddObjectsCommand() {
            if (state() == Undone) {
                while (!m_brushes.empty()) delete m_brushes.back(), m_brushes.pop_back();
                while (!m_entities.empty()) delete m_entities.back(), m_entities.pop_back();
            } else {
                m_entities.clear();
                m_brushes.clear();
            }
        }
    }
}