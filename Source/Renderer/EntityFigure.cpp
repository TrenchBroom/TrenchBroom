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

#include "EntityFigure.h"

#include "Renderer/EntityRenderer.h"
#include "Utility/Color.h"
#include "Utility/Preferences.h"

namespace TrenchBroom {
    namespace Renderer {
        EntityFigure::EntityFigure(Model::MapDocument& document, Model::Entity& entity) :
        m_document(document),
        m_entity(entity),
        m_entityRenderer(NULL) {}
        
        EntityFigure::~EntityFigure() {
            delete m_entityRenderer;
            m_entityRenderer = NULL;
        }
        
        void EntityFigure::invalidate() {
            if (m_entityRenderer != NULL)
                m_entityRenderer->invalidateBounds();
        }
        
        void EntityFigure::render(Vbo& vbo, RenderContext& context) {
            if (m_entityRenderer == NULL) {
                Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();

                m_entityRenderer = new EntityRenderer(vbo, m_document);
                m_entityRenderer->setClassnameFadeDistance(prefs.getFloat(Preferences::SelectedInfoOverlayFadeDistance));
                m_entityRenderer->setClassnameColor(prefs.getColor(Preferences::SelectedInfoOverlayTextColor), prefs.getColor(Preferences::SelectedInfoOverlayBackgroundColor));
                m_entityRenderer->setOccludedClassnameColor(prefs.getColor(Preferences::SelectedInfoOverlayTextColor), prefs.getColor(Preferences::SelectedInfoOverlayBackgroundColor));
                m_entityRenderer->setBoundsColor(prefs.getColor(Preferences::SelectedEntityBoundsColor));
                m_entityRenderer->setOccludedBoundsColor(prefs.getColor(Preferences::OccludedSelectedEntityBoundsColor));
                m_entityRenderer->setTintColor(prefs.getColor(Preferences::SelectedEntityColor));
                
                m_entityRenderer->addEntity(m_entity);
            }
            
            assert(m_entityRenderer != NULL);
            m_entityRenderer->render(context);
        }
    }
}
