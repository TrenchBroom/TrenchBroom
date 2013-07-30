/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "EntityModelRenderer.h"

#include "TrenchBroom.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "VecMath.h"
#include "CollectionUtils.h"
#include "Assets/EntityModel.h"
#include "Model/Entity.h"
#include "Renderer/MeshRenderer.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Transformation.h"

namespace TrenchBroom {
    namespace Renderer {
        EntityModelRenderer::EntityModelRenderer() :
        m_vbo(new Vbo(0xFFFFF)) {}

        EntityModelRenderer::~EntityModelRenderer() {
            clear();
        }
        
        void EntityModelRenderer::addEntity(Model::Entity* entity) {
            const Assets::ModelSpecification modelSpec = entity->modelSpecification();
            RendererCache::iterator it = m_renderers.find(modelSpec);
            if (it != m_renderers.end()) {
                m_entities[entity] = it->second;
            } else if (m_mismatches.count(modelSpec) == 0) {
                Assets::EntityModel* model = entity->model();
                if (model != NULL) {
                    MeshRenderer* renderer = model->buildRenderer(*m_vbo, modelSpec.skinIndex, modelSpec.frameIndex);
                    if (renderer != NULL) {
                        m_renderers[modelSpec] = renderer;
                        m_entities[entity] = renderer;
                    }
                } else {
                    m_mismatches.insert(modelSpec);
                }
            }
        }
        
        void EntityModelRenderer::addEntities(const Model::EntityList& entities) {
            Model::EntityList::const_iterator it, end;
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
            MapUtils::clearAndDelete(m_renderers);
            m_mismatches.clear();
        }
        
        void EntityModelRenderer::render(RenderContext& context) {
            SetVboState setVboState(*m_vbo);
            setVboState.mapped();
            EntityMap::iterator it, end;
            for (it = m_entities.begin(), end = m_entities.end(); it != end; ++it) {
                MeshRenderer* renderer = it->second;
                renderer->prepare();
            }

            PreferenceManager& prefs = PreferenceManager::instance();
            
            ActiveShader shader(context.shaderManager(), Shaders::EntityModelShader);
            shader.set("Brightness", prefs.getFloat(Preferences::Brightness));
            shader.set("ApplyTinting", false);
//            shader.set("TintColor", m_tintColor);
            shader.set("GrayScale", false);
            shader.set("Texture", 0);
            
            glEnable(GL_TEXTURE_2D);
            glActiveTexture(GL_TEXTURE0);
            setVboState.active();
            for (it = m_entities.begin(), end = m_entities.end(); it != end; ++it) {
                Model::Entity* entity = it->first;
                MeshRenderer* renderer = it->second;
                
                const Vec3f position = entity->origin();
                const Quatf rotation = entity->rotation();
                
                const Mat4x4f matrix = translationMatrix(position) * rotationMatrix(rotation);
                
                MultiplyModelMatrix multMatrix(context.transformation(), matrix);
                renderer->render();
            }
        }
    }
}
