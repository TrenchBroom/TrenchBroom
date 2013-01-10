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

#include "CreateEntityTool.h"

#include "Controller/AddObjectsCommand.h"
#include "Controller/ChangeEditStateCommand.h"
#include "Model/Entity.h"
#include "Model/EntityDefinition.h"
#include "Model/EntityDefinitionManager.h"
#include "Model/Map.h"
#include "Model/MapDocument.h"
#include "Model/Picker.h"
#include "Renderer/EntityFigure.h"
#include "Utility/Grid.h"
#include "Utility/String.h"
#include "Utility/VecMath.h"
#include "View/EditorView.h"

#include <cassert>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Controller {
        void CreateEntityTool::updateEntityPosition(InputState& inputState) {
            assert(m_entity != NULL);
            
            Utility::Grid& grid = document().grid();
            Model::FaceHit* hit = static_cast<Model::FaceHit*>(inputState.pickResult().first(Model::HitType::FaceHit, true, view().filter()));
            
            Vec3f delta;
            if (hit == NULL) {
                Vec3f newPosition = inputState.camera().defaultPoint(inputState.pickRay().direction);
                const Vec3f& center = m_entity->bounds().center();
                delta = grid.moveDeltaForEntity(center, document().map().worldBounds(), newPosition - center);
            } else {
                Model::Face& face = hit->face();
                delta = grid.moveDeltaForEntity(face, m_entity->bounds(), document().map().worldBounds(), inputState.pickRay(), hit->hitPoint());
            }
            
            if (delta.null())
                return;
            
            m_entity->setProperty(Model::Entity::OriginKey, m_entity->origin() + delta, true);
            m_entityFigure->invalidate();
        }

        bool CreateEntityTool::handleIsModal(InputState& inputState) {
            return dragType() == DTDragTarget;
        }

        void CreateEntityTool::handleRenderFirst(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext) {
            if (m_entityFigure != NULL)
                m_entityFigure->render(vbo, renderContext);
        }

        bool CreateEntityTool::handleDragEnter(InputState& inputState, const String& payload) {
            StringList parts = Utility::split(payload, ':');
            if (parts.size() != 2)
                return NULL;
            if (parts[0] != "entity")
                return NULL;
            
            Model::EntityDefinitionManager& definitionManager = document().definitionManager();
            Model::EntityDefinition* definition = definitionManager.definition(parts[1]);
            if (definition == NULL)
                return false;
            
            m_entity = new Model::Entity(document().map().worldBounds());
            m_entity->setProperty(Model::Entity::ClassnameKey, definition->name());
            m_entity->setDefinition(definition);
            m_entityFigure = new Renderer::EntityFigure(document(), *m_entity);
            updateEntityPosition(inputState);
            
            return true;
        }
        
        void CreateEntityTool::handleDragMove(InputState& inputState, const String& payload) {
            updateEntityPosition(inputState);
        }
        
        void CreateEntityTool::handleDragLeave(InputState& inputState, const String& payload) {
            assert(m_entity != NULL);
            delete m_entity;
            m_entity = NULL;
            deleteFigure(m_entityFigure);
            m_entityFigure = NULL;
        }
        
        bool CreateEntityTool::handleDragDrop(InputState& inputState, const String& payload) {
            assert(m_entity != NULL);

            beginCommandGroup(wxT("Create Entity"));
            Controller::AddObjectsCommand* addObjectsCommand = Controller::AddObjectsCommand::addEntity(document(), *m_entity);
            submitCommand(addObjectsCommand);
            Controller::ChangeEditStateCommand* changeEditStateCommand = Controller::ChangeEditStateCommand::select(document(), *m_entity);
            submitCommand(changeEditStateCommand);
            endCommandGroup();
            
            m_entity = NULL;
            deleteFigure(m_entityFigure);
            m_entityFigure = NULL;

            return true;
        }

        CreateEntityTool::CreateEntityTool(View::DocumentViewHolder& documentViewHolder) :
        Tool(documentViewHolder, true),
        m_entity(NULL),
        m_entityFigure(NULL) {}
        
        CreateEntityTool::~CreateEntityTool() {
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
