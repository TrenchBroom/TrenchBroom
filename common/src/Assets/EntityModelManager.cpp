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
#include "Renderer/MeshRenderer.h"

namespace TrenchBroom {
    namespace Assets {
        EntityModelManager::EntityModelManager(Logger* logger) :
        m_logger(logger),
        m_loader(NULL) {}
        
        EntityModelManager::~EntityModelManager() {
            clear();
        }
        
        void EntityModelManager::clear() {
            MapUtils::clearAndDelete(m_renderers);
            MapUtils::clearAndDelete(m_models);
            m_rendererMismatches.clear();
            m_modelMismatches.clear();
            
            if (m_logger != NULL)
                m_logger->debug("Cleared entity models");
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
                
                if (m_logger != NULL)
                    m_logger->debug("Constructed entity model renderer for %s", spec.asString().c_str());
            }
            return renderer;
        }
        
        EntityModel* EntityModelManager::loadModel(const IO::Path& path) const {
            assert(m_loader != NULL);
            return m_loader->loadEntityModel(path);
        }
    }
}
