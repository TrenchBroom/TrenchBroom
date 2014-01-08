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

#include "EntityModelRenderer.h"

#include "TrenchBroom.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "VecMath.h"
#include "CollectionUtils.h"
#include "Assets/EntityModel.h"
#include "Assets/EntityModelManager.h"
#include "Model/Entity.h"
#include "Model/ModelFilter.h"
#include "Renderer/MeshRenderer.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Transformation.h"

namespace TrenchBroom {
    namespace Renderer {
        EntityModelRenderer::EntityModelRenderer(Assets::EntityModelManager& entityModelManager, const Model::ModelFilter& filter) :
        m_entityModelManager(entityModelManager),
        m_filter(filter) {}

        EntityModelRenderer::~EntityModelRenderer() {
            clear();
        }
        
        void EntityModelRenderer::addEntity(Model::Entity* entity) {
            const Assets::ModelSpecification modelSpec = entity->modelSpecification();
            MeshRenderer* renderer = m_entityModelManager.renderer(modelSpec);
            if (renderer != NULL)
                m_entities[entity] = renderer;
        }
        
        void EntityModelRenderer::addEntities(const Model::EntityList& entities) {
            Model::EntityList::const_iterator it, end;
            for (it = entities.begin(), end = entities.end(); it != end; ++it)
                addEntity(*it);
        }
        
        void EntityModelRenderer::addEntities(const Model::EntitySet& entities) {
            Model::EntitySet::const_iterator it, end;
            for (it = entities.begin(), end = entities.end(); it != end; ++it)
                addEntity(*it);
        }
        
        void EntityModelRenderer::updateEntity(Model::Entity* entity) {
            removeEntity(entity);
            addEntity(entity);
        }
        
        void EntityModelRenderer::updateEntities(const Model::EntityList& entities) {
            Model::EntityList::const_iterator it, end;
            for (it = entities.begin(), end = entities.end(); it != end; ++it)
                updateEntity(*it);
        }
        
        void EntityModelRenderer::removeEntity(Model::Entity* entity) {
            m_entities.erase(entity);
        }
        
        void EntityModelRenderer::removeEntities(const Model::EntityList& entities) {
            Model::EntityList::const_iterator it, end;
            for (it = entities.begin(), end = entities.end(); it != end; ++it)
                removeEntity(*it);
        }

        void EntityModelRenderer::clear() {
            m_entities.clear();
        }

        bool EntityModelRenderer::applyTinting() const {
            return m_applyTinting;
        }
        
        void EntityModelRenderer::setApplyTinting(const bool applyTinting) {
            m_applyTinting = applyTinting;
        }
        
        const Color& EntityModelRenderer::tintColor() const {
            return m_tintColor;
        }
        
        void EntityModelRenderer::setTintColor(const Color& tintColor) {
            m_tintColor = tintColor;
        }
        
        void EntityModelRenderer::render(RenderContext& context) {
            PreferenceManager& prefs = PreferenceManager::instance();
            
            ActiveShader shader(context.shaderManager(), Shaders::EntityModelShader);
            shader.set("Brightness", prefs.get(Preferences::Brightness));
            shader.set("ApplyTinting", m_applyTinting);
            shader.set("TintColor", m_tintColor);
            shader.set("GrayScale", false);
            shader.set("Texture", 0);
            
            glEnable(GL_TEXTURE_2D);
            glActiveTexture(GL_TEXTURE0);
            m_entityModelManager.activateVbo();

            EntityMap::iterator it, end;
            for (it = m_entities.begin(), end = m_entities.end(); it != end; ++it) {
                Model::Entity* entity = it->first;
                if (!m_filter.visible(entity))
                    continue;
                
                MeshRenderer* renderer = it->second;
                
                const Vec3f position = entity->origin();
                const Quatf rotation = entity->rotation();
                const Mat4x4f matrix = translationMatrix(position) * rotationMatrix(rotation);
                MultiplyModelMatrix multMatrix(context.transformation(), matrix);

                renderer->render();
            }
            
            m_entityModelManager.deactivateVbo();
        }
    }
}
