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

#import "MacPreferences.h"
#import "Controller/Tool.h"

namespace TrenchBroom {
    namespace Model {
        void MacPreferences::setDictionaryValue(NSMutableDictionary* dict, const std::string& key, int value) {
            [dict setObject:[NSNumber numberWithInt:value] 
                     forKey:[NSString stringWithCString:key.c_str() encoding:NSASCIIStringEncoding]];
        }
        
        void MacPreferences::setDictionaryValue(NSMutableDictionary* dict, const std::string& key, float value) {
            [dict setObject:[NSNumber numberWithFloat:value] 
                     forKey:[NSString stringWithCString:key.c_str() encoding:NSASCIIStringEncoding]];
        }

        void MacPreferences::setDictionaryValue(NSMutableDictionary* dict, const std::string& key, bool value) {
            [dict setObject:[NSNumber numberWithBool:value == true ? YES : NO] 
                     forKey:[NSString stringWithCString:key.c_str() encoding:NSASCIIStringEncoding]];
        }
        
        void MacPreferences::setDictionaryValue(NSMutableDictionary* dict, const std::string& key, const std::string& value) {
            [dict setObject:[NSString stringWithCString:value.c_str() encoding:NSASCIIStringEncoding] 
                     forKey:[NSString stringWithCString:key.c_str() encoding:NSASCIIStringEncoding]];
        }
        
        void MacPreferences::setDictionaryValue(NSMutableDictionary* dict, const std::string& key, const Vec3f& value) {
            setDictionaryValue(dict, key, value.asString());
        }

        void MacPreferences::setDictionaryValue(NSMutableDictionary* dict, const std::string& key, const Vec4f& value) {
            setDictionaryValue(dict, key, value.asString());
        }
        
        void MacPreferences::loadDefaults() {
            Preferences::loadDefaults();

            NSMutableDictionary* dict = [[NSMutableDictionary alloc] init];
            setDictionaryValue(dict, CameraKey, m_cameraKey);
            setDictionaryValue(dict, CameraOrbitKey, m_cameraOrbitKey);
            setDictionaryValue(dict, CameraLookSpeed, m_cameraLookSpeed);
            setDictionaryValue(dict, CameraPanSpeed, m_cameraPanSpeed);
            setDictionaryValue(dict, CameraMoveSpeed, m_cameraMoveSpeed);
            setDictionaryValue(dict, CameraLookInvertY, m_cameraLookInvertY);
            setDictionaryValue(dict, CameraPanInvertX, m_cameraPanInvertX);
            setDictionaryValue(dict, CameraPanInvertY, m_cameraPanInvertY);
            setDictionaryValue(dict, CameraFov, m_cameraFov);
            setDictionaryValue(dict, SelectionToolMultiKey, m_selectionToolMultiKey);
            setDictionaryValue(dict, SelectionToolGridKey, m_selectionToolGridKey);
            setDictionaryValue(dict, ResizeToolKey, m_resizeToolKey);
            setDictionaryValue(dict, Brightness, m_brightness);
            setDictionaryValue(dict, GridColor, m_gridColor);
            setDictionaryValue(dict, FaceColor, m_faceColor);
            setDictionaryValue(dict, EdgeColor, m_edgeColor);
            setDictionaryValue(dict, SelectedFaceColor, m_selectedFaceColor);
            setDictionaryValue(dict, SelectedEdgeColor, m_selectedEdgeColor);
            setDictionaryValue(dict, HiddenSelectedEdgeColor, m_hiddenSelectedEdgeColor);
            setDictionaryValue(dict, EntityBoundsColor, m_entityBoundsColor);
            setDictionaryValue(dict, EntityBoundsWireframeColor, m_entityBoundsWireframeColor);
            setDictionaryValue(dict, SelectedEntityBoundsColor, m_selectedEntityBoundsColor);
            setDictionaryValue(dict, HiddenSelectedEntityBoundsColor, m_hiddenSelectedEntityBoundsColor);
            setDictionaryValue(dict, SelectionGuideColor, m_selectionGuideColor);
            setDictionaryValue(dict, HiddenSelectionGuideColor, m_hiddenSelectionGuideColor);
            setDictionaryValue(dict, BackgroundColor, m_backgroundColor);
            setDictionaryValue(dict, InfoOverlayColor, m_infoOverlayColor);
            setDictionaryValue(dict, InfoOverlayFadeDistance, m_infoOverlayFadeDistance);
            setDictionaryValue(dict, SelectedInfoOverlayColor, m_selectedInfoOverlayColor);
            setDictionaryValue(dict, SelectedInfoOverlayFadeDistance, m_selectedInfoOverlayFadeDistance);
            setDictionaryValue(dict, SelectedTextureColor, m_selectedTextureColor);
            setDictionaryValue(dict, UsedTextureColor, m_usedTextureColor);
            setDictionaryValue(dict, OverriddenTextureColor, m_overriddenTextureColor);
            setDictionaryValue(dict, RendererFontName, m_rendererFontName);
            setDictionaryValue(dict, RendererFontSize, m_rendererFontSize);
            setDictionaryValue(dict, QuakePath, m_quakePath);
            setDictionaryValue(dict, VertexHandleSize, m_vertexHandleSize);
            setDictionaryValue(dict, VertexHandleColor, m_vertexHandleColor);
            setDictionaryValue(dict, HiddenVertexHandleColor, m_hiddenVertexHandleColor);
            setDictionaryValue(dict, SelectedVertexHandleColor, m_selectedVertexHandleColor);
            setDictionaryValue(dict, HiddenSelectedVertexHandleColor, m_hiddenSelectedVertexHandleColor);
            setDictionaryValue(dict, EdgeHandleColor, m_edgeHandleColor);
            setDictionaryValue(dict, HiddenEdgeHandleColor, m_hiddenEdgeHandleColor);
            setDictionaryValue(dict, SelectedEdgeHandleColor, m_selectedEdgeHandleColor);
            setDictionaryValue(dict, HiddenSelectedEdgeHandleColor, m_hiddenSelectedEdgeHandleColor);
            setDictionaryValue(dict, FaceHandleColor, m_faceHandleColor);
            setDictionaryValue(dict, HiddenFaceHandleColor, m_hiddenFaceHandleColor);
            setDictionaryValue(dict, SelectedFaceHandleColor, m_selectedFaceHandleColor);
            setDictionaryValue(dict, HiddenSelectedFaceHandleColor, m_hiddenSelectedFaceHandleColor);

            NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
            [defaults registerDefaults:dict];
            [dict release];
        }
        
        void MacPreferences::loadPlatformDefaults() {
            m_cameraKey = Controller::TB_MK_SHIFT;
            m_cameraOrbitKey = Controller::TB_MK_SHIFT | Controller::TB_MK_CMD;
            m_selectionToolMultiKey = Controller::TB_MK_CMD;
            m_selectionToolGridKey = Controller::TB_MK_ALT;
            m_resizeToolKey = Controller::TB_MK_CMD;
            m_rendererFontName = "LucidaGrande";
            m_rendererFontSize = 11;
            m_quakePath = "";
        }
        
        bool MacPreferences::loadInt(const std::string& key, int& value) {
            value =  static_cast<int>([[NSUserDefaults standardUserDefaults] integerForKey:[NSString stringWithCString:key.c_str() encoding:NSASCIIStringEncoding]]);
            return true;
        }
        
        bool MacPreferences::loadFloat(const std::string& key, float& value) {
            value = [[NSUserDefaults standardUserDefaults] floatForKey:[NSString stringWithCString:key.c_str() encoding:NSASCIIStringEncoding]];
            return true;
        }
        
        bool MacPreferences::loadBool(const std::string& key, bool& value) {
            value = [[NSUserDefaults standardUserDefaults] boolForKey:[NSString stringWithCString:key.c_str() encoding:NSASCIIStringEncoding]] == YES ? true : false;
            return true;
        }
        
        bool MacPreferences::loadString(const std::string& key, std::string& value) {
            NSString* objcValue = [[NSUserDefaults standardUserDefaults] stringForKey:[NSString stringWithCString:key.c_str() encoding:NSASCIIStringEncoding]];
            if (objcValue == nil)
                value = "";
            else
                value = [objcValue cStringUsingEncoding:NSASCIIStringEncoding];
            return true;
        }
        
        bool MacPreferences::loadVec3f(const std::string& key, Vec3f& value) {
            NSString* objcValue = [[NSUserDefaults standardUserDefaults] stringForKey:[NSString stringWithCString:key.c_str() encoding:NSASCIIStringEncoding]];
            if (objcValue == nil)
                value = Vec3f();
            else
                value = Vec3f([objcValue cStringUsingEncoding:NSASCIIStringEncoding]);
            return true;
        }

        bool MacPreferences::loadVec4f(const std::string& key, Vec4f& value) {
            NSString* objcValue = [[NSUserDefaults standardUserDefaults] stringForKey:[NSString stringWithCString:key.c_str() encoding:NSASCIIStringEncoding]];
            if (objcValue == nil)
                value = Vec4f();
            else
                value = Vec4f([objcValue cStringUsingEncoding:NSASCIIStringEncoding]);
            return true;
        }

        void MacPreferences::saveInt(const std::string& key, int value) {
            [[NSUserDefaults standardUserDefaults] setInteger:value 
                                                       forKey:[NSString stringWithCString:key.c_str() encoding:NSASCIIStringEncoding]];
        }
        
        void MacPreferences::saveFloat(const std::string& key, float value) {
            [[NSUserDefaults standardUserDefaults] setFloat:value 
                                                     forKey:[NSString stringWithCString:key.c_str() encoding:NSASCIIStringEncoding]];
        }
        
        void MacPreferences::saveBool(const std::string& key, bool value) {
            [[NSUserDefaults standardUserDefaults] setBool:value 
                                                     forKey:[NSString stringWithCString:key.c_str() encoding:NSASCIIStringEncoding]];
        }
        
        void MacPreferences::saveString(const std::string& key, const std::string& value) {
            [[NSUserDefaults standardUserDefaults] setObject:[NSString stringWithCString:value.c_str() encoding:NSASCIIStringEncoding] 
                                                      forKey:[NSString stringWithCString:key.c_str() encoding:NSASCIIStringEncoding]];
        }

        void MacPreferences::saveVec4f(const std::string& key, const Vec4f& value) {
            saveString(key, value.asString());
        }

        void MacPreferences::saveVec3f(const std::string& key, const Vec3f& value) {
            saveString(key, value.asString());
        }
        
        bool MacPreferences::saveInstantly() {
            return true;
        }
    }
}