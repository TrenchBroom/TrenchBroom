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

#include "EntityDragTargetTool.h"

#include "Controller/ChangeEditStateCommand.h"
#include "Controller/CreateEntityCommand.h"
#include "Model/Entity.h"
#include "Model/EntityDefinition.h"
#include "Model/EntityDefinitionManager.h"
#include "Model/Filter.h"
#include "Model/Map.h"
#include "Model/MapDocument.h"
#include "Model/Picker.h"
#include "Renderer/Camera.h"
#include "Renderer/EntityFigure.h"
#include "Utility/Grid.h"
#include "Utility/String.h"
#include "Utility/VecMath.h"
#include "View/DocumentViewHolder.h"
#include "View/EditorView.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        void EntityDragTargetTool::updateFigure(InputEvent& event) {
            Model::MapDocument& document = documentViewHolder().document();
            View::EditorView& view = documentViewHolder().view();
            Model::Filter& filter = view.filter();
            Utility::Grid& grid = document.grid();
            
            Model::Hit* hit = event.pickResult->first(Model::Hit::FaceHit, true, filter);
            
            Vec3f delta;
            if (hit == NULL) {
                Vec3f newPosition = view.camera().defaultPoint(event.ray.direction);
                const Vec3f& center = m_entity->bounds().center();
                delta = grid.moveDeltaForEntity(center, document.map().worldBounds(), newPosition - center);
            } else {
                Model::Face& face = hit->face();
                delta = grid.moveDeltaForEntity(face, m_entity->bounds(), document.map().worldBounds(), event.ray, hit->hitPoint());
            }

            if (delta.null())
                return;
            
            m_entity->setProperty(Model::Entity::OriginKey, m_entity->origin() + delta, true);
            m_entityFigure->invalidate();
            document.UpdateAllViews();
        }
        
        bool EntityDragTargetTool::handleDragEnter(InputEvent& event, const String& payload) {
            StringList parts = Utility::split(payload, ':');
            if (parts.size() != 2)
                return NULL;
            if (parts[0] != "entity")
                return NULL;
            
            Model::MapDocument& document = documentViewHolder().document();
            Model::EntityDefinitionManager& definitionManager = document.definitionManager();
            Model::EntityDefinition* definition = definitionManager.definition(parts[1]);
            if (definition == NULL)
                return false;
            
            m_entity = new Model::Entity(document.map().worldBounds());
            m_entity->setProperty(Model::Entity::ClassnameKey, definition->name());
            m_entity->setDefinition(definition);
            
            m_entityFigure = new Renderer::EntityFigure(document, *m_entity);
            addFigure(m_entityFigure);
            updateFigure(event);
            
            return true;
        }
        
        void EntityDragTargetTool::handleDragMove(InputEvent& event) {
            assert(m_entity != NULL);
            updateFigure(event);
        }
        
        void EntityDragTargetTool::handleDragLeave() {
            assert(m_entity != NULL);

            removeFigure(m_entityFigure);
            deleteFigure(m_entityFigure);
            m_entityFigure = NULL;
            
            documentViewHolder().document().UpdateAllViews();
        }

        bool EntityDragTargetTool::handleDrop(InputEvent& event) {
            assert(m_entity != NULL);
            
            removeFigure(m_entityFigure);
            deleteFigure(m_entityFigure);
            m_entityFigure = NULL;

            BeginCommandGroup("Create Entity");
            
            Controller::CreateEntityCommand* createEntityCommand = Controller::CreateEntityCommand::createFromTemplate(documentViewHolder().document(), *m_entity);
            postCommand(createEntityCommand);
            
            Controller::ChangeEditStateCommand* changeEditStateCommand = Controller::ChangeEditStateCommand::select(documentViewHolder().document(), *createEntityCommand->entity());
            postCommand(changeEditStateCommand);
            
            EndCommandGroup();
            
            return true;
        }
        
        EntityDragTargetTool::EntityDragTargetTool(View::DocumentViewHolder& documentViewHolder) :
        DragTargetTool(documentViewHolder),
        m_entity(NULL),
        m_entityFigure(NULL) {}

        EntityDragTargetTool::~EntityDragTargetTool() {
            if (m_entityFigure != NULL) {
                deleteFigure(m_entityFigure);
                m_entityFigure = NULL;
            }
            if (m_entity != NULL) {
                delete m_entity;
                m_entity = NULL;
            }
        }
    }
}