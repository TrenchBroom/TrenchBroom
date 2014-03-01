/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "CreateEntityTool.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Assets/EntityDefinition.h"
#include "Assets/EntityDefinitionManager.h"
#include "Assets/EntityModelManager.h"
#include "Assets/ModelDefinition.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/HitAdapter.h"
#include "Model/HitFilters.h"
#include "Model/Map.h"
#include "Model/Picker.h"
#include "View/ControllerFacade.h"
#include "View/Grid.h"
#include "View/InputState.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        CreateEntityTool::CreateEntityTool(BaseTool* next, MapDocumentWPtr document, ControllerWPtr controller, Renderer::FontManager& fontManager) :
        Tool(next, document, controller),
        m_renderer(lock(document)->entityModelManager(), fontManager, lock(document)->filter()),
        m_entity(NULL) {}

        bool CreateEntityTool::doDragEnter(const InputState& inputState, const String& payload) {
            assert(m_entity == NULL);
            
            const StringList parts = StringUtils::split(payload, ':');
            if (parts.size() != 2)
                return false;
            if (parts[0] != "entity")
                return false;
            
            const Assets::EntityDefinitionManager& definitionManager = document()->entityDefinitionManager();
            Assets::EntityDefinition* definition = definitionManager.definition(parts[1]);
            if (definition == NULL)
                return false;
            
            if (definition->type() != Assets::EntityDefinition::PointEntity)
                return false;
            
            const Model::Map* map = document()->map();
            m_entity = map->createEntity();
            m_entity->addOrUpdateProperty(Model::PropertyKeys::Classname, definition->name());
            m_entity->setDefinition(definition);
            
            const Assets::ModelSpecification modelSpec = m_entity->modelSpecification();
            const Assets::EntityModelManager& modelManager = document()->entityModelManager();
            Assets::EntityModel* model = modelManager.model(modelSpec.path);
            m_entity->setModel(model);
            
            m_renderer.addEntity(m_entity);
            updateEntityPosition(inputState);
            
            return true;
        }
        
        bool CreateEntityTool::doDragMove(const InputState& inputState) {
            assert(m_entity != NULL);
            updateEntityPosition(inputState);
            return true;
        }
        
        void CreateEntityTool::doDragLeave(const InputState& inputState) {
            assert(m_entity != NULL);
            m_renderer.removeEntity(m_entity);
            delete m_entity;
            m_entity = NULL;
        }
        
        bool CreateEntityTool::doDragDrop(const InputState& inputState) {
            assert(m_entity != NULL);
            
            m_renderer.removeEntity(m_entity);
            m_entity->setModel(NULL);
            m_entity->setDefinition(NULL);

            controller()->beginUndoableGroup("Create " + m_entity->classname());
            controller()->deselectAll();
            controller()->addEntity(m_entity);
            controller()->selectObject(m_entity);
            controller()->closeGroup();
            
            m_entity = NULL;
            return true;
        }
        
        void CreateEntityTool::updateEntityPosition(const InputState& inputState) {
            assert(m_entity != NULL);

            Vec3 delta;
            const Grid& grid = document()->grid();
            const Model::PickResult::FirstHit first = Model::firstHit(inputState.pickResult(), Model::Brush::BrushHit, document()->filter(), true);
            if (first.matches) {
                Model::BrushFace* face = hitAsFace(first.hit);
                delta = grid.moveDeltaForBounds(face, m_entity->bounds(), document()->worldBounds(), inputState.pickRay(), first.hit.hitPoint());
            } else {
                const Vec3 newPosition(inputState.camera().defaultPoint(inputState.pickRay()));
                const Vec3 center = m_entity->bounds().center();
                delta = grid.moveDeltaForPoint(center, document()->worldBounds(), newPosition - center);
            }
            
            if (delta.null())
                return;
            
            m_entity->addOrUpdateProperty(Model::PropertyKeys::Origin, m_entity->origin() + delta);
            m_renderer.updateEntity(m_entity);
        }

        void CreateEntityTool::doRender(const InputState& inputState, Renderer::RenderContext& renderContext) {
            PreferenceManager& prefs = PreferenceManager::instance();
            
            m_renderer.setOverlayTextColor(prefs.get(Preferences::InfoOverlayTextColor));
            m_renderer.setOverlayBackgroundColor(prefs.get(Preferences::InfoOverlayBackgroundColor));
            m_renderer.setApplyTinting(false);
            m_renderer.render(renderContext);
        }
    }
}
