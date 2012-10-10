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
        m_entity(entity) {}
        
        void EntityFigure::invalidate() {
            if (m_entityRenderer.get() != NULL)
                m_entityRenderer->invalidateBounds();
        }
        
        void EntityFigure::render(Vbo& vbo, RenderContext& context) {
            if (m_entityRenderer.get() == NULL) {
                Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
                float fadeDistance = prefs.getFloat(Preferences::SelectedInfoOverlayFadeDistance);
                const Color& color = prefs.getColor(Preferences::SelectedEntityBoundsColor);
                const Color& occludedColor = prefs.getColor(Preferences::OccludedSelectedEntityBoundsColor);

                m_entityRenderer = EntityRendererPtr(new EntityRenderer(vbo, m_document, fadeDistance, color, occludedColor));
                m_entityRenderer->addEntity(m_entity);
            }
            
            m_entityRenderer->render(context);
        }
    }
}