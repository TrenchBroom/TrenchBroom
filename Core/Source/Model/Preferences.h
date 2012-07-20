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
        public:
            static const std::string CameraKey;
            static const std::string CameraOrbitKey;
            static const std::string CameraInvertY;
            static const std::string CameraFov;
			static const std::string SelectionToolMultiKey;
			static const std::string SelectionToolGridKey;
            static const std::string ResizeToolKey;
            static const std::string Brightness;
            static const std::string GridColor;
            static const std::string FaceColor;
            static const std::string EdgeColor;
            static const std::string SelectedFaceColor;
            static const std::string SelectedEdgeColor;
            static const std::string HiddenSelectedEdgeColor;
            static const std::string EntityBoundsColor;
            static const std::string EntityBoundsWireframeColor;
            static const std::string SelectedEntityBoundsColor;
            static const std::string HiddenSelectedEntityBoundsColor;
            static const std::string SelectionGuideColor;
            static const std::string HiddenSelectionGuideColor;
            static const std::string BackgroundColor;
            static const std::string InfoOverlayColor;
            static const std::string InfoOverlayFadeDistance;
            static const std::string SelectedInfoOverlayColor;
            static const std::string SelectedInfoOverlayFadeDistance;
            static const std::string SelectedTextureColor;
            static const std::string UsedTextureColor;
            static const std::string OverriddenTextureColor;
            static const std::string RendererFontName;
            static const std::string RendererFontSize;
            static const std::string QuakePath;
            static const std::string VertexHandleSize;
            static const std::string VertexHandleColor;
            static const std::string HiddenVertexHandleColor;
            static const std::string SelectedVertexHandleColor;
            static const std::string HiddenSelectedVertexHandleColor;
            static const std::string EdgeHandleColor;
            static const std::string HiddenEdgeHandleColor;
            static const std::string SelectedEdgeHandleColor;
            static const std::string HiddenSelectedEdgeHandleColor;
            static const std::string FaceHandleColor;
            static const std::string HiddenFaceHandleColor;
            static const std::string SelectedFaceHandleColor;
            static const std::string HiddenSelectedFaceHandleColor;
        protected:
            int m_cameraKey;
            int m_cameraOrbitKey;
            bool m_cameraInvertY;
            
			int m_selectionToolMultiKey;
			int m_selectionToolGridKey;

            int m_resizeToolKey;
            
            float m_cameraFov;
            
            float m_brightness;

            Vec4f m_gridColor;
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
            
            Vec4f m_selectedTextureColor;
            Vec4f m_usedTextureColor;
            Vec4f m_overriddenTextureColor;
            
            std::string m_rendererFontName;
            int m_rendererFontSize;
            
            std::string m_quakePath;

            float m_vertexHandleSize;
            Vec4f m_vertexHandleColor;
            Vec4f m_hiddenVertexHandleColor;
            Vec4f m_selectedVertexHandleColor;
            Vec4f m_hiddenSelectedVertexHandleColor;
            Vec4f m_edgeHandleColor;
            Vec4f m_hiddenEdgeHandleColor;
            Vec4f m_selectedEdgeHandleColor;
            Vec4f m_hiddenSelectedEdgeHandleColor;
            Vec4f m_faceHandleColor;
            Vec4f m_hiddenFaceHandleColor;
            Vec4f m_selectedFaceHandleColor;
            Vec4f m_hiddenSelectedFaceHandleColor;
            
            virtual void loadDefaults();
            virtual void loadPlatformDefaults() = 0;

            virtual bool loadInt(const std::string& key, int& value) = 0;
            virtual bool loadFloat(const std::string& key, float& value) = 0;
            virtual bool loadBool(const std::string& key, bool& value) = 0;
            virtual bool loadString(const std::string& key, std::string& value) = 0;
            virtual bool loadVec3f(const std::string& key, Vec3f& value) = 0;
            virtual bool loadVec4f(const std::string& key, Vec4f& value) = 0;
            void loadPreferences();
            
            virtual void saveInt(const std::string& key, int value) = 0;
            virtual void saveFloat(const std::string& key, float value) = 0;
            virtual void saveBool(const std::string& key, bool value) = 0;
            virtual void saveString(const std::string& key, const std::string& value) = 0;
			virtual void saveVec3f(const std::string& key, const Vec3f& value) = 0;
			virtual void saveVec4f(const std::string& key, const Vec4f& value) = 0;
            virtual bool saveInstantly() = 0;
			void savePreferences();
        public:
            static Preferences* sharedPreferences;

            typedef Event<const std::string&> PreferencesEvent;
            PreferencesEvent preferencesDidChange;
            
            Preferences() {};
            virtual ~Preferences() {};
            
            void init();
			void save();
            
            int cameraKey();
            int cameraOrbitKey();
            bool cameraInvertY();
            void setCameraInvertY(bool cameraInvertY);
            
			int selectionToolMultiKey();
			int selectionToolGridKey();

            int resizeToolKey();
            
            float cameraFov();
            void setCameraFov(float cameraFov);
            float cameraNear();
            float cameraFar();
            float brightness();
            void setBrightness(float brightness);
            
            const Vec4f& gridColor();
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
            
            const Vec4f& selectedTextureColor();
            const Vec4f& usedTextureColor();
            const Vec4f& overriddenTextureColor();
            
            const std::string& rendererFontName();
            unsigned int rendererFontSize();
            
            const std::string& quakePath();
            void setQuakePath(const std::string& quakePath);
            
            float vertexHandleSize();
            const Vec4f& vertexHandleColor();
            const Vec4f& hiddenVertexHandleColor();
            const Vec4f& selectedVertexHandleColor();
            const Vec4f& hiddenSelectedVertexHandleColor();
            const Vec4f& edgeHandleColor();
            const Vec4f& hiddenEdgeHandleColor();
            const Vec4f& selectedEdgeHandleColor();
            const Vec4f& hiddenSelectedEdgeHandleColor();
            const Vec4f& faceHandleColor();
            const Vec4f& hiddenFaceHandleColor();
            const Vec4f& selectedFaceHandleColor();
            const Vec4f& hiddenSelectedFaceHandleColor();
        };
    }
}

#endif
