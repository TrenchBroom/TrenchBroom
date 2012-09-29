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

#ifndef TrenchBroom_CommandIds_h
#define TrenchBroom_CommandIds_h

#include <wx/wx.h>

namespace TrenchBroom {
    namespace View {
        namespace CommandIds {
            namespace Menu {
                static const int Lowest                     = wxID_HIGHEST +  1;
                static const int EditSelectAll              = Lowest +  2;
                static const int EditSelectSiblings         = Lowest +  3;
                static const int EditSelectTouching         = Lowest +  4;
                static const int EditSelectNone             = Lowest +  5;
                static const int EditHideSelected           = Lowest +  6;
                static const int EditHideUnselected         = Lowest +  7;
                static const int EditUnhideAll              = Lowest +  8;
                static const int EditLockSelected           = Lowest +  9;
                static const int EditLockUnselected         = Lowest + 10;
                static const int EditUnlockAll              = Lowest + 11;
                static const int Highest                    = Lowest + 99;
            }

            namespace ViewInspector {
                static const int Lowest                             = Menu::Highest +  1;
                static const int ShowEntitiesCheckBoxId             = Lowest +  1;
                static const int ShowEntityModelsCheckBoxId         = Lowest +  2;
                static const int ShowEntityBoundsCheckBoxId         = Lowest +  3;
                static const int ShowEntityClassnamesCheckBoxId     = Lowest +  4;
                static const int ShowBrushesCheckBoxId              = Lowest +  5;
                static const int ShowClipBrushesCheckBoxId          = Lowest +  6;
                static const int ShowSkipBrushesCheckBoxId          = Lowest +  7;
                static const int FaceRenderModeChoiceId             = Lowest +  8;
                static const int RenderEdgesCheckBoxId              = Lowest +  9;
                static const int Highest                            = Lowest + 99;
            }
            
            namespace PreferencesDialog {
                static const int Lowest                             = ViewInspector::Highest +  1;
                static const int ChooseQuakePathButtonId            = Lowest +  1;
                static const int BrightnessSliderId                 = Lowest +  2;
                static const int GridAlphaSliderId                  = Lowest +  3;
                static const int LookSpeedSliderId                  = Lowest +  4;
                static const int InvertLookXAxisCheckBoxId          = Lowest +  5;
                static const int InvertLookYAxisCheckBoxId          = Lowest +  6;
                static const int PanSpeedSliderId                   = Lowest +  7;
                static const int InvertPanXAxisCheckBoxId           = Lowest +  8;
                static const int InvertPanYAxisCheckBoxId           = Lowest +  9;
                static const int MoveSpeedSliderId                  = Lowest + 10;
                static const int Highest                            = Lowest + 99;
            }
            
            namespace FaceInspector {
                static const int Lowest                             = PreferencesDialog::Highest + 1;
                static const int TextureBrowserSortOrderChoiceId    = Lowest +  1;
                static const int TextureBrowserGroupButtonId        = Lowest +  2;
                static const int TextureBrowserUsedButtonId         = Lowest +  3;
                static const int TextureBrowserFilterBoxId          = Lowest +  4;
                static const int TextureBrowserId                   = Lowest +  5;
                static const int XOffsetEditorId                    = Lowest +  6;
                static const int YOffsetEditorId                    = Lowest +  7;
                static const int XScaleEditorId                     = Lowest +  8;
                static const int YScaleEditorId                     = Lowest +  9;
                static const int RotationEditorId                   = Lowest + 10;
                static const int TextureCollectionListId            = Lowest + 11;
                static const int AddTextureCollectionButtonId       = Lowest + 12;
                static const int RemoveTextureCollectionsButtonId   = Lowest + 13;
                static const int Highest                            = Lowest + 99;
            }
            
            namespace EntityInspector {
                static const int Lowest                             = FaceInspector::Highest + 1;
                static const int EntityPropertyViewId               = Lowest + 1;
                static const int AddEntityPropertyButtonId          = Lowest + 2;
                static const int RemoveEntityPropertiesButtonId     = Lowest + 3;
                static const int Highest                            = Lowest + 99;
            }
        }
    }
}

#endif
