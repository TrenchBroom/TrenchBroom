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

#ifndef TrenchBroom_MenuCommandIds_h
#define TrenchBroom_MenuCommandIds_h

#include <wx/wx.h>

namespace TrenchBroom {
    namespace View {
        namespace MenuCommandIds {
            static const int tbID_MENU_LOWEST               = wxID_HIGHEST +  0;
            static const int tbID_EDIT_SELECT_ALL           = wxID_HIGHEST +  0;
            static const int tbID_EDIT_SELECT_SIBLINGS      = wxID_HIGHEST +  1;
            static const int tbID_EDIT_SELECT_TOUCHING      = wxID_HIGHEST +  2;
            static const int tbID_EDIT_SELECT_NONE          = wxID_HIGHEST +  3;
            static const int tbID_EDIT_HIDE_SELECTED        = wxID_HIGHEST +  4;
            static const int tbID_EDIT_HIDE_UNSELECTED      = wxID_HIGHEST +  5;
            static const int tbID_EDIT_UNHIDE_ALL           = wxID_HIGHEST +  6;
            static const int tbID_EDIT_LOCK_SELECTED        = wxID_HIGHEST +  7;
            static const int tbID_EDIT_LOCK_UNSELECTED      = wxID_HIGHEST +  8;
            static const int tbID_EDIT_UNLOCK_ALL           = wxID_HIGHEST +  9;
            static const int tbID_MENU_HIGHEST              = wxID_HIGHEST + 99;
        }
    }
}

#endif
