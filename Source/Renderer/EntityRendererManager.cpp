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

#include "EntityRendererManager.h"

#include <GL/glew.h>
#include "Model/Alias.h"
#include "Model/Bsp.h"
#include "Model/Entity.h"
#include "Model/EntityDefinition.h"
#include "Renderer/AliasRenderer.h"
#include "Renderer/BspRenderer.h"
#include "Renderer/EntityRenderer.h"
#include "Renderer/Palette.h"
#include "Renderer/Vbo.h"
#include "IO/FileManager.h"
#include "Utility/Console.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        const String EntityRendererManager::entityRendererKey(const Model::PointEntityModel& modelInfo, const StringList& searchPaths) {
            StringStream key;
            for (unsigned int i = 0; i < searchPaths.size(); i++)
                key << searchPaths[i] << " ";
            key << modelInfo.name() << " ";
            key << modelInfo.flagName() << " ";
            key << modelInfo.skinIndex();
            return Utility::toLower(key.str());
        }

        EntityRenderer* EntityRendererManager::entityRenderer(const Model::PointEntityModel& modelInfo, const StringList& mods) {
            assert(m_palette != NULL);
            
            if (!m_valid) {
                clear();
                m_valid = true;
            }
            
            IO::FileManager fileManager;
            
            StringList searchPaths;
            for (unsigned int i = 0; i < mods.size(); i++)
                searchPaths.push_back(fileManager.appendPath(m_quakePath, mods[i]));

            const String key = entityRendererKey(modelInfo, searchPaths);

            MismatchCache::iterator mismatchIt = m_mismatches.find(key);
            if (mismatchIt != m_mismatches.end())
                return NULL;
            
            EntityRendererCache::iterator rendererIt = m_entityRenderers.find(key);
            if (rendererIt != m_entityRenderers.end())
                return rendererIt->second;

            String modelName = Utility::toLower(modelInfo.name().substr(1));
            String ext = Utility::toLower(fileManager.pathExtension(modelName));
            if (ext == "mdl") {
                Model::AliasManager& aliasManager = *Model::AliasManager::sharedManager;
                const Model::Alias* alias = aliasManager.alias(modelName, searchPaths, m_console);
                if (alias != NULL) {
                    unsigned int skinIndex = modelInfo.skinIndex();
                    Renderer::EntityRenderer* renderer = new AliasRenderer(*alias, skinIndex, *m_vbo, *m_palette);
                    m_entityRenderers[key] = renderer;
                    return renderer;
                }
            } else if (ext == "bsp") {
                Model::BspManager& bspManager = *Model::BspManager::sharedManager;
                const Model::Bsp* bsp = bspManager.bsp(modelName, searchPaths, m_console);
                if (bsp != NULL) {
                    Renderer::EntityRenderer* renderer = new BspRenderer(*bsp, *m_vbo, *m_palette);
                    m_entityRenderers[key] = renderer;
                    return renderer;
                }
            } else {
                m_console.warn("Unknown model type '%s'", ext.c_str());
            }

            m_mismatches.insert(key);
            return NULL;
        }

        EntityRendererManager::EntityRendererManager(Utility::Console& console) :
        m_palette(NULL),
        m_console(console),
        m_valid(true) {
            m_vbo = new Renderer::Vbo(GL_ARRAY_BUFFER, 0xFFFF);
        }

        EntityRendererManager::~EntityRendererManager() {
            clear();
            delete m_vbo;
            m_vbo = NULL;
        }

        EntityRenderer* EntityRendererManager::entityRenderer(const Model::PointEntityDefinition& entityDefinition, const StringList& mods) {
            assert(!mods.empty());
            
            const Model::PointEntityModel* modelInfo = entityDefinition.model();
            if (modelInfo == NULL)
                return NULL;
            return entityRenderer(*modelInfo, mods);
        }

        EntityRenderer* EntityRendererManager::entityRenderer(const Model::Entity& entity, const StringList& mods) {
            const Model::EntityDefinition* definition = entity.definition();
            if (definition == NULL || definition->type() != Model::EntityDefinition::PointEntity)
                return NULL;
            return entityRenderer(*static_cast<const Model::PointEntityDefinition*>(definition), mods);
        }

        void EntityRendererManager::clear() {
            for (EntityRendererCache::iterator it = m_entityRenderers.begin(); it != m_entityRenderers.end(); ++it)
                delete it->second;
            m_entityRenderers.clear();
            m_mismatches.clear();
        }

        void EntityRendererManager::setQuakePath(const String& quakePath) {
            if (quakePath == m_quakePath)
                return;
            m_quakePath = quakePath;
            m_valid = false;
        }
        
        void EntityRendererManager::setPalette(const Palette& palette) {
            if (&palette == m_palette)
                return;
            m_palette = &palette;
            m_valid = false;
        }

        void EntityRendererManager::activate() {
            m_vbo->activate();
        }

        void EntityRendererManager::deactivate() {
            m_vbo->deactivate();
        }
    }
}
