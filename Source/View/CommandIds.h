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
                static const int EditSelectAll              = wxID_HIGHEST +  2;
                static const int EditSelectSiblings         = wxID_HIGHEST +  3;
                static const int EditSelectTouching         = wxID_HIGHEST +  4;
                static const int EditSelectNone             = wxID_HIGHEST +  5;
                static const int EditHideSelected           = wxID_HIGHEST +  6;
                static const int EditHideUnselected         = wxID_HIGHEST +  7;
                static const int EditUnhideAll              = wxID_HIGHEST +  8;
                static const int EditLockSelected           = wxID_HIGHEST +  9;
                static const int EditLockUnselected         = wxID_HIGHEST + 10;
                static const int EditUnlockAll              = wxID_HIGHEST + 11;
                static const int Highest                    = wxID_HIGHEST + 99;
            }

            namespace ViewInspector {
                static const int Lowest                             = Menu::Highest +  1;
                static const int ShowEntitiesCheckBoxId             = Menu::Highest +  1;
                static const int ShowEntityModelsCheckBoxId         = Menu::Highest +  2;
                static const int ShowEntityBoundsCheckBoxId         = Menu::Highest +  3;
                static const int ShowEntityClassnamesCheckBoxId     = Menu::Highest +  4;
                static const int ShowBrushesCheckBoxId              = Menu::Highest +  5;
                static const int ShowClipBrushesCheckBoxId          = Menu::Highest +  6;
                static const int ShowSkipBrushesCheckBoxId          = Menu::Highest +  7;
                static const int FaceRenderModeChoiceId             = Menu::Highest +  8;
                static const int RenderEdgesCheckBoxId              = Menu::Highest +  9;
                static const int Highest                            = Menu::Highest + 99;
            }
            
            namespace PreferencesDialog {
                static const int Lowest                             = ViewInspector::Highest +  1;
                static const int ChooseQuakePathButtonId            = ViewInspector::Highest +  1;
                static const int BrightnessSliderId                 = ViewInspector::Highest +  2;
                static const int GridAlphaSliderId                  = ViewInspector::Highest +  3;
                static const int LookSpeedSliderId                  = ViewInspector::Highest +  4;
                static const int InvertLookXAxisCheckBoxId          = ViewInspector::Highest +  5;
                static const int InvertLookYAxisCheckBoxId          = ViewInspector::Highest +  6;
                static const int PanSpeedSliderId                   = ViewInspector::Highest +  7;
                static const int InvertPanXAxisCheckBoxId           = ViewInspector::Highest +  8;
                static const int InvertPanYAxisCheckBoxId           = ViewInspector::Highest +  9;
                static const int MoveSpeedSliderId                  = ViewInspector::Highest + 10;
                static const int Highest                            = ViewInspector::Highest + 99;
            }
        }
    }
}

#endif
