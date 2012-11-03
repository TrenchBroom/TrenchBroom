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

#include "FlipObjectsCommand.h"

#include "Model/Brush.h"
#include "Model/Entity.h"

namespace TrenchBroom {
    namespace Controller {
        bool FlipObjectsCommand::performDo() {
            document().entitiesWillChange(m_entities);
            document().brushesWillChange(m_brushes);

            Model::EntityList::const_iterator entityIt, entityEnd;
            for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity& entity = **entityIt;
                entity.flip(m_axis, m_center, m_lockTextures);
            }
            
            Model::BrushList::const_iterator brushIt, brushEnd;
            for (brushIt = m_brushes.begin(), brushEnd = m_brushes.end(); brushIt != brushEnd; ++brushIt) {
                Model::Brush& brush = **brushIt;
                brush.flip(m_axis, m_center, m_lockTextures);
            }
            
            document().entitiesDidChange(m_entities);
            document().brushesDidChange(m_brushes);

            return true;
        }
        
        bool FlipObjectsCommand::performUndo() {
            return performDo();
        }

        FlipObjectsCommand* FlipObjectsCommand::flip(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes, Axis::Type axis, const Vec3f& center, bool lockTextures) {
            wxString commandName = Command::makeObjectActionName(wxT("Flip"), entities, brushes);
            return new FlipObjectsCommand(document, entities, brushes, commandName, axis, center, lockTextures);
        }
    }
}