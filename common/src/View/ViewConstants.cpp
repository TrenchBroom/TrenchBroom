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

#include "ViewConstants.h"

#include <wx/settings.h>
#include <wx/font.h>

namespace TrenchBroom {
    namespace View {
        namespace Fonts {
            const wxFont& fixedWidthFont() {
                static const wxFont font =
#if defined __APPLE__
                wxFont(wxFontInfo().FaceName("Monaco")).Smaller();
#elif defined _WIN32
                wxFont(wxFontInfo().FaceName("Lucida Console"));
#else
                wxFont(wxFontInfo().Family(wxFONTFAMILY_MODERN)).Smaller().Smaller();
#endif
                return font;
            }
        }

        namespace Colors {
            const wxColour& defaultText() {
                static const wxColour col = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT);
                return col;
            }

            const wxColour& highlightText() {
                // Used for selected tabs of TabBar control.
                static const wxColour col =
#if defined __APPLE__
                wxColour(26, 79, 189);
#else
                wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
#endif
                return col;
            }

            const wxColour& disabledText() {
                static const wxColour col =
#if defined __APPLE__
                wxColour(108, 108, 108);
#else
                wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);
#endif
                return col;
            }

            const wxColour& borderColor() {
                static const wxColour col =
#if defined __APPLE__
                wxColour(67, 67, 67);
#else
                *wxBLACK;
#endif
                return col;
            }

            const wxColour& separatorColor() {
                static const wxColour col =
#if defined __APPLE__
                wxColour(183, 183, 183);
#else
                *wxLIGHT_GREY;
#endif
                return col;
            }
        }
    }
}
