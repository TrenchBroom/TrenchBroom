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

#include "MoveObjectsCommand.h"

#include "Model/Brush.h"
#include "Model/EditStateManager.h"
#include "Model/Entity.h"
#include "Model/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        bool MoveObjectsCommand::translate(const Vec3f& delta) {
            document().entitiesWillChange(m_entities);
            document().brushesWillChange(m_brushes);
            
            Model::EntityList::const_iterator entityIt, entityEnd;
            for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity& entity = **entityIt;
                entity.translate(delta, m_lockTextures);
            }
            
            Model::BrushList::const_iterator brushIt, brushEnd;
            for (brushIt = m_brushes.begin(), brushEnd = m_brushes.end(); brushIt != brushEnd; ++brushIt) {
                Model::Brush& brush = **brushIt;
                brush.translate(delta, m_lockTextures);
            }

            document().entitiesDidChange(m_entities);
            document().brushesDidChange(m_brushes);
            
            return true;
        }

        bool MoveObjectsCommand::performDo() {
            return translate(m_delta);
        }
        
        bool MoveObjectsCommand::performUndo() {
            return translate(-m_delta);
        }

        MoveObjectsCommand::MoveObjectsCommand(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes, const wxString& name, const Vec3f& delta, bool lockTextures) :
        DocumentCommand(MoveObjects, document, true, name, true),
        m_entities(entities),
        m_brushes(brushes),
        m_delta(delta),
        m_lockTextures(lockTextures) {}
        
        MoveObjectsCommand* MoveObjectsCommand::moveObjects(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes, const Vec3f& delta, bool lockTextures) {
            wxString name = Controller::Command::makeObjectActionName("Move", entities, brushes);
            return new MoveObjectsCommand(document, entities, brushes, name, delta, lockTextures);
        }

        MoveObjectsCommand* MoveObjectsCommand::moveEntity(Model::MapDocument& document, Model::Entity& entity, const Vec3f& delta, bool lockTextures) {
            Model::EntityList entities;
            entities.push_back(&entity);
            return moveObjects(document, entities, Model::EmptyBrushList, delta, lockTextures);
        }
    }
}
