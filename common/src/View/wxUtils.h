/*
 Copyright (C) 2010-2017 Kristian Duske

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

#ifndef TrenchBroom_wxUtils
#define TrenchBroom_wxUtils

#include "Color.h"
#include "StringUtils.h"

#include <vector>

#include <wx/colour.h>

class wxBitmapButton;
class wxBitmapToggleButton;
class wxCursor;
class wxFrame;
class wxListCtrl;
class wxSizer;
class wxTopLevelWindow;
class wxWindow;

namespace TrenchBroom {
    namespace View {
        class MapFrame;

        MapFrame* findMapFrame(wxWindow* window);
        wxFrame* findFrame(wxWindow* window);

        void fitAll(wxWindow* window);

        wxColor makeLighter(const wxColor& color);
        Color fromWxColor(const wxColor& color);
        wxColor toWxColor(const Color& color);

        std::vector<size_t> getListCtrlSelection(const wxListCtrl* listCtrl);
        void deselectAllListrCtrlItems(wxListCtrl* listCtrl);

        wxWindow* createBitmapButton(wxWindow* parent, const String& image, const String& tooltip);
        wxWindow* createBitmapToggleButton(wxWindow* parent, const String& upImage, const String& downImage, const String& tooltip);

        wxWindow* createDefaultPage(wxWindow* parent, const wxString& message);

        wxSizer* wrapDialogButtonSizer(wxSizer* buttonSizer, wxWindow* parent);

        void setWindowIcon(wxTopLevelWindow* window);

        template <typename T>
        void setHint(T* ctrl, const wxString& hint) {
#ifndef __WXGTK20__
            ctrl->SetHint(hint);
#endif
        }

        wxArrayString filterBySuffix(const wxArrayString& strings, const wxString& suffix, bool caseSensitive = false);
    }
}

#endif /* defined(TrenchBroom_wxUtils) */
