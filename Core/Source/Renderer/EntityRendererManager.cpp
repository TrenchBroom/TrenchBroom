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
#include <cassert>

#include "Model/Assets/Alias.h"
#include "Model/Assets/Bsp.h"
#include "Model/Assets/Palette.h"
#include "Model/Map/Entity.h"
#include "Model/Map/EntityDefinition.h"
#include "Renderer/AliasRenderer.h"
#include "Renderer/BspRenderer.h"
#include "Renderer/EntityRenderer.h"
#include "Renderer/Vbo.h"
#include "Utilities/glplat.h"
#include "Utilities/Utils.h"

namespace TrenchBroom {
    namespace Renderer {
        const string EntityRendererManager::entityRendererKey(const Model::ModelProperty& modelProperty, const vector<string>& searchPaths) {
            string key;
            for (int i = 0; i < searchPaths.size(); i++)
                key += searchPaths[i] + " ";
            key += modelProperty.modelPath + " ";
            key += modelProperty.flagName + " ";
            key += modelProperty.skinIndex;
            return key;
        }
        
        EntityRenderer* EntityRendererManager::entityRenderer(const Model::ModelProperty& modelProperty, const vector<string>& mods) {
            vector<string> searchPaths;
            for (int i = 0; i < mods.size(); i++)
                searchPaths.push_back(appendPath(m_quakePath, mods[i]));

            const string key = entityRendererKey(modelProperty, searchPaths);
            EntityRendererCache::iterator it = m_entityRenderers.find(key);
            if (it != m_entityRenderers.end()) return it->second;
            
            string modelName = modelProperty.modelPath.substr(1);
            string ext = pathExtension(modelName);
            if (ext == "mdl") {
                Model::Assets::AliasManager& aliasManager = Model::Assets::AliasManager::sharedManager();
                Model::Assets::Alias* alias = aliasManager.aliasForName(modelName, searchPaths);
                if (alias != NULL) {
                    int skinIndex = modelProperty.skinIndex;
                    Renderer::EntityRenderer* renderer = new Renderer::AliasRenderer(*alias, skinIndex, *m_vbo, m_palette);
                    m_entityRenderers[key] = renderer;
                    return renderer;
                }
            } else if (ext == "bsp") {
                Model::Assets::BspManager& bspManager = Model::Assets::BspManager::sharedManager();
                Model::Assets::Bsp* bsp = bspManager.bspForName(modelName, searchPaths);
                if (bsp != NULL) {
                    Renderer::EntityRenderer* renderer = new Renderer::BspRenderer(*bsp, *m_vbo, m_palette);
                    m_entityRenderers[key] = renderer;
                    return renderer;
                }
            } else {
                fprintf(stdout, "Warning: Unknown model type '%s'\n", ext.c_str());
            }
            
            return NULL;
        }

        EntityRendererManager::EntityRendererManager(const string& quakePath, Model::Assets::Palette& palette) : m_quakePath(quakePath), m_palette(palette) {
            m_vbo = new Renderer::Vbo(GL_ARRAY_BUFFER, 0xFFFF);
        }
        
        EntityRendererManager::~EntityRendererManager() {
            clear();
            delete m_vbo;
        }
        
        EntityRenderer* EntityRendererManager::entityRenderer(const Model::EntityDefinition& entityDefinition, const vector<string>& mods) {
            assert(!mods.empty());
            Model::ModelProperty* modelProperty = entityDefinition.defaultModelProperty();
            if (modelProperty == NULL)
                return NULL;
            return entityRenderer(*modelProperty, mods);
        }
        
        EntityRenderer* EntityRendererManager::entityRenderer(const Model::Entity& entity, const vector<string>& mods) {
            const Model::EntityDefinition* entityDefinition = entity.entityDefinition();
            if (entityDefinition == NULL)
                return NULL;
            return entityRenderer(*entityDefinition, mods);
        }
        
        void EntityRendererManager::clear() {
            for (EntityRendererCache::iterator it = m_entityRenderers.begin(); it != m_entityRenderers.end(); ++it)
                delete it->second;
            m_entityRenderers.clear();
        }
        
        void EntityRendererManager::setQuakePath(const string& quakePath) {
            if (m_quakePath == quakePath) return;
            m_quakePath = quakePath;
            clear();
        }

        void EntityRendererManager::activate() {
            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
            m_vbo->activate();
        }
        
        void EntityRendererManager::deactivate() {
            m_vbo->deactivate();
            glPopClientAttrib();
        }
    }
}
