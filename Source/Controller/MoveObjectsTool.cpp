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

#include "Controller/AddObjectsCommand.h"
#include "Controller/ChangeEditStateCommand.h"
#include "Controller/Command.h"
#include "Controller/MoveObjectsCommand.h"
#include "Model/EditStateManager.h"
#include "Model/Map.h"
#include "Model/MapDocument.h"
#include "View/EditorFrame.h"
#include "View/FlashSelectionAnimation.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        bool MoveObjectsTool::isApplicable(InputState& inputState, Vec3f& hitPoint) {
            if (inputState.mouseButtons() == MouseButtons::MBLeft &&
                (inputState.modifierKeys() == ModifierKeys::MKNone ||
                 inputState.modifierKeys() == ModifierKeys::MKAlt ||
                 inputState.modifierKeys() == ModifierKeys::MKCtrlCmd ||
                 inputState.modifierKeys() == (ModifierKeys::MKCtrlCmd | ModifierKeys::MKAlt))) {
                    
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
            
            return false;
        }
        
        wxString MoveObjectsTool::actionName(InputState& inputState) {
            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();

            if ((inputState.modifierKeys() & ModifierKeys::MKCtrlCmd))
                return Command::makeObjectActionName(wxT("Duplicate"), entities, brushes);
            return Command::makeObjectActionName(wxT("Move"), entities, brushes);
        }
        
        void MoveObjectsTool::startDrag(InputState& inputState) {
            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::EntityList& entities = editStateManager.selectedEntities();
            const Model::BrushList& brushes = editStateManager.selectedBrushes();

            if ((inputState.modifierKeys() & ModifierKeys::MKCtrlCmd)) {
                m_mode = MMDuplicate;
                beginCommandGroup(Command::makeObjectActionName(wxT("Duplicate"), entities, brushes));
                duplicateObjects();
            } else {
                m_mode = MMMove;
                beginCommandGroup(Command::makeObjectActionName(wxT("Move"), entities, brushes));
            }
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

        void MoveObjectsTool::endDrag(InputState& inputState) {
            endCommandGroup();
        }

        void MoveObjectsTool::duplicateObjects() {
            typedef std::map<Model::Entity*, Model::Entity*> EntityMap;
            
            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::EntityList& originalEntities = editStateManager.selectedEntities();
            const Model::BrushList& originalBrushes = editStateManager.selectedBrushes();
            
            Model::EntityList newPointEntities;
            Model::EntityList newBrushEntities;
            Model::BrushList newWorldBrushes;
            Model::BrushList newEntityBrushes;
            EntityMap brushEntities;
            
            Model::EntityList::const_iterator entityIt, entityEnd;
            for (entityIt = originalEntities.begin(), entityEnd = originalEntities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity& entity = **entityIt;
                assert(entity.definition() == NULL || entity.definition()->type() == Model::EntityDefinition::PointEntity);
                assert(!entity.worldspawn());
                
                Model::Entity* newPointEntity = new Model::Entity(document().map().worldBounds(), entity);
                newPointEntities.push_back(newPointEntity);
            }
            
            Model::BrushList::const_iterator brushIt, brushEnd;
            for (brushIt = originalBrushes.begin(), brushEnd = originalBrushes.end(); brushIt != brushEnd; ++brushIt) {
                Model::Brush& brush = **brushIt;
                Model::Entity& entity = *brush.entity();
                
                Model::Brush* newBrush = new Model::Brush(document().map().worldBounds(), document().map().forceIntegerFacePoints(), brush);
                if (entity.worldspawn()) {
                    newWorldBrushes.push_back(newBrush);
                } else {
                    Model::Entity* newEntity = NULL;
                    EntityMap::iterator newEntityIt = brushEntities.find(&entity);
                    if (newEntityIt == brushEntities.end()) {
                        newEntity = new Model::Entity(document().map().worldBounds(), entity);
                        newBrushEntities.push_back(newEntity);
                        brushEntities[&entity] = newEntity;
                    } else {
                        newEntity = newEntityIt->second;
                    }
                    newEntity->addBrush(*newBrush);
                    newEntityBrushes.push_back(newBrush);
                }
            }
            
            Model::EntityList allNewEntities = Utility::concatenate(newPointEntities, newBrushEntities);
            Model::BrushList allNewBrushes = Utility::concatenate(newWorldBrushes, newEntityBrushes);
            
            Controller::AddObjectsCommand* addObjectsCommand = Controller::AddObjectsCommand::addObjects(document(), allNewEntities, newWorldBrushes);
            Controller::ChangeEditStateCommand* changeEditStateCommand = Controller::ChangeEditStateCommand::replace(document(), newPointEntities, allNewBrushes);
            
            submitCommand(addObjectsCommand);
            submitCommand(changeEditStateCommand);
        }

        MoveObjectsTool::MoveObjectsTool(View::DocumentViewHolder& documentViewHolder, InputController& inputController) :
        MoveTool(documentViewHolder, inputController, true),
        m_filter(Model::SelectedFilter(view().filter())) {}
    }
}
