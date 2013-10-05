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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "EntityModelManager.h"

#include "CollectionUtils.h"
#include "Assets/EntityModel.h"
#include "Model/Game.h"
#include "Renderer/MeshRenderer.h"

namespace TrenchBroom {
    namespace Assets {
        EntityModelManager::EntityModelManager() :
        m_vbo(0xFFFFF),
        m_prepared(true) {}

        EntityModelManager::~EntityModelManager() {
            clear();
        }

        void EntityModelManager::clear() {
            MapUtils::clearAndDelete(m_renderers);
            MapUtils::clearAndDelete(m_models);
            m_rendererMismatches.clear();
            m_modelMismatches.clear();
            m_prepared = true;
        }

        void EntityModelManager::reset(Model::GamePtr game) {
            if (m_game == game)
                return;
            clear();
            m_game = game;
        }
        
        EntityModel* EntityModelManager::model(const IO::Path& path) const {
            if (path.isEmpty())
                return NULL;
            
            ModelCache::const_iterator it = m_models.find(path);
            if (it != m_models.end())
                return it->second;

            if (m_modelMismatches.count(path) > 0)
                return NULL;
            
            EntityModel* model = m_game->loadModel(path);
            if (model == NULL)
                m_modelMismatches.insert(path);
            else
                m_models[path] = model;
            return model;
        }

        Renderer::MeshRenderer* EntityModelManager::renderer(const Assets::ModelSpecification& spec) const {
            EntityModel* entityModel = model(spec.path);
            if (entityModel == NULL)
                return NULL;
            
            RendererCache::const_iterator it = m_renderers.find(spec);
            if (it != m_renderers.end())
                return it->second;
            
            if (m_rendererMismatches.count(spec) > 0)
                return NULL;
            
            Renderer::MeshRenderer* renderer = entityModel->buildRenderer(m_vbo, spec.skinIndex, spec.frameIndex);
            if (renderer == NULL) {
                m_rendererMismatches.insert(spec);
            } else {
                m_renderers[spec] = renderer;
                m_prepared = false;
            }
            return renderer;
        }

        void EntityModelManager::activateVbo() {
            m_vbo.activate();
            prepareRenderers();
        }
        
        void EntityModelManager::deactivateVbo() {
            m_vbo.deactivate();
        }

        void EntityModelManager::prepareRenderers() {
            if (m_prepared)
                return;
            
            Renderer::SetVboState setVboState(m_vbo);
            setVboState.mapped();
            
            RendererCache::const_iterator it, end;
            for (it = m_renderers.begin(), end = m_renderers.end(); it != end; ++it) {
                Renderer::MeshRenderer* renderer = it->second;
                renderer->prepare();
            }
            m_prepared = true;
        }
    }
}
