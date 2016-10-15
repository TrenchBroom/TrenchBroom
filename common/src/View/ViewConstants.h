/*
 Copyright (C) 2010-2016 Kristian Duske
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_ViewConstants_h
#define TrenchBroom_ViewConstants_h

class wxColour;
class wxFont;

namespace TrenchBroom {
    namespace View {
        namespace LayoutConstants {
#if defined _WIN32
            static const int DialogOuterMargin                  = 10;
            static const int DialogButtonTopMargin              = 10;
            static const int DialogButtonLeftMargin             = 7;
            static const int DialogButtonRightMargin            = 7;
            static const int DialogButtonBottomMargin           = 10;
            static const int WideHMargin                        = 8;
            static const int MediumHMargin                      = 6;
            static const int NarrowHMargin                      = 4;
            static const int WideVMargin                        = 4;
            static const int NarrowVMargin                      = 2;
            static const int StaticBoxSideMargin                = 15;
            static const int StaticBoxTopMargin                 = 20;
            static const int StaticBoxBottomMargin              = 15;
            static const int ChoiceTopMargin                    = 1;
            static const int ChoiceLeftMargin                   = 0;
            static const int ChoiceSizeDelta                    = 0;
            static const int TextBoxInnerMargin                 = 5;
            static const int TabBarBarLeftMargin                = 10;
            static const int ToggleButtonStyle                  = 0;
#elif defined __APPLE__
            static const int DialogOuterMargin                  = 10;
            static const int DialogButtonTopMargin              = 0;
            static const int DialogButtonLeftMargin             = 12;
            static const int DialogButtonRightMargin            = 0;
            static const int DialogButtonBottomMargin           = 3;
            static const int WideHMargin                        = 8;
            static const int MediumHMargin                      = 4;
            static const int NarrowHMargin                      = 2;
            static const int WideVMargin                        = 4;
            static const int NarrowVMargin                      = 2;
            static const int StaticBoxSideMargin                = 10;
            static const int StaticBoxTopMargin                 = 10;
            static const int StaticBoxBottomMargin              = 10;
            static const int ChoiceTopMargin                    = 1;
            static const int ChoiceLeftMargin                   = 1;
            static const int ChoiceSizeDelta                    = 1;
            static const int TextBoxInnerMargin                 = 0;
            static const int TabBarBarLeftMargin                = 10;
            static const int ToggleButtonStyle                  = 0x08000000; // wxBORDER_SUNKEN
#elif defined __linux__ || defined __FreeBSD__
            static const int DialogOuterMargin                  = 10;
            static const int DialogButtonTopMargin              = 10;
            static const int DialogButtonLeftMargin             = 8;
            static const int DialogButtonRightMargin            = 0;
            static const int DialogButtonBottomMargin           = 10;
            static const int WideHMargin                        = 8;
            static const int MediumHMargin                      = 4;
            static const int NarrowHMargin                      = 2;
            static const int WideVMargin                        = 4;
            static const int NarrowVMargin                      = 2;
            static const int StaticBoxSideMargin                = 15;
            static const int StaticBoxTopMargin                 = 10;
            static const int StaticBoxBottomMargin              = 30;
            static const int ChoiceTopMargin                    = 0;
            static const int ChoiceLeftMargin                   = 0;
            static const int ChoiceSizeDelta                    = 0;
            static const int TextBoxInnerMargin                 = 2;
            static const int TabBarBarLeftMargin                = 10;
            static const int ToggleButtonStyle                  = 0;
#endif
            static const int MinPreferenceLabelWidth            = 100;
            static const int HighlightBoxMargin                 = 5;
        }
        
        namespace Fonts {
            const wxFont& fixedWidthFont();
        }
        
        namespace Colors {
            const wxColour& defaultText();
            const wxColour& highlightText();
            const wxColour& disabledText();
            const wxColour& borderColor();
            const wxColour& separatorColor();
        }
    }
}

#endif
