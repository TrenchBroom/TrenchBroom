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

#ifndef TrenchBroom_ControllerUtils_h
#define TrenchBroom_ControllerUtils_h

#include "Controller/AddObjectsCommand.h"
#include "Controller/ChangeEditStateCommand.h"
#include "Utility/CommandProcessor.h"
#include "Model/Brush.h"
#include "Model/EditStateManager.h"
#include "Model/Entity.h"
#include "Model/EntityDefinition.h"
#include "Model/Map.h"
#include "Model/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        inline void duplicateObjects(Model::MapDocument& document) {
            typedef std::map<Model::Entity*, Model::Entity*> EntityMap;
            
            Model::EditStateManager& editStateManager = document.editStateManager();
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
                
                Model::Entity* newPointEntity = new Model::Entity(document.map().worldBounds(), entity);
                newPointEntities.push_back(newPointEntity);
            }
            
            Model::BrushList::const_iterator brushIt, brushEnd;
            for (brushIt = originalBrushes.begin(), brushEnd = originalBrushes.end(); brushIt != brushEnd; ++brushIt) {
                Model::Brush& brush = **brushIt;
                Model::Entity& entity = *brush.entity();
                
                Model::Brush* newBrush = new Model::Brush(document.map().worldBounds(), document.map().forceIntegerFacePoints(), brush);
                if (entity.worldspawn()) {
                    newWorldBrushes.push_back(newBrush);
                } else {
                    Model::Entity* newEntity = NULL;
                    EntityMap::iterator newEntityIt = brushEntities.find(&entity);
                    if (newEntityIt == brushEntities.end()) {
                        newEntity = new Model::Entity(document.map().worldBounds(), entity);
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

            Controller::AddObjectsCommand* addObjectsCommand = Controller::AddObjectsCommand::addObjects(document, allNewEntities, newWorldBrushes);
            Controller::ChangeEditStateCommand* changeEditStateCommand = Controller::ChangeEditStateCommand::replace(document, newPointEntities, allNewBrushes);
            
            wxCommandProcessor* commandProcessor = document.GetCommandProcessor();
            CommandProcessor::BeginGroup(commandProcessor, Controller::Command::makeObjectActionName(wxT("Duplicate"), originalEntities, originalBrushes));
            commandProcessor->Submit(addObjectsCommand);
            commandProcessor->Submit(changeEditStateCommand);
            CommandProcessor::EndGroup(commandProcessor);
        }
    }
}

#endif
