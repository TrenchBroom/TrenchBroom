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

#include "MoveObjectsTool.h"

#include "Controller/Command.h"
#include "Controller/MoveObjectsCommand.h"
#include "Model/EditStateManager.h"
#include "Model/Map.h"
#include "Model/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        bool MoveObjectsTool::isApplicable(InputState& inputState, Vec3f& hitPoint) {
            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            
            if (entities.empty() && brushes.empty())
                return false;
            
            Model::ObjectHit* hit = static_cast<Model::ObjectHit*>(inputState.pickResult().first(Model::HitType::ObjectHit, false, m_filter));
            if (hit == NULL)
                return false;
            
            hitPoint = hit->hitPoint();
            return true;
        }
        
        wxString MoveObjectsTool::actionName() {
            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();

            return Command::makeObjectActionName(wxT("Move"), entities, brushes);
        }
        
        MoveTool::MoveResult MoveObjectsTool::performMove(const Vec3f& delta) {
            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            
            BBox bounds = editStateManager.bounds();
            bounds.translate(delta);
            if (!document().map().worldBounds().contains(bounds))
                return Deny;
            
            MoveObjectsCommand* command = MoveObjectsCommand::moveObjects(document(), entities, brushes, delta, document().textureLock());
            submitCommand(command);
            
            return Continue;
        }

        MoveObjectsTool::MoveObjectsTool(View::DocumentViewHolder& documentViewHolder, InputController& inputController) :
        MoveTool(documentViewHolder, inputController, true),
        m_filter(Model::SelectedFilter(view().filter())) {}
    }
}
