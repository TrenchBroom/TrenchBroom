/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#include "Logger.h"
#include "Assets/EntityModel.h"
#include "IO/EntityModelLoader.h"
#include "Model/Entity.h"
#include "Renderer/TexturedIndexRangeRenderer.h"

namespace TrenchBroom {
    namespace Assets {
        EntityModelManager::EntityModelManager(Logger* logger, int minFilter, int magFilter) :
        m_logger(logger),
        m_loader(nullptr),
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
            
            // Remove logging because it might fail when the document is already destroyed.
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
            if (path.isEmpty()) {
                return nullptr;
            }

            auto it = m_models.find(path);
            if (it != std::end(m_models)) {
                return it->second;
            }

            if (m_modelMismatches.count(path) > 0) {
                return nullptr;
            }

            try {
                auto* model = loadModel(path);
                ensure(model != nullptr, "model is null");
                m_models[path] = model;
                m_unpreparedModels.push_back(model);
                
                if (m_logger != nullptr) {
                    m_logger->debug("Loaded entity model %s", path.asString().c_str());
                }

                return model;
            } catch (const GameException&) {
                m_modelMismatches.insert(path);
                throw;
            }
        }
        
        EntityModel* EntityModelManager::safeGetModel(const IO::Path& path) const {
            try {
                return model(path);
            } catch (const GameException&) {
                return nullptr;
            }
        }
        
        Renderer::TexturedIndexRangeRenderer* EntityModelManager::renderer(const Assets::ModelSpecification& spec) const {
            auto* entityModel = safeGetModel(spec.path);

            if (entityModel == nullptr) {
                return nullptr;
            }

            auto it = m_renderers.find(spec);
            if (it != std::end(m_renderers)) {
                return it->second;
            }

            if (m_rendererMismatches.count(spec) > 0) {
                return nullptr;
            }

            auto* renderer = entityModel->buildRenderer(spec.skinIndex, spec.frameIndex);
            if (renderer == nullptr) {
                m_rendererMismatches.insert(spec);
                
                if (m_logger != nullptr) {
                    m_logger->error("Failed to construct entity model renderer for %s, check the skin and frame indices", spec.asString().c_str());
                }
            } else {
                m_renderers[spec] = renderer;
                m_unpreparedRenderers.push_back(renderer);
                
                if (m_logger != nullptr) {
                    m_logger->debug("Constructed entity model renderer for %s", spec.asString().c_str());
                }
            }
            return renderer;
        }
        
        bool EntityModelManager::hasModel(const Model::Entity* entity) const {
            return hasModel(entity->modelSpecification());
        }
        
        bool EntityModelManager::hasModel(const Assets::ModelSpecification& spec) const {
            return renderer(spec) != nullptr;
        }

        EntityModel* EntityModelManager::loadModel(const IO::Path& path) const {
            ensure(m_loader != nullptr, "loader is null");
            return m_loader->loadEntityModel(path);
        }

        void EntityModelManager::prepare(Renderer::Vbo& vbo) {
            resetTextureMode();
            prepareModels();
            prepareRenderers(vbo);
        }

        void EntityModelManager::resetTextureMode() {
            if (m_resetTextureMode) {
                for (const auto& entry : m_models) {
                    auto* model = entry.second;
                    model->setTextureMode(m_minFilter, m_magFilter);
                }
                m_resetTextureMode = false;
            }
        }
        
        void EntityModelManager::prepareModels() {
            for (auto* model : m_unpreparedModels) {
                model->prepare(m_minFilter, m_magFilter);
            }
            m_unpreparedModels.clear();
        }
        
        void EntityModelManager::prepareRenderers(Renderer::Vbo& vbo) {
            for (auto* renderer : m_unpreparedRenderers) {
                renderer->prepare(vbo);
            }
            m_unpreparedRenderers.clear();
        }
    }
}
