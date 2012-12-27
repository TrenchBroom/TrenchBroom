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
#include "Utility/Preferences.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        const String EntityModelRendererManager::modelRendererKey(const Model::PointEntityModel& modelInfo, const StringList& searchPaths) {
            StringStream key;
            for (unsigned int i = 0; i < searchPaths.size(); i++)
                key << searchPaths[i] << " ";
            key << modelInfo.name() << " ";
            key << modelInfo.flagName() << " ";
            key << modelInfo.skinIndex();
            return Utility::toLower(key.str());
        }

        EntityModelRenderer* EntityModelRendererManager::modelRenderer(const Model::PointEntityModel& modelInfo, const StringList& mods) {
            assert(m_palette != NULL);
            
            if (!m_valid) {
                clear();
                m_valid = true;
            }
            
            IO::FileManager fileManager;
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            
            StringList searchPaths;
            String quakePath = prefs.getString(Preferences::QuakePath);
            for (unsigned int i = 0; i < mods.size(); i++)
                searchPaths.push_back(fileManager.appendPath(quakePath, mods[i]));

            const String key = modelRendererKey(modelInfo, searchPaths);

            MismatchCache::iterator mismatchIt = m_mismatches.find(key);
            if (mismatchIt != m_mismatches.end())
                return NULL;
            
            EntityModelRendererCache::iterator rendererIt = m_modelRenderers.find(key);
            if (rendererIt != m_modelRenderers.end())
                return rendererIt->second;

            String modelName = Utility::toLower(modelInfo.name().substr(1));
            String ext = Utility::toLower(fileManager.pathExtension(modelName));
            if (ext == "mdl") {
                Model::AliasManager& aliasManager = *Model::AliasManager::sharedManager;
                const Model::Alias* alias = aliasManager.alias(modelName, searchPaths, m_console);
                if (alias != NULL) {
                    unsigned int skinIndex = modelInfo.skinIndex();
                    Renderer::EntityModelRenderer* renderer = new AliasModelRenderer(*alias, skinIndex, *m_vbo, *m_palette);
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

        EntityModelRenderer* EntityModelRendererManager::modelRenderer(const Model::PointEntityDefinition& entityDefinition, const StringList& mods) {
            assert(!mods.empty());
            
            const Model::PointEntityModel* modelInfo = entityDefinition.model();
            if (modelInfo == NULL)
                return NULL;
            return modelRenderer(*modelInfo, mods);
        }

        EntityModelRenderer* EntityModelRendererManager::modelRenderer(const Model::Entity& entity, const StringList& mods) {
            const Model::EntityDefinition* definition = entity.definition();
            if (definition == NULL || definition->type() != Model::EntityDefinition::PointEntity)
                return NULL;
            return modelRenderer(*static_cast<const Model::PointEntityDefinition*>(definition), mods);
        }

        void EntityModelRendererManager::clear() {
            for (EntityModelRendererCache::iterator it = m_modelRenderers.begin(); it != m_modelRenderers.end(); ++it)
                delete it->second;
            m_modelRenderers.clear();
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
