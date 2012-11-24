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
            m_hasAddedBrushes = !m_addedBrushes.empty();
            
            Model::EntityList::iterator entityIt, entityEnd;
            for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity& entity = **entityIt;
                const Model::BrushList& entityBrushes = entity.brushes();
                if (entity.worldspawn()) {
                    m_addedBrushes.insert(m_addedBrushes.begin(), entityBrushes.begin(), entityBrushes.end());
                } else {
                    document().addEntity(entity);
                    m_addedEntities.push_back(&entity);
                }
                m_hasAddedBrushes |= !entityBrushes.empty();
            }
            
            Model::Entity& worldspawn = *document().worldspawn(true);
            Model::BrushList::iterator brushIt, brushEnd;
            for (brushIt = m_addedBrushes.begin(), brushEnd = m_addedBrushes.end(); brushIt != brushEnd; ++brushIt) {
                Model::Brush& brush = **brushIt;
                document().addBrush(worldspawn, brush);
            }
            
            return true;
        }
        
        bool AddObjectsCommand::performUndo() {
            Model::BrushList::iterator brushIt, brushEnd;
            for (brushIt = m_addedBrushes.begin(), brushEnd = m_addedBrushes.end(); brushIt != brushEnd; ++brushIt) {
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
        DocumentCommand(AddObjects, document, true, name, true),
        m_entities(entities),
        m_brushes(brushes),
        m_hasAddedBrushes(false) {}

        AddObjectsCommand* AddObjectsCommand::addObjects(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes){
            assert(!entities.empty() || !brushes.empty());
            return new AddObjectsCommand(document, makeObjectActionName(wxT("Add"), entities, brushes), entities, brushes);
        };

        AddObjectsCommand* AddObjectsCommand::addEntities(Model::MapDocument& document, const Model::EntityList& entities) {
            return new AddObjectsCommand(document, makeObjectActionName(wxT("Add"), entities, Model::EmptyBrushList), entities, Model::EmptyBrushList);
        }
        
        AddObjectsCommand* AddObjectsCommand::addBrushes(Model::MapDocument& document, const Model::BrushList& brushes) {
            return new AddObjectsCommand(document, makeObjectActionName(wxT("Add"), Model::EmptyEntityList, brushes), Model::EmptyEntityList, brushes);
        }

        AddObjectsCommand* AddObjectsCommand::addEntity(Model::MapDocument& document, Model::Entity& entity) {
            Model::EntityList entities;
            entities.push_back(&entity);
            return new AddObjectsCommand(document, wxT("Add Entity"), entities, Model::EmptyBrushList);
        }

        AddObjectsCommand* AddObjectsCommand::addBrush(Model::MapDocument& document, Model::Brush& brush) {
            Model::BrushList brushes;
            brushes.push_back(&brush);
            return new AddObjectsCommand(document, wxT("Add Brush"), Model::EmptyEntityList, brushes);
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