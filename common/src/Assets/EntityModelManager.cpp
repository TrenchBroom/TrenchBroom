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
#include "IO/EntityModelLoader.h"
#include "Renderer/TexturedIndexRangeRenderer.h"

namespace TrenchBroom {
    namespace Assets {
        EntityModelManager::EntityModelManager(Logger* logger, int minFilter, int magFilter) :
        m_logger(logger),
        m_loader(NULL),
        m_minFilter(minFilter),
        m_magFilter(magFilter),
        m_resetTextureMode(false) {}
        
        EntityModelManager::~EntityModelManager() {
            clear();
        }
        
        void EntityModelManager::clear() {
            MapUtils::clearAndDelete(m_renderers);
            MapUtils::clearAndDelete(m_models);
            m_rendererMismatches.clear();
            m_modelMismatches.clear();
            
            m_unpreparedModels.clear();
            m_unpreparedRenderers.clear();
            
            if (m_logger != NULL)
                m_logger->debug("Cleared entity models");
        }
        
        void EntityModelManager::setTextureMode(const int minFilter, const int magFilter) {
            m_minFilter = minFilter;
            m_magFilter = magFilter;
            m_resetTextureMode = true;
        }

        void EntityModelManager::setLoader(const IO::EntityModelLoader* loader) {
            clear();
            m_loader = loader;
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
                EntityModel* model = loadModel(path);
                assert(model != NULL);
                m_models[path] = model;
                m_unpreparedModels.push_back(model);
                
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
        
        Renderer::TexturedIndexRangeRenderer* EntityModelManager::renderer(const Assets::ModelSpecification& spec) const {
            EntityModel* entityModel = model(spec.path);
            if (entityModel == NULL)
                return NULL;
            
            RendererCache::const_iterator it = m_renderers.find(spec);
            if (it != m_renderers.end())
                return it->second;
            
            if (m_rendererMismatches.count(spec) > 0)
                return NULL;
            
            Renderer::TexturedIndexRangeRenderer* renderer = entityModel->buildRenderer(spec.skinIndex, spec.frameIndex);
            if (renderer == NULL) {
                m_rendererMismatches.insert(spec);
                
                if (m_logger != NULL)
                    m_logger->debug("Failed to construct entity model renderer for %s", spec.asString().c_str());
            } else {
                m_renderers[spec] = renderer;
                m_unpreparedRenderers.push_back(renderer);
                
                if (m_logger != NULL)
                    m_logger->debug("Constructed entity model renderer for %s", spec.asString().c_str());
            }
            return renderer;
        }
        
        EntityModel* EntityModelManager::loadModel(const IO::Path& path) const {
            assert(m_loader != NULL);
            return m_loader->loadEntityModel(path);
        }

        void EntityModelManager::prepare(Renderer::Vbo& vbo) {
            resetTextureMode();
            prepareModels();
            prepareRenderers(vbo);
        }

        void EntityModelManager::resetTextureMode() {
            if (m_resetTextureMode) {
                ModelCache::const_iterator it, end;
                for (it = m_models.begin(), end = m_models.end(); it != end; ++it) {
                    EntityModel* model = it->second;
                    model->setTextureMode(m_minFilter, m_magFilter);
                }
                m_resetTextureMode = false;
            }
        }
        
        void EntityModelManager::prepareModels() {
            ModelList::const_iterator it, end;
            for (it = m_unpreparedModels.begin(), end = m_unpreparedModels.end(); it != end; ++it) {
                Assets::EntityModel* model = *it;
                model->prepare(m_minFilter, m_magFilter);
            }
            m_unpreparedModels.clear();
        }
        
        void EntityModelManager::prepareRenderers(Renderer::Vbo& vbo) {
            RendererList::const_iterator it, end;
            for (it = m_unpreparedRenderers.begin(), end = m_unpreparedRenderers.end(); it != end; ++it) {
                Renderer::TexturedIndexRangeRenderer* renderer = *it;
                renderer->prepare(vbo);
            }
            m_unpreparedRenderers.clear();
        }
    }
}
