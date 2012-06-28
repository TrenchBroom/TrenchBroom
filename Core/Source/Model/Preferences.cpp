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

#include "Preferences.h"

namespace TrenchBroom {
    namespace Model {
        const std::string Preferences::CameraKey                         = "Controls: camera key";
        const std::string Preferences::CameraOrbitKey                    = "Controls: camera orbit key";
        const std::string Preferences::CameraInvertY                     = "Controls: invert camera Y axis";
        const std::string Preferences::CameraFov                         = "Camera: field of vision";
        const std::string Preferences::Brightness                        = "Renderer: brightness";
        const std::string Preferences::GridColor                         = "Renderer: grid color";
        const std::string Preferences::FaceColor                         = "Renderer: face color";
        const std::string Preferences::EdgeColor                         = "Renderer: edge color";
        const std::string Preferences::SelectedFaceColor                 = "Renderer: face color (selected)";
        const std::string Preferences::SelectedEdgeColor                 = "Renderer: edge color (selected)";
        const std::string Preferences::HiddenSelectedEdgeColor           = "Renderer: edge color (selected and hidden)";
        const std::string Preferences::EntityBoundsColor                 = "Renderer: entity bounds color";
        const std::string Preferences::EntityBoundsWireframeColor        = "Renderer: entity bounds color (wireframe mode)";
        const std::string Preferences::SelectedEntityBoundsColor         = "Renderer: entity bounds color (selected)";
        const std::string Preferences::HiddenSelectedEntityBoundsColor   = "Renderer: entity bounds color (selected and hidden)";
        const std::string Preferences::SelectionGuideColor               = "Renderer: selection guide color";
        const std::string Preferences::HiddenSelectionGuideColor         = "Renderer: selection guide color (hidden)";
        const std::string Preferences::BackgroundColor                   = "Renderer: background color";
        const std::string Preferences::InfoOverlayColor                  = "Renderer: info overlay color";
        const std::string Preferences::InfoOverlayFadeDistance           = "Renderer: info overlay fade distance";
        const std::string Preferences::SelectedInfoOverlayColor          = "Renderer: info overlay color (selected)";
        const std::string Preferences::SelectedInfoOverlayFadeDistance   = "Renderer: info overlay fade distance (selected)";
        const std::string Preferences::SelectedTextureColor              = "Texture Browser: selected texture color";
        const std::string Preferences::UsedTextureColor                  = "Texture Browser: used texture color";
        const std::string Preferences::OverriddenTextureColor            = "Texture Browser: overridden texture color";
        const std::string Preferences::RendererFontName                  = "Renderer: font name";
        const std::string Preferences::RendererFontSize                  = "Renderer: font size";
        const std::string Preferences::QuakePath                         = "General: quake path";
        
        void Preferences::loadDefaults() {
            m_cameraInvertY = false;
            m_cameraFov = 90;
            m_gridColor = Vec4f(1.0f, 1.0f, 1.0f, 0.22f);
            m_faceColor = Vec4f(0.2f, 0.2f, 0.2f, 1);
            m_edgeColor = Vec4f(0.6f, 0.6f, 0.6f, 1.0f);
            m_selectedFaceColor = Vec4f(0.6f, 0.35f, 0.35f, 1);
            m_selectedEdgeColor = Vec4f(1, 0, 0, 1);
            m_hiddenSelectedEdgeColor = Vec4f(1, 0, 0, 0.35f);
            m_entityBoundsColor = Vec4f(0.5f, 0.5f, 0.5f, 1.0f);
            m_entityBoundsWireframeColor = Vec4f(0.5f, 0.5f, 0.5f, 0.6f);
            m_selectedEntityBoundsColor = m_selectedEdgeColor;
            m_hiddenSelectedEntityBoundsColor = m_hiddenSelectedEdgeColor;
            m_selectionGuideColor = m_selectedEdgeColor;
            m_hiddenSelectionGuideColor = m_hiddenSelectedEdgeColor;
            m_backgroundColor = Vec4f(0, 0, 0, 1);
            
            m_infoOverlayColor = Vec4f(1, 1, 1, 1);
            m_infoOverlayFadeDistance = 400;
            m_selectedInfoOverlayColor = Vec4f(1, 0, 0, 1);
            m_selectedInfoOverlayFadeDistance = 2000;
            
            m_selectedTextureColor = Vec4f(0.8f, 0, 0, 1);
            m_usedTextureColor = Vec4f(0.8f, 0.8f, 0, 1);
            m_overriddenTextureColor = Vec4f(0.5f, 0.5f, 0.5f, 1);
            
            m_rendererFontName = "Arial.ttf";
            m_rendererFontSize = 11;
            
			m_brightness = 1.0f;
            
            m_quakePath = "";
            
            loadPlatformDefaults();
        }
        
        void Preferences::loadPreferences() {
            loadInt(CameraKey, m_cameraKey);
            loadInt(CameraOrbitKey, m_cameraOrbitKey);
            loadBool(CameraInvertY, m_cameraInvertY);
            loadFloat(CameraFov, m_cameraFov);
            loadFloat(Brightness, m_brightness);
            loadVec4f(GridColor, m_gridColor);
            loadVec4f(FaceColor, m_faceColor);
            loadVec4f(EdgeColor, m_edgeColor);
            loadVec4f(SelectedFaceColor, m_selectedFaceColor);
            loadVec4f(SelectedEdgeColor, m_selectedEdgeColor);
            loadVec4f(HiddenSelectedEdgeColor, m_hiddenSelectedEdgeColor);
            loadVec4f(EntityBoundsColor, m_entityBoundsColor);
            loadVec4f(EntityBoundsWireframeColor, m_entityBoundsWireframeColor);
            loadVec4f(SelectedEntityBoundsColor, m_selectedEntityBoundsColor);
            loadVec4f(HiddenSelectedEntityBoundsColor, m_hiddenSelectedEntityBoundsColor);
            loadVec4f(SelectionGuideColor, m_selectionGuideColor);
            loadVec4f(HiddenSelectionGuideColor, m_hiddenSelectionGuideColor);
            loadVec4f(BackgroundColor, m_backgroundColor);
			loadVec4f(InfoOverlayColor, m_infoOverlayColor);
			loadFloat(InfoOverlayFadeDistance, m_infoOverlayFadeDistance);
			loadVec4f(SelectedInfoOverlayColor, m_selectedInfoOverlayColor);
			loadFloat(SelectedInfoOverlayFadeDistance, m_selectedInfoOverlayFadeDistance);
            loadVec4f(SelectedTextureColor, m_selectedTextureColor);
            loadVec4f(UsedTextureColor, m_usedTextureColor);
            loadVec4f(OverriddenTextureColor, m_overriddenTextureColor);
			loadString(RendererFontName, m_rendererFontName);
			loadInt(RendererFontSize, m_rendererFontSize);
			loadString(QuakePath, m_quakePath);
        }

		void Preferences::savePreferences() {
			saveInt(CameraKey, m_cameraKey);
			saveInt(CameraOrbitKey, m_cameraOrbitKey);
			saveBool(CameraInvertY, m_cameraInvertY);
			saveFloat(CameraFov, m_cameraFov);
			saveFloat(Brightness, m_brightness);
            saveVec4f(GridColor, m_gridColor);
			saveVec4f(FaceColor, m_faceColor);
			saveVec4f(EdgeColor, m_edgeColor);
			saveVec4f(SelectedFaceColor, m_selectedFaceColor);
			saveVec4f(SelectedEdgeColor, m_selectedEdgeColor);
			saveVec4f(HiddenSelectedEdgeColor, m_hiddenSelectedEdgeColor);
			saveVec4f(EntityBoundsColor, m_entityBoundsColor);
			saveVec4f(EntityBoundsWireframeColor, m_entityBoundsWireframeColor);
			saveVec4f(SelectedEntityBoundsColor, m_selectedEntityBoundsColor);
			saveVec4f(HiddenSelectedEntityBoundsColor, m_hiddenSelectedEntityBoundsColor);
			saveVec4f(SelectionGuideColor, m_selectionGuideColor);
			saveVec4f(HiddenSelectionGuideColor, m_hiddenSelectionGuideColor);
			saveVec4f(BackgroundColor, m_backgroundColor);
			saveVec4f(InfoOverlayColor, m_infoOverlayColor);
			saveFloat(InfoOverlayFadeDistance, m_infoOverlayFadeDistance);
			saveVec4f(SelectedInfoOverlayColor, m_selectedInfoOverlayColor);
			saveFloat(SelectedInfoOverlayFadeDistance, m_selectedInfoOverlayFadeDistance);
            saveVec4f(SelectedTextureColor, m_selectedTextureColor);
            saveVec4f(UsedTextureColor, m_usedTextureColor);
            saveVec4f(OverriddenTextureColor, m_overriddenTextureColor);
			saveString(RendererFontName, m_rendererFontName);
			saveInt(RendererFontSize, m_rendererFontSize);
			saveString(QuakePath, m_quakePath);
		}

		Preferences* Preferences::sharedPreferences = NULL;

        void Preferences::init() {
            loadDefaults();
            loadPlatformDefaults();
            loadPreferences();
        }
        
		void Preferences::save() {
			savePreferences();
		}

		int Preferences::cameraKey() {
            return m_cameraKey;
        }
        
        int Preferences::cameraOrbitKey() {
            return m_cameraOrbitKey;
        }

        bool Preferences::cameraInvertY() {
            return m_cameraInvertY;
        }

        void Preferences::setCameraInvertY(bool cameraInvertY) {
            if (cameraInvertY == m_cameraInvertY)
                return;
            
            m_cameraInvertY = cameraInvertY;
            if (saveInstantly())
                saveBool(CameraInvertY, m_cameraInvertY);
            preferencesDidChange(QuakePath);
        }

        float Preferences::cameraFov() {
            return m_cameraFov;
        }
        
        void Preferences::setCameraFov(float cameraFov) {
            if (cameraFov == m_cameraFov)
                return;
            
            m_cameraFov = cameraFov;
            if (saveInstantly())
                saveFloat(CameraFov, m_cameraFov);
            preferencesDidChange(QuakePath);
        }

        float Preferences::cameraNear() {
            return 10;
        }
        
        float Preferences::cameraFar() {
            return 10000;
        }

        float Preferences::brightness() {
            return m_brightness;
        }

        void Preferences::setBrightness(float brightness) {
            if (brightness == m_brightness)
                return;
            
            m_brightness = brightness;
            if (saveInstantly())
                saveFloat(Brightness, m_brightness);
            preferencesDidChange(QuakePath);
        }
        
        const Vec4f& Preferences::gridColor() {
            return m_gridColor;
        }

        const Vec4f& Preferences::faceColor() {
            return m_faceColor;
        }
        
        const Vec4f& Preferences::edgeColor(){
            return m_edgeColor;
        }
        
        const Vec4f& Preferences::selectedFaceColor() {
            return m_selectedFaceColor;
        }
        
        const Vec4f& Preferences::selectedEdgeColor() {
            return m_selectedEdgeColor;
        }
        
        const Vec4f& Preferences::hiddenSelectedEdgeColor() {
            return m_hiddenSelectedEdgeColor;
        }
        
        const Vec4f& Preferences::entityBoundsColor() {
            return m_entityBoundsColor;
        }
        
        const Vec4f& Preferences::entityBoundsWireframeColor() {
            return m_entityBoundsWireframeColor;
        }

        const Vec4f& Preferences::selectedEntityBoundsColor() {
            return m_selectedEntityBoundsColor;
        }
        
        const Vec4f& Preferences::hiddenSelectedEntityBoundsColor() {
            return m_hiddenSelectedEntityBoundsColor;
        }

        const Vec4f& Preferences::selectionGuideColor() {
            return m_selectionGuideColor;
        }
        const Vec4f& Preferences::hiddenSelectionGuideColor() {
            return m_hiddenSelectionGuideColor;
        }

        const Vec4f& Preferences::backgroundColor() {
            return m_backgroundColor;
        }

        const Vec4f& Preferences::infoOverlayColor() {
            return m_infoOverlayColor;
        }
        
        float Preferences::infoOverlayFadeDistance() {
            return m_infoOverlayFadeDistance;
        }
        
        const Vec4f& Preferences::selectedInfoOverlayColor() {
            return m_selectedInfoOverlayColor;
        }
        
        float Preferences::selectedInfoOverlayFadeDistance() {
            return m_selectedInfoOverlayFadeDistance;
        }

        const Vec4f& Preferences::selectedTextureColor() {
            return m_selectedTextureColor;
        }
        
        const Vec4f& Preferences::usedTextureColor() {
            return m_usedTextureColor;
        }
        
        const Vec4f& Preferences::overriddenTextureColor() {
            return m_overriddenTextureColor;
        }

        const std::string& Preferences::rendererFontName() {
            return m_rendererFontName;
        }
        
        unsigned int Preferences::rendererFontSize() {
            return m_rendererFontSize;
        }

        const std::string& Preferences::quakePath() {
            return m_quakePath;
        }

        void Preferences::setQuakePath(const std::string& quakePath) {
            if (quakePath == m_quakePath)
                return;
            
            m_quakePath = quakePath;
            if (saveInstantly())
                saveString(QuakePath, m_quakePath);
            preferencesDidChange(QuakePath);
        }
    }
}
