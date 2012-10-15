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
            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            
            Model::EntityList::const_iterator entityIt, entityEnd;
            for (entityIt = entities.begin(), entityEnd = entities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity& entity = **entityIt;
                entity.translate(delta, m_lockTextures);
                document().updateEntity(entity);
            }
            
            Model::BrushList::const_iterator brushIt, brushEnd;
            for (brushIt = brushes.begin(), brushEnd = brushes.end(); brushIt != brushEnd; ++brushIt) {
                Model::Brush& brush = **brushIt;
                brush.translate(delta, m_lockTextures);
                document().updateBrush(brush);
            }

            return true;
        }

        bool MoveObjectsCommand::performDo() {
            return translate(m_delta);
        }
        
        bool MoveObjectsCommand::performUndo() {
            return translate(-1.0f * m_delta);
        }

        MoveObjectsCommand::MoveObjectsCommand(Model::MapDocument& document, const wxString& name, const Vec3f& delta, bool lockTextures) :
        DocumentCommand(MoveObjects, document, true, name),
        m_delta(delta),
        m_lockTextures(lockTextures) {}
        
        MoveObjectsCommand* MoveObjectsCommand::moveObjects(Model::MapDocument& document, const wxString& name, const Vec3f& delta, bool lockTextures) {
            return new MoveObjectsCommand(document, name, delta, lockTextures);
        }
    }
}