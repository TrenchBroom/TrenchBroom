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
#include "View/KeyboardShortcut.h"
#include "Utility/Color.h"
#include "Utility/MessageException.h"
#include "Utility/String.h"
#include "Utility/VecMath.h"

#include <wx/config.h>
#include <wx/confbase.h>

#include <limits>
#include <map>

using namespace TrenchBroom::Math;
using namespace TrenchBroom::Controller::ModifierKeys;
using namespace TrenchBroom::Controller::MouseButtons;

namespace TrenchBroom {
    namespace Preferences {
        using View::KeyboardShortcut;
        
        class PreferenceManager;

        template <typename T>
        class Converter {
        public:
            wxString toWxString(const T& value) const {
                return "";
            }

            T fromWxString(const wxString& string) const {
                return T();
            }
        };
        
        template <>
        class Converter<bool> {
        public:
            wxString toWxString(const bool& value) const {
                wxString string;
                string << (value ? 1 : 0);
                return string;
            }
            
            bool fromWxString(const wxString& string) const {
                long longValue;
                if (string.ToLong(&longValue))
                    return longValue != 0L;
                return false;
            }
        };
        
        template <>
        class Converter<int> {
        public:
            wxString toWxString(const int& value) const {
                wxString string;
                string << value;
                return string;
            }
            
            int fromWxString(const wxString& string) const {
                long longValue;
                if (string.ToLong(&longValue) && longValue >= std::numeric_limits<int>::min() && longValue <= std::numeric_limits<int>::max())
                    return static_cast<int>(longValue);
                return 0;
            }
        };
        
        template <>
        class Converter<float> {
        public:
            wxString toWxString(const float& value) const {
                wxString string;
                string << value;
                return string;
            }
            
            float fromWxString(const wxString& string) const {
                double doubleValue;
                if (string.ToDouble(&doubleValue) && doubleValue >= std::numeric_limits<float>::min() && doubleValue <= std::numeric_limits<float>::max())
                    return static_cast<float>(doubleValue);
                return 0.0f;
            }
        };
        
        template <>
        class Converter<String> {
        public:
            wxString toWxString(const String& value) const {
                return wxString(value);
            }
            
            String fromWxString(const wxString& string) const {
                return string.ToStdString();
            }
        };

        template <>
        class Converter<Color> {
        public:
            wxString toWxString(const Color& value) const {
                return wxString(value.asString());
            }
            
            Color fromWxString(const wxString& string) const {
                return Color(string.ToStdString());
            }
        };
        
        template<>
        class Converter<KeyboardShortcut> {
        public:
            wxString toWxString(const KeyboardShortcut& value) const {
                return wxString(value.asString());
            }
            
            KeyboardShortcut fromWxString(const wxString& string) const {
                return KeyboardShortcut(string.ToStdString());
            }
        };

        class ValueHolderBase {
        };
        
        template <typename T>
        class ValueHolder : public ValueHolderBase {
        private:
            T m_value;
        public:
            ValueHolder(T value) :
            m_value(value) {}
            
            inline const T& value() const {
                return m_value;
            }
        };
        
        class PreferenceBase {
        public:
            virtual ~PreferenceBase() {}
            
            virtual void load(wxConfigBase* config) const = 0;
            virtual void save(wxConfigBase* config) const = 0;
            virtual void setValue(const ValueHolderBase* valueHolder) const = 0;
        };
        
        template <typename T>
        class Preference : public PreferenceBase {
        protected:
            friend class PreferenceManager;

            Converter<T> m_converter;
            String m_name;
            mutable T m_value;
            mutable bool m_initialized;
            mutable bool m_modified;
            
            inline void setValue(const T& value) const {
                m_modified = true;
                m_value = value;
            }
            
            inline bool initialized() const {
                return m_initialized;
            }
            
            inline void load(wxConfigBase* config) const {
                wxString string;
                if (config->Read(m_name, &string))
                    m_value = m_converter.fromWxString(string);
                m_initialized = true;
            }
            
            inline void save(wxConfigBase* config) const {
                if (m_modified) {
                    wxString string = m_converter.toWxString(m_value);
                    bool success = config->Write(m_name, string);
                    assert(success);
                    m_modified = false;
                }
            }
        public:
            Preference(const String& name, const T& defaultValue) :
            m_name(name),
            m_value(defaultValue),
            m_initialized(false),
            m_modified(false) {
                m_modified = m_initialized;
            }
            
            void setValue(const ValueHolderBase* valueHolder) const {
                const ValueHolder<T>* actualValueHolder = static_cast<const ValueHolder<T>*>(valueHolder);
                setValue(actualValueHolder->value());
            }

            inline const String& name() const {
                return m_name;
            }
            
            inline const T& value() const {
                return m_value;
            }
        };
        
        extern const Preference<float>  CameraLookSpeed;
        extern const Preference<float>  CameraPanSpeed;
        extern const Preference<float>  CameraMoveSpeed;
        extern const Preference<bool>   CameraLookInvertX;
        extern const Preference<bool>   CameraLookInvertY;
        extern const Preference<bool>   CameraPanInvertX;
        extern const Preference<bool>   CameraPanInvertY;
        extern const Preference<bool>   CameraEnableAltMove;
        extern const Preference<bool>   CameraMoveInCursorDir;
        extern const Preference<float>  HandleRadius;
        extern const Preference<float>  MaximumHandleDistance;
        extern const Preference<float>  HandleScalingFactor;
        extern const Preference<float>  MaximumNearFaceDistance;
        extern const Preference<float>  CameraFieldOfVision;
        extern const Preference<float>  CameraNearPlane;
        extern const Preference<float>  CameraFarPlane;
        
        extern const Preference<float>  InfoOverlayFadeDistance;
        extern const Preference<float>  SelectedInfoOverlayFadeDistance;
        extern const Preference<int>    RendererFontSize;
        extern const Preference<float>  RendererBrightness;
        extern const Preference<float>  GridAlpha;
        extern const Preference<bool>   GridCheckerboard;
        
        extern const Preference<Color>  EntityRotationDecoratorColor;
        
        extern const Preference<Color>  XColor;
        extern const Preference<Color>  YColor;
        extern const Preference<Color>  ZColor;
        extern const Preference<Color>  BackgroundColor;
        
        extern const Preference<Color>  GuideColor;
        extern const Preference<Color>  HoveredGuideColor;
        
        extern const Preference<Color>  EntityLinkColor;
        extern const Preference<Color>  OccludedEntityLinkColor;
        extern const Preference<Color>  SelectedEntityLinkColor;
        extern const Preference<Color>  OccludedSelectedEntityLinkColor;
        
        extern const Preference<Color>  EntityKillLinkColor;
        extern const Preference<Color>  OccludedEntityKillLinkColor;
        extern const Preference<Color>  SelectedEntityKillLinkColor;
        extern const Preference<Color>  OccludedSelectedEntityKillLinkColor;
        
        extern const Preference<Color>  FaceColor;
        extern const Preference<Color>  SelectedFaceColor;
        extern const Preference<Color>  LockedFaceColor;
        extern const Preference<Color>  ClippedFaceColor;
        
        extern const Preference<Color>  EdgeColor;
        extern const Preference<Color>  SelectedEdgeColor;
        extern const Preference<Color>  OccludedSelectedEdgeColor;
        extern const Preference<Color>  LockedEdgeColor;
        extern const Preference<Color>  ClippedEdgeColor;
        extern const Preference<Color>  OccludedClippedEdgeColor;
        
        extern const Preference<Color>  SelectedEntityColor;
        extern const Preference<Color>  EntityBoundsColor;
        extern const Preference<Color>  SelectedEntityBoundsColor;
        extern const Preference<Color>  OccludedSelectedEntityBoundsColor;
        extern const Preference<Color>  LockedEntityColor;
        extern const Preference<Color>  LockedEntityBoundsColor;
        extern const Preference<Color>  EntityBoundsWireframeColor;
        
        extern const Preference<Color>  SelectionGuideColor;
        extern const Preference<Color>  OccludedSelectionGuideColor;
        
        extern const Preference<Color>  InfoOverlayTextColor;
        extern const Preference<Color>  InfoOverlayBackgroundColor;
        extern const Preference<Color>  OccludedInfoOverlayTextColor;
        extern const Preference<Color>  OccludedInfoOverlayBackgroundColor;
        extern const Preference<Color>  SelectedInfoOverlayTextColor;
        extern const Preference<Color>  SelectedInfoOverlayBackgroundColor;
        extern const Preference<Color>  OccludedSelectedInfoOverlayTextColor;
        extern const Preference<Color>  OccludedSelectedInfoOverlayBackgroundColor;
        extern const Preference<Color>  LockedInfoOverlayTextColor;
        extern const Preference<Color>  LockedInfoOverlayBackgroundColor;
        
        extern const Preference<Color>  VertexHandleColor;
        extern const Preference<Color>  OccludedVertexHandleColor;
        extern const Preference<Color>  SelectedVertexHandleColor;
        extern const Preference<Color>  OccludedSelectedVertexHandleColor;
        
        extern const Preference<Color>  SplitHandleColor;
        extern const Preference<Color>  OccludedSplitHandleColor;
        extern const Preference<Color>  SelectedSplitHandleColor;
        extern const Preference<Color>  OccludedSelectedSplitHandleColor;

        extern const Preference<Color>  EdgeHandleColor;
        extern const Preference<Color>  OccludedEdgeHandleColor;
        extern const Preference<Color>  SelectedEdgeHandleColor;
        extern const Preference<Color>  OccludedSelectedEdgeHandleColor;
        
        extern const Preference<Color>  FaceHandleColor;
        extern const Preference<Color>  OccludedFaceHandleColor;
        extern const Preference<Color>  SelectedFaceHandleColor;
        extern const Preference<Color>  OccludedSelectedFaceHandleColor;
        
        extern const Preference<Color>  ClipHandleColor;
        extern const Preference<Color>  OccludedClipHandleColor;
        extern const Preference<Color>  SelectedClipHandleColor;
        extern const Preference<Color>  ClipPlaneColor;
        
        extern const Preference<Color>  ResizeBrushFaceColor;
        extern const Preference<Color>  OccludedResizeBrushFaceColor;
        
        extern const Preference<Color>  BrowserTextColor;
        extern const Preference<Color>  SelectedTextureColor;
        extern const Preference<Color>  UsedTextureColor;
        extern const Preference<Color>  OverriddenTextureColor;
        extern const Preference<Color>  BrowserGroupBackgroundColor;
        extern const Preference<int>    TextureBrowserFontSize;
        extern const Preference<int>    EntityBrowserFontSize;
        extern const Preference<float>  TextureBrowserIconSize;
        
        extern const Preference<String> QuakePath;
        extern const Preference<String> RendererFontName;
        extern const Preference<int>    RendererInstancingMode;
        extern const int                RendererInstancingModeAutodetect;
        extern const int                RendererInstancingModeForceOn;
        extern const int                RendererInstancingModeForceOff;
        
        // File menu
        extern const Preference<KeyboardShortcut>   FileNew;
        extern const Preference<KeyboardShortcut>   FileOpen;
        extern const Preference<KeyboardShortcut>   FileSave;
        extern const Preference<KeyboardShortcut>   FileSaveAs;
        extern const Preference<KeyboardShortcut>   FileLoadPointFile;
        extern const Preference<KeyboardShortcut>   FileUnloadPointFile;
        extern const Preference<KeyboardShortcut>   FileClose;
 
        // Edit menu
        extern const Preference<KeyboardShortcut>   EditUndo;
        extern const Preference<KeyboardShortcut>   EditRedo;
        extern const Preference<KeyboardShortcut>   EditCut;
        extern const Preference<KeyboardShortcut>   EditCopy;
        extern const Preference<KeyboardShortcut>   EditPaste;
        extern const Preference<KeyboardShortcut>   EditPasteAtOriginalPosition;
        extern const Preference<KeyboardShortcut>   EditDelete;

        extern const Preference<KeyboardShortcut>   EditSelectAll;
        extern const Preference<KeyboardShortcut>   EditSelectSiblings;
        extern const Preference<KeyboardShortcut>   EditSelectTouching;
        extern const Preference<KeyboardShortcut>   EditSelectByFilePosition;
        extern const Preference<KeyboardShortcut>   EditSelectNone;
        extern const Preference<KeyboardShortcut>   EditHideSelected;
        extern const Preference<KeyboardShortcut>   EditHideUnselected;
        extern const Preference<KeyboardShortcut>   EditUnhideAll;
        extern const Preference<KeyboardShortcut>   EditLockSelected;
        extern const Preference<KeyboardShortcut>   EditLockUnselected;
        extern const Preference<KeyboardShortcut>   EditUnlockAll;
        extern const Preference<KeyboardShortcut>   EditToggleTextureLock;
        extern const Preference<KeyboardShortcut>   EditNavigateUp;
        extern const Preference<KeyboardShortcut>   EditShowMapProperties;
        
        // Edit > Tools menu
        extern const Preference<KeyboardShortcut>   EditToolsToggleClipTool;
        extern const Preference<KeyboardShortcut>   EditToolsToggleClipSide;
        extern const Preference<KeyboardShortcut>   EditToolsPerformClip;
        extern const Preference<KeyboardShortcut>   EditToolsToggleVertexTool;
        extern const Preference<KeyboardShortcut>   EditToolsToggleRotateTool;

        // Edit > Actions menu in face mode
        extern const Preference<KeyboardShortcut>   EditActionsMoveTexturesUp;
        extern const Preference<KeyboardShortcut>   EditActionsMoveTexturesDown;
        extern const Preference<KeyboardShortcut>   EditActionsMoveTexturesLeft;
        extern const Preference<KeyboardShortcut>   EditActionsMoveTexturesRight;
        extern const Preference<KeyboardShortcut>   EditActionsRotateTexturesCW;
        extern const Preference<KeyboardShortcut>   EditActionsRotateTexturesCCW;
        extern const Preference<KeyboardShortcut>   EditActionsMoveTexturesUpFine;
        extern const Preference<KeyboardShortcut>   EditActionsMoveTexturesDownFine;
        extern const Preference<KeyboardShortcut>   EditActionsMoveTexturesLeftFine;
        extern const Preference<KeyboardShortcut>   EditActionsMoveTexturesRightFine;
        extern const Preference<KeyboardShortcut>   EditActionsRotateTexturesCWFine;
        extern const Preference<KeyboardShortcut>   EditActionsRotateTexturesCCWFine;
        
        // Edit > Actions menu in objects mode
        extern const Preference<KeyboardShortcut>   EditActionsMoveObjectsForward;
        extern const Preference<KeyboardShortcut>   EditActionsMoveObjectsBackward;
        extern const Preference<KeyboardShortcut>   EditActionsMoveObjectsLeft;
        extern const Preference<KeyboardShortcut>   EditActionsMoveObjectsRight;
        extern const Preference<KeyboardShortcut>   EditActionsMoveObjectsUp;
        extern const Preference<KeyboardShortcut>   EditActionsMoveObjectsDown;
        extern const Preference<KeyboardShortcut>   EditActionsDuplicateObjectsForward;
        extern const Preference<KeyboardShortcut>   EditActionsDuplicateObjectsBackward;
        extern const Preference<KeyboardShortcut>   EditActionsDuplicateObjectsLeft;
        extern const Preference<KeyboardShortcut>   EditActionsDuplicateObjectsRight;
        extern const Preference<KeyboardShortcut>   EditActionsDuplicateObjectsUp;
        extern const Preference<KeyboardShortcut>   EditActionsDuplicateObjectsDown;
        extern const Preference<KeyboardShortcut>   EditActionsRollObjectsCW;
        extern const Preference<KeyboardShortcut>   EditActionsRollObjectsCCW;
        extern const Preference<KeyboardShortcut>   EditActionsYawObjectsCW;
        extern const Preference<KeyboardShortcut>   EditActionsYawObjectsCCW;
        extern const Preference<KeyboardShortcut>   EditActionsPitchObjectsCW;
        extern const Preference<KeyboardShortcut>   EditActionsPitchObjectsCCW;
        extern const Preference<KeyboardShortcut>   EditActionsFlipObjectsHorizontally;
        extern const Preference<KeyboardShortcut>   EditActionsFlipObjectsVertically;
        extern const Preference<KeyboardShortcut>   EditActionsDuplicateObjects;
        
        // Edit > Actions menu in vertex mode
        extern const Preference<KeyboardShortcut>   EditActionsMoveVerticesForward;
        extern const Preference<KeyboardShortcut>   EditActionsMoveVerticesBackward;
        extern const Preference<KeyboardShortcut>   EditActionsMoveVerticesLeft;
        extern const Preference<KeyboardShortcut>   EditActionsMoveVerticesRight;
        extern const Preference<KeyboardShortcut>   EditActionsMoveVerticesUp;
        extern const Preference<KeyboardShortcut>   EditActionsMoveVerticesDown;
        
        // Edit > Actions menu items in both objects and vertex mode
        extern const Preference<KeyboardShortcut>   EditActionsCorrectVertices;
        extern const Preference<KeyboardShortcut>   EditActionsSnapVertices;

        // View > Grid menu
        extern const Preference<KeyboardShortcut>   ViewGridToggleShowGrid;
        extern const Preference<KeyboardShortcut>   ViewGridToggleSnapToGrid;
        extern const Preference<KeyboardShortcut>   ViewGridIncGridSize;
        extern const Preference<KeyboardShortcut>   ViewGridDecGridSize;
        extern const Preference<KeyboardShortcut>   ViewGridSetSize1;
        extern const Preference<KeyboardShortcut>   ViewGridSetSize2;
        extern const Preference<KeyboardShortcut>   ViewGridSetSize4;
        extern const Preference<KeyboardShortcut>   ViewGridSetSize8;
        extern const Preference<KeyboardShortcut>   ViewGridSetSize16;
        extern const Preference<KeyboardShortcut>   ViewGridSetSize32;
        extern const Preference<KeyboardShortcut>   ViewGridSetSize64;
        extern const Preference<KeyboardShortcut>   ViewGridSetSize128;
        extern const Preference<KeyboardShortcut>   ViewGridSetSize256;
        
        // View > Camera menu
        extern const Preference<KeyboardShortcut>   ViewCameraMoveForward;
        extern const Preference<KeyboardShortcut>   ViewCameraMoveBackward;
        extern const Preference<KeyboardShortcut>   ViewCameraMoveLeft;
        extern const Preference<KeyboardShortcut>   ViewCameraMoveRight;
        extern const Preference<KeyboardShortcut>   ViewCameraMoveUp;
        extern const Preference<KeyboardShortcut>   ViewCameraMoveDown;
        extern const Preference<KeyboardShortcut>   ViewCameraMoveToNextPoint;
        extern const Preference<KeyboardShortcut>   ViewCameraMoveToPreviousPoint;
        extern const Preference<KeyboardShortcut>   ViewCameraCenterCameraOnSelection;
        
        // View menu
        extern const Preference<KeyboardShortcut>   ViewSwitchToEntityTab;
        extern const Preference<KeyboardShortcut>   ViewSwitchToFaceTab;
        extern const Preference<KeyboardShortcut>   ViewSwitchToViewTab;
        
        class PreferenceManager {
        private:
            typedef std::map<const PreferenceBase*, ValueHolderBase*> UnsavedPreferences;

            bool m_saveInstantly;
            UnsavedPreferences m_unsavedPreferences;

            PreferenceManager() {
#if defined __APPLE__
                m_saveInstantly = true;
#else
                m_saveInstantly = false;
#endif
            }
            
            void markAsUnsaved(const PreferenceBase* preference, ValueHolderBase* valueHolder) {
                UnsavedPreferences::iterator it = m_unsavedPreferences.find(preference);
                if (it == m_unsavedPreferences.end())
                    m_unsavedPreferences[preference] = valueHolder;
                else
                    delete valueHolder;
            }
            
        public:
            inline static PreferenceManager& preferences() {
                static PreferenceManager prefs;
                return prefs;
            }
            
            inline void save() {
                UnsavedPreferences::iterator it, end;
                for (it = m_unsavedPreferences.begin(), end = m_unsavedPreferences.end(); it != end; ++it) {
                    it->first->save(wxConfig::Get());
                    delete it->second;
                }
                
                m_unsavedPreferences.clear();
            }
            
            inline void discardChanges() {
                UnsavedPreferences::iterator it, end;
                for (it = m_unsavedPreferences.begin(), end = m_unsavedPreferences.end(); it != end; ++it) {
                    it->first->setValue(it->second);
                    delete it->second;
                }
                
                m_unsavedPreferences.clear();
            }
            
            inline bool getBool(const Preference<bool>& preference) const {
                if (!preference.initialized())
                    preference.load(wxConfig::Get());
                
                return preference.value();
            }
            
            inline void setBool(const Preference<bool>& preference, bool value) {
                bool previousValue = preference.value();
                preference.setValue(value);
                if (m_saveInstantly)
                    preference.save(wxConfig::Get());
                else
                    markAsUnsaved(&preference, new ValueHolder<bool>(previousValue));
            }
            
            inline int getInt(const Preference<int>& preference) const {
                if (!preference.initialized())
                    preference.load(wxConfig::Get());
                
                return preference.value();
            }
            
            inline void setInt(const Preference<int>& preference, int value) {
                int previousValue = preference.value();
                preference.setValue(value);
                if (m_saveInstantly)
                    preference.save(wxConfig::Get());
                else
                    markAsUnsaved(&preference, new ValueHolder<int>(previousValue));
            }
            
            inline float getFloat(const Preference<float>& preference) const {
                if (!preference.initialized())
                    preference.load(wxConfig::Get());
                
                return preference.value();
            }
            
            inline void setFloat(const Preference<float>& preference, float value) {
                float previousValue = preference.value();
                preference.setValue(value);
                if (m_saveInstantly)
                    preference.save(wxConfig::Get());
                else
                    markAsUnsaved(&preference, new ValueHolder<float>(previousValue));
            }
            
            inline const String& getString(const Preference<String>& preference) const {
                if (!preference.initialized())
                    preference.load(wxConfig::Get());
                
                return preference.value();
            }

            inline void setString(const Preference<String>& preference, const String& value) {
                String previousValue = preference.value();
                preference.setValue(value);
                if (m_saveInstantly)
                    preference.save(wxConfig::Get());
                else
                    markAsUnsaved(&preference, new ValueHolder<String>(previousValue));
            }
            
            inline const Color& getColor(const Preference<Color>& preference) const {
                if (!preference.initialized())
                    preference.load(wxConfig::Get());
                
                return preference.value();
            }
            
            inline void setColor(const Preference<Color>& preference, const Color& value) {
                Color previousValue = preference.value();
                preference.setValue(value);
                if (m_saveInstantly)
                    preference.save(wxConfig::Get());
                else
                    markAsUnsaved(&preference, new ValueHolder<Color>(previousValue));
            }
            
            inline const KeyboardShortcut& getKeyboardShortcut(const Preference<KeyboardShortcut>& preference) const {
                if (!preference.initialized())
                    preference.load(wxConfig::Get());
                
                return preference.value();
            }
            
            inline void setKeyboardShortcut(const Preference<KeyboardShortcut>& preference, const KeyboardShortcut& value) {
                KeyboardShortcut previousValue = preference.value();
                preference.setValue(value);
                if (m_saveInstantly)
                    preference.save(wxConfig::Get());
                else
                    markAsUnsaved(&preference, new ValueHolder<KeyboardShortcut>(previousValue));
            }
        };
    }
}

#endif /* defined(__TrenchBroom__Preferences__) */
