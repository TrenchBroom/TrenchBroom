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

#include "EntityRenderer.h"

#include "Model/MapDocument.h"
#include "Renderer/EntityClassnameAnchor.h"
#include "Renderer/EntityModelRendererManager.h"
#include "Renderer/SharedResources.h"
#include "Utility/Preferences.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        EntityRenderer::EntityRenderer(Vbo& vbo, Model::MapDocument& document) :
        m_vbo(vbo),
        m_document(document),
        m_modelRendererCacheValid(true),
        m_boundsValid(true) {}

        void EntityRenderer::addEntity(Model::Entity& entity) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            EntityModelRendererManager& modelRendererManager = m_document.sharedResources().modelRendererManager();
            
            const String& fontName = prefs.getString(Preferences::RendererFontName);
            int fontSize = prefs.getInt(Preferences::RendererFontSize);
            Text::FontDescriptor fontDescriptor(fontName, fontSize);
            
            const Model::PropertyValue& classname = *entity.classname();
            EntityModelRenderer* renderer = modelRendererManager.modelRenderer(entity, m_document.mods());
            if (renderer != NULL)
                m_modelRenderers[&entity] = CachedEntityModelRenderer(renderer, classname);
            
            EntityClassnameAnchor* anchor = new EntityClassnameAnchor(entity);
            m_classnameRenderer->addString(&entity, fontDescriptor, classname, anchor);
            
            m_boundsValid = false;
        }
        
        void EntityRenderer::addEntities(const Model::EntityList& entities) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            EntityModelRendererManager& modelRendererManager = m_document.sharedResources().modelRendererManager();
            
            const String& fontName = prefs.getString(Preferences::RendererFontName);
            int fontSize = prefs.getInt(Preferences::RendererFontSize);
            Text::FontDescriptor fontDescriptor(fontName, fontSize);
            
            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity* entity = entities[i];
                const Model::PropertyValue& classname = *entity->classname();
                EntityModelRenderer* renderer = modelRendererManager.modelRenderer(*entity, m_document.mods());
                if (renderer != NULL)
                    m_modelRenderers[entity] = CachedEntityModelRenderer(renderer, classname);
                
                EntityClassnameAnchor* anchor = new EntityClassnameAnchor(*entity);
                m_classnameRenderer->addString(entity, fontDescriptor, classname, anchor);
            }
            
            m_boundsValid = false;
        }
        
        void EntityRenderer::updateEntity(Model::Entity& entity) {
        }
        
        void EntityRenderer::updateEntities(const Model::EntityList& entities) {
        }
        
        void EntityRenderer::removeEntity(Model::Entity& entity) {
            m_modelRenderers.erase(&entity);
            m_classnameRenderer->removeString(&entity);
            m_boundsValid = false;
        }
        
        void EntityRenderer::removeEntities(const Model::EntityList& entities) {
            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity* entity = entities[i];
                m_modelRenderers.erase(entity);
                m_classnameRenderer->removeString(entity);
            }
            m_boundsValid = false;
        }
        
        void EntityRenderer::render(RenderContext& context) {
        }
    }
}