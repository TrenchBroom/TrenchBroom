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

#include "EntityModelManager.h"

#include "CollectionUtils.h"
#include "Exceptions.h"
#include "Logger.h"
#include "Assets/EntityModel.h"
#include "Renderer/MeshRenderer.h"

namespace TrenchBroom {
    namespace Assets {
        EntityModelManager::EntityModelManager(Logger* logger) :
        m_logger(logger),
        m_vbo(0xFFFFF) {}
        
        EntityModelManager::~EntityModelManager() {
            clear();
        }
        
        void EntityModelManager::clear() {
            MapUtils::clearAndDelete(m_renderers);
            MapUtils::clearAndDelete(m_models);
            m_rendererMismatches.clear();
            m_modelMismatches.clear();
            m_renderersToPrepare.clear();
            m_modelsToPrepare.clear();
            
            if (m_logger != NULL)
                m_logger->debug("Cleared entity models");
        }
        
        EntityModel* EntityModelManager::model(const IO::Path& path) const {
            if (path.isEmpty())
                return NULL;
            
            ModelCache::const_iterator it = m_models.find(path);
            if (it != m_models.end())
                return it->second;
            
            if (m_modelMismatches.count(path) > 0)
                return NULL;
            
            try {
                EntityModel* model = m_game->loadModel(path);
                assert(model != NULL);
                m_models[path] = model;
                m_modelsToPrepare.push_back(model);
                
                if (m_logger != NULL)
                    m_logger->debug("Loaded entity model %s", path.asString().c_str());

                return model;
            } catch (const GameException& e) {
                m_modelMismatches.insert(path);
                if (m_logger != NULL)
                    m_logger->debug(e.what());
                throw;
            }
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
            
            Renderer::MeshRenderer* renderer = entityModel->buildRenderer(spec.skinIndex, spec.frameIndex);
            if (renderer == NULL) {
                m_rendererMismatches.insert(spec);
                
                if (m_logger != NULL)
                    m_logger->debug("Failed to construct entity model renderer for %s", spec.asString().c_str());
            } else {
                m_renderers[spec] = renderer;
                m_renderersToPrepare.push_back(renderer);
                
                if (m_logger != NULL)
                    m_logger->debug("Constructed entity model renderer for %s", spec.asString().c_str());
            }
            return renderer;
        }
        
        void EntityModelManager::activateVbo() {
            prepareModels();
            m_vbo.activate();
            prepareRenderers();
        }
        
        void EntityModelManager::deactivateVbo() {
            m_vbo.deactivate();
        }
        
        void EntityModelManager::prepareRenderers() {
            if (m_renderersToPrepare.empty())
                return;
            
            Renderer::SetVboState setVboState(m_vbo);
            setVboState.mapped();
            
            RendererList::const_iterator it, end;
            for (it = m_renderersToPrepare.begin(), end = m_renderersToPrepare.end(); it != end; ++it) {
                Renderer::MeshRenderer* renderer = *it;
                renderer->prepare(m_vbo);
            }
            m_renderersToPrepare.clear();
        }

        void EntityModelManager::prepareModels() {
            ModelList::const_iterator it, end;
            for (it = m_modelsToPrepare.begin(), end = m_modelsToPrepare.end(); it != end; ++it) {
                EntityModel* model = *it;
                model->prepare();
            }
            m_modelsToPrepare.clear();
        }
    }
}
