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
#include "FlashSelectionAnimation.h"

#include "Controller/CameraEvent.h"
#include "Renderer/MapRenderer.h"
#include "Utility/Preferences.h"
#include "View/MapGLCanvas.h"

namespace TrenchBroom {
    namespace View {
        void FlashSelectionAnimation::doUpdate(double progress) {
            static const Color white(1.0f, 1.0f, 1.0f, 1.0f);
            const float fltProgress = static_cast<float>(progress);
            if (fltProgress < 1.0f) {
                // factor ranges from 1 to 0 and then back to 1
                const float factor = fltProgress < 0.5f ? 1.0f - 2.0f * fltProgress : 2.0f * (fltProgress - 0.5f);
                
                Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
                Color faceColor = prefs.getColor(Preferences::SelectedFaceColor);
                const Color& regularEdgeColor = prefs.getColor(Preferences::EdgeColor);
                const Color& selectedEdgeColor = prefs.getColor(Preferences::SelectedEdgeColor);
                
                faceColor.mix(white, 1.0f - factor);
                const Color edgeColor = regularEdgeColor.mixed(selectedEdgeColor, factor);
                
                m_renderer.setOverrideSelectionColors(true, faceColor, edgeColor, edgeColor);
            } else {
                m_renderer.setOverrideSelectionColors(false);
            }
            
            m_canvas.Refresh();
        }
        
        FlashSelectionAnimation::FlashSelectionAnimation(Renderer::MapRenderer& renderer, View::MapGLCanvas& canvas, wxLongLong duration) :
        Animation(EaseInEaseOutCurve, duration),
        m_renderer(renderer),
        m_canvas(canvas) {}
        
        Animation::Type FlashSelectionAnimation::type() const {
            static const Type type = Animation::uniqueType();
            return type;
        }
    }
}
