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

#ifndef __TrenchBroom__Preferences__
#define __TrenchBroom__Preferences__

#include "Controller/Input.h"
#include "Utility/Color.h"
#include "Utility/MessageException.h"
#include "Utility/String.h"
#include "Utility/VecMath.h"

#include <wx/config.h>
#include <wx/confbase.h>

#include <map>

using namespace TrenchBroom::Math;
using namespace TrenchBroom::Controller::ModifierKeys;
using namespace TrenchBroom::Controller::MouseButtons;

namespace TrenchBroom {
    namespace Preferences {
        class PreferenceManager;
        
        template <typename T>
        class Preference {
        protected:
            friend class PreferenceManager;

            String m_name;
            mutable T m_value;
            mutable bool m_initialized;
            
            inline void setValue(const T& value) const {
                m_value = value;
                m_initialized = true;
            }
            
            inline bool initialized() const {
                return m_initialized;
            }
        public:
            Preference(const String& name, const T& defaultValue) :  m_name(name), m_value(defaultValue), m_initialized(false) {}
            
            inline const String& name() const {
                return m_name;
            }
            
            inline const T& value() const {
                return m_value;
            }
        };
        
        static const Preference<float>  CameraLookSpeed(                    "Controls/Camera/Look speed",                       0.5f);
        static const Preference<float>  CameraPanSpeed(                     "Controls/Camera/Pan speed",                        0.5f);
        static const Preference<float>  CameraMoveSpeed(                    "Controls/Camera/Move speed",                       0.5f);
        static const Preference<bool>   CameraLookInvertX(                  "Controls/Camera/Look X inverted",                  false);
        static const Preference<bool>   CameraLookInvertY(                  "Controls/Camera/Look Y inverted",                  false);
        static const Preference<bool>   CameraPanInvertX(                   "Controls/Camera/Pan X inverted",                   false);
        static const Preference<bool>   CameraPanInvertY(                   "Controls/Camera/Pan Y inverted",                   false);
        static const Preference<float>  VertexHandleSize(                   "Controls/Vertex handle size",                      1.5f);
        static const Preference<float>  CameraFieldOfVision(                "Renderer/Camera field of vision",                  90.0f);
        static const Preference<float>  CameraNearPlane(                    "Renderer/Camera near plane",                       1.0f);
        static const Preference<float>  CameraFarPlane(                     "Renderer/Camera far plane",                        5000.0f);

        static const Preference<float>  InfoOverlayFadeDistance(            "Renderer/Info overlay fade distance",              400.0f);
        static const Preference<float>  SelectedInfoOverlayFadeDistance(    "Renderer/Selected info overlay fade distance",     400.0f);
        static const Preference<int>    RendererFontSize(                   "Renderer/Font size",                               16);
        static const Preference<float>  RendererBrightness(                 "Renderer/Brightness",                              1.0f);

        static const Preference<Color>  BackgroundColor(                    "Renderer/Colors/Background",                       Color(0.0f,  0.0f,  0.0f,  1.0f ));
        static const Preference<Color>  GridColor(                          "Renderer/Colors/Grid",                             Color(1.0f,  1.0f,  1.0f,  0.22f));

        static const Preference<Color>  FaceColor(                          "Renderer/Colors/Face",                             Color(0.2f,  0.2f,  0.2f,  1.0f ));
        static const Preference<Color>  SelectedFaceColor(                  "Renderer/Colors/Selected face",                    Color(0.6f,  0.35f, 0.35f, 1.0f ));
        static const Preference<Color>  LockedFaceColor(                    "Renderer/Colors/Locked face",                      Color(1.0f,  1.0f,  1.0f,  0.5f ));
        
        static const Preference<Color>  EdgeColor(                          "Renderer/Colors/Edge",                             Color(0.6f,  0.6f,  0.6f,  1.0f ));
        static const Preference<Color>  SelectedEdgeColor(                  "Renderer/Colors/Selected edge",                    Color(1.0f,  0.0f,  0.0f,  1.0f ));
        static const Preference<Color>  OccludedSelectedEdgeColor(          "Renderer/Colors/Occluded selected edge",           Color(1.0f,  0.0f,  0.0f,  0.35f));
        static const Preference<Color>  LockedEdgeColor(                    "Renderer/Colors/Locked edge",                      Color(0.6f,  0.6f,  0.6f,  0.5f ));
        
        static const Preference<Color>  EntityBoundsColor(                  "Renderer/Colors/Entity bounds",                    Color(0.5f,  0.5f,  0.5f,  1.0f ));
        static const Preference<Color>  SelectedEntityBoundsColor(          "Renderer/Colors/Selected entity bounds",           Color(1.0f,  0.0f,  0.0f,  1.0f ));
        static const Preference<Color>  OccludedSelectedEntityBoundsColor(  "Renderer/Colors/Occluded selected entity bounds",  Color(1.0f,  0.0f,  0.0f,  0.35f));
        static const Preference<Color>  LockedEntityBoundsColor(            "Renderer/Colors/Locked entity bounds",             Color(0.5f,  0.5f,  0.5f,  0.5f ));
        static const Preference<Color>  EntityBoundsWireframeColor(         "Renderer/Colors/Entity bounds (wireframe mode)",   Color(0.5f,  0.5f,  0.5f,  0.6f ));

        static const Preference<Color>  SelectionGuideColor(                "Renderer/Colors/Selection guide",                  Color(1.0f,  0.0f,  0.0f,  1.0f ));
        static const Preference<Color>  OccludedSelectionGuideColor(        "Renderer/Colors/Occluded selection guide",         Color(1.0f,  0.0f,  0.0f,  0.35f));
        
        static const Preference<Color>  InfoOverlayColor(                   "Renderer/Colors/Info overlay",                     Color(1.0f,  1.0f,  1.0f,  1.0f ));
        static const Preference<Color>  SelectedInfoOverlayColor(           "Renderer/Colors/Selected info overlay",            Color(1.0f,  0.0f,  0.0f,  1.0f ));
        static const Preference<Color>  OccludedSelectedInfoOverlayColor(   "Renderer/Colors/Occluded selected info overlay",   Color(1.0f,  0.0f,  0.0f,  0.35f));
        
        static const Preference<Color>  VertexHandleColor(                  "Renderer/Colors/Vertex handle",                    Color(1.0f,  1.0f,  1.0f,  1.0f ));
        static const Preference<Color>  OccludedVertexHandleColor(          "Renderer/Colors/Occluded vertex handle",           Color(1.0f,  1.0f,  1.0f,  0.35f));
        static const Preference<Color>  SelectedVertexHandleColor(          "Renderer/Colors/Selected vertex handle",           Color(1.0f,  0.0f,  0.0f,  1.0f ));
        static const Preference<Color>  OccludedSelectedVertexHandleColor(  "Renderer/Colors/Occluded selected vertex handle",  Color(1.0f,  0.0f,  0.0f,  0.35f));

        static const Preference<Color>  EdgeHandleColor(                    "Renderer/Colors/edge handle",                      Color(1.0f,  1.0f,  1.0f,  1.0f ));
        static const Preference<Color>  OccludedEdgeHandleColor(            "Renderer/Colors/Occluded edge handle",             Color(1.0f,  1.0f,  1.0f,  0.35f));
        static const Preference<Color>  SelectedEdgeHandleColor(            "Renderer/Colors/Selected edge handle",             Color(1.0f,  0.0f,  0.0f,  1.0f ));
        static const Preference<Color>  OccludedSelectedEdgeHandleColor(    "Renderer/Colors/Occluded selected edge handle",    Color(1.0f,  0.0f,  0.0f,  0.35f));

        static const Preference<Color>  FaceHandleColor(                    "Renderer/Colors/face handle",                      Color(1.0f,  1.0f,  1.0f,  1.0f ));
        static const Preference<Color>  OccludedFaceHandleColor(            "Renderer/Colors/Occluded face handle",             Color(1.0f,  1.0f,  1.0f,  0.35f));
        static const Preference<Color>  SelectedFaceHandleColor(            "Renderer/Colors/Selected face handle",             Color(1.0f,  0.0f,  0.0f,  1.0f ));
        static const Preference<Color>  OccludedSelectedFaceHandleColor(    "Renderer/Colors/Occluded selected face handle",    Color(1.0f,  0.0f,  0.0f,  0.35f));
        
        static const Preference<Color>  SelectedTextureColor(               "Texture browser/Selected texture color",           Color(0.8f,  0.0f,  0.0f,  1.0f ));
        static const Preference<Color>  UsedTextureColor(                   "Texture browser/Used texture color",               Color(0.8f,  0.8f,  0.0f,  1.0f ));
        static const Preference<Color>  OverriddenTextureColor(             "Texture browser/Overridden texture color",         Color(0.5f,  0.5f,  0.5f,  1.0f ));
        
#if defined _WIN32
        static const Preference<String> QuakePath(                          "General/Quake path",                               "C:\\Program Files\\Quake");
        static const Preference<String> RendererFontName(                   "Renderer/Font name",                               "Arial");
#elif defined __APPLE__
        static const Preference<String> QuakePath(                          "General/Quake path",                               "/Applications/Quake");
        static const Preference<String> RendererFontName(                   "Renderer/Font name",                               "LucidaGrande");
#elif defined __linux__
#endif

        class PreferenceManager {
        private:
            wxConfig* m_config;

            PreferenceManager() {
                m_config = new wxConfig("TrenchBroom");
            }
            
            ~PreferenceManager() {
                delete m_config;
                m_config = NULL;
            }
            
            bool parseMouseState(const String& str, Controller::MouseState& mouseState) const;
        public:
            inline static PreferenceManager& preferences() {
                static PreferenceManager prefs;
                return prefs;
            }
            
            inline bool getBool(const Preference<bool>& preference) const {
                if (!preference.initialized()) {
                    wxString str;
                    if (m_config->Read(preference.name(), &str)) {
                        long longValue;
                        if (str.ToLong(&longValue))
                            preference.setValue(longValue > 0L);
                    }
                }
                
                return preference.value();
            }
            
            inline int getInt(const Preference<int>& preference) const {
                if (!preference.initialized()) {
                    wxString str;
                    if (m_config->Read(preference.name(), &str)) {
                        long longValue;
                        if (str.ToLong(&longValue))
                            preference.setValue(static_cast<int>(longValue));
                    }
                }
                
                return preference.value();
            }
            
            inline float getFloat(const Preference<float>& preference) const {
                if (!preference.initialized()) {
                    wxString str;
                    if (m_config->Read(preference.name(), &str)) {
                        double doubleValue;
                        if (str.ToDouble(&doubleValue))
                            preference.setValue(static_cast<float>(doubleValue));
                    }
                }
                
                return preference.value();
            }
            
            inline const String& getString(const Preference<String>& preference) const {
                if (!preference.initialized()) {
                    wxString str;
                    if (m_config->Read(preference.name(), &str))
                        preference.setValue(str.ToStdString());
                }
                
                return preference.value();
            }

            inline const Color& getColor(const Preference<Color>& preference) const {
                if (!preference.initialized()) {
                    wxString str;
                    if (m_config->Read(preference.name(), &str))
                        preference.setValue(Color(str.ToStdString()));
                }
                
                return preference.value();
            }
            
            inline const Controller::MouseState& getMouseState(const Preference<Controller::MouseState>& preference) const {
                if (!preference.initialized()) {
                    wxString str;
                    if (m_config->Read(preference.name(), &str)) {
                        Controller::MouseState mouseState;
                        if (parseMouseState(str.ToStdString(), mouseState))
                            preference.setValue(mouseState);
                    }
                }
                
                return preference.value();
            }
        };
    }
}

#endif /* defined(__TrenchBroom__Preferences__) */
