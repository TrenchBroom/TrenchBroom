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

#include "EntityModelRendererManager.h"

#include <GL/glew.h>
#include "Model/Alias.h"
#include "Model/Bsp.h"
#include "Model/Entity.h"
#include "Model/EntityDefinition.h"
#include "Renderer/AliasModelRenderer.h"
#include "Renderer/BspModelRenderer.h"
#include "Renderer/EntityModelRenderer.h"
#include "Renderer/Palette.h"
#include "Renderer/Vbo.h"
#include "IO/FileManager.h"
#include "Utility/Console.h"
#include "Utility/Map.h"
#include "Utility/Preferences.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        const String EntityModelRendererManager::modelRendererKey(const Model::ModelDefinition& modelDefinition, const StringList& searchPaths) {
            StringStream key;
            for (size_t i = 0; i < searchPaths.size(); i++)
                key << searchPaths[i] << " ";
            key << modelDefinition.name() << " " << modelDefinition.skinIndex() << " " << modelDefinition.frameIndex();
            return Utility::toLower(key.str());
        }

        EntityModelRenderer* EntityModelRendererManager::modelRenderer(const Model::ModelDefinition& modelDefinition, const StringList& searchPaths) {
            assert(m_palette != NULL);
            IO::FileManager fileManager;
            
            if (!m_valid) {
                clear();
                m_valid = true;
            }
            
            const String key = modelRendererKey(modelDefinition, searchPaths);
            MismatchCache::iterator mismatchIt = m_mismatches.find(key);
            if (mismatchIt != m_mismatches.end())
                return NULL;
            
            EntityModelRendererCache::iterator rendererIt = m_modelRenderers.find(key);
            if (rendererIt != m_modelRenderers.end())
                return rendererIt->second;

            String modelName = Utility::toLower(modelDefinition.name().substr(1));
            String ext = Utility::toLower(fileManager.pathExtension(modelName));
            if (ext == "mdl") {
                unsigned int skinIndex = modelDefinition.skinIndex();
                unsigned int frameIndex = modelDefinition.frameIndex();

                Model::AliasManager& aliasManager = *Model::AliasManager::sharedManager;
                const Model::Alias* alias = aliasManager.alias(modelName, searchPaths, m_console);

                if (alias != NULL && skinIndex < alias->skins().size() && frameIndex < alias->frames().size()) {
                    Renderer::EntityModelRenderer* renderer = new AliasModelRenderer(*alias, frameIndex, skinIndex, *m_vbo, *m_palette);
                    m_modelRenderers[key] = renderer;
                    return renderer;
                }
            } else if (ext == "bsp") {
                Model::BspManager& bspManager = *Model::BspManager::sharedManager;
                const Model::Bsp* bsp = bspManager.bsp(modelName, searchPaths, m_console);
                if (bsp != NULL) {
                    Renderer::EntityModelRenderer* renderer = new BspModelRenderer(*bsp, *m_vbo, *m_palette);
                    m_modelRenderers[key] = renderer;
                    return renderer;
                }
            } else {
                m_console.warn("Unknown model type '%s'", ext.c_str());
            }

            m_mismatches.insert(key);
            return NULL;
        }

        EntityModelRendererManager::EntityModelRendererManager(Utility::Console& console) :
        m_palette(NULL),
        m_console(console),
        m_valid(true) {
            m_vbo = new Renderer::Vbo(GL_ARRAY_BUFFER, 0xFFFF);
        }

        EntityModelRendererManager::~EntityModelRendererManager() {
            clear();
            delete m_vbo;
            m_vbo = NULL;
        }

        EntityModelRenderer* EntityModelRendererManager::modelRenderer(const Model::PointEntityDefinition& entityDefinition, const StringList& searchPaths) {
            const Model::ModelDefinition* modelDefinition = entityDefinition.model();
            if (modelDefinition == NULL)
                return NULL;
            return modelRenderer(*modelDefinition, searchPaths);
        }

        EntityModelRenderer* EntityModelRendererManager::modelRenderer(const Model::Entity& entity, const StringList& searchPaths) {
            const Model::EntityDefinition* definition = entity.definition();
            if (definition == NULL || definition->type() != Model::EntityDefinition::PointEntity)
                return NULL;
            const Model::PointEntityDefinition* pointDefinition = static_cast<const Model::PointEntityDefinition*>(definition);
            const Model::ModelDefinition* modelDefinition = pointDefinition->model(entity.properties());
            if (modelDefinition == NULL)
                return NULL;
            return modelRenderer(*modelDefinition, searchPaths);
        }

        void EntityModelRendererManager::clear() {
            clearMismatches();
            Utility::deleteAll(m_modelRenderers);
        }
        
        void EntityModelRendererManager::clearMismatches() {
            m_mismatches.clear();
        }

        void EntityModelRendererManager::setPalette(const Palette& palette) {
            if (&palette == m_palette)
                return;
            m_palette = &palette;
            m_valid = false;
        }

        void EntityModelRendererManager::activate() {
            m_vbo->activate();
        }

        void EntityModelRendererManager::deactivate() {
            m_vbo->deactivate();
        }
    }
}
