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

#ifndef TrenchBroom_Header_h
#define TrenchBroom_Header_h

#include "Utilities/Event.h"
#include "Utilities/VecMath.h"

namespace TrenchBroom {
    namespace Model {
        class Preferences {
        protected:
            static const string CameraKey;
            static const string CameraOrbitKey;
            static const string FaceColor;
            static const string EdgeColor;
            static const string SelectedFaceColor;
            static const string SelectedEdgeColor;
            static const string HiddenSelectedEdgeColor;
            static const string EntityBoundsColor;
            static const string EntityBoundsWireframeColor;
            static const string SelectedEntityBoundsColor;
            static const string HiddenSelectedEntityBoundsColor;
            static const string SelectionGuideColor;
            static const string HiddenSelectionGuideColor;
            static const string BackgroundColor;
            static const string InfoOverlayColor;
            static const string InfoOverlayFadeDistance;
            static const string SelectedInfoOverlayColor;
            static const string SelectedInfoOverlayFadeDistance;
            static const string RendererFontName;
            static const string RendererFontSize;
            static const string GridAlpha;
            static const string QuakePath;
            
            int m_cameraKey;
            int m_cameraOrbitKey;
            
            Vec4f m_faceColor;
            Vec4f m_edgeColor;
            Vec4f m_selectedFaceColor;
            Vec4f m_selectedEdgeColor;
            Vec4f m_hiddenSelectedEdgeColor;
            Vec4f m_entityBoundsColor;
            Vec4f m_entityBoundsWireframeColor;
            Vec4f m_selectedEntityBoundsColor;
            Vec4f m_hiddenSelectedEntityBoundsColor;
            Vec4f m_selectionGuideColor;
            Vec4f m_hiddenSelectionGuideColor;
            Vec4f m_backgroundColor;
            
            Vec4f m_infoOverlayColor;
            float m_infoOverlayFadeDistance;
            Vec4f m_selectedInfoOverlayColor;
            float m_selectedInfoOverlayFadeDistance;
            
            string m_rendererFontName;
            float m_rendererFontSize;
            
            float m_gridAlpha;
            
            string m_quakePath;

            virtual void loadDefaults();
            virtual void loadPlatformDefaults() = 0;
            virtual void loadPreferences() = 0;
            
            virtual void saveInt(const string& key, int value) = 0;
            virtual void saveFloat(const string& key, float value) = 0;
            virtual void saveString(const string& key, const string& value) = 0;
        public:
            static Preferences* sharedPreferences;

            Preferences() {};
            virtual ~Preferences() {};
            
            void init();
            
            int cameraKey();
            int cameraOrbitKey();
            
            float cameraFov();
            float cameraNear();
            float cameraFar();
            float brightness();
            
            const Vec4f& faceColor();
            const Vec4f& edgeColor();
            const Vec4f& selectedFaceColor();
            const Vec4f& selectedEdgeColor();
            const Vec4f& hiddenSelectedEdgeColor();
            const Vec4f& entityBoundsColor();
            const Vec4f& entityBoundsWireframeColor();
            const Vec4f& selectedEntityBoundsColor();
            const Vec4f& hiddenSelectedEntityBoundsColor();
            const Vec4f& selectionGuideColor();
            const Vec4f& hiddenSelectionGuideColor();
            const Vec4f& backgroundColor();

            const Vec4f& infoOverlayColor();
            float infoOverlayFadeDistance();
            const Vec4f& selectedInfoOverlayColor();
            float selectedInfoOverlayFadeDistance();
            
            const string& rendererFontName();
            float rendererFontSize();
            
            float gridAlpha();
            
            const string& quakePath();
            
            
        };
    }
}

#endif
