/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef TrenchBroom_MapViewBar
#define TrenchBroom_MapViewBar

#include "View/ContainerBar.h"
#include "View/ViewTypes.h"

class wxBookCtrlBase;
class wxSearchCtrl;
class wxStaticText;

namespace TrenchBroom {
    namespace View {
        class ViewPopupEditor;
        
        class MapViewBar : public ContainerBar {
        private:
            MapDocumentWPtr m_document;
            wxBookCtrlBase* m_toolBook;
            ViewPopupEditor* m_viewEditor;
        public:
            MapViewBar(wxWindow* parent, MapDocumentWPtr document);
            
            wxBookCtrlBase* toolBook();
            
            void OnSearchPatternChanged(wxCommandEvent& event);
        private:
            void createGui(MapDocumentWPtr document);
        };
    }
}

#endif /* defined(TrenchBroom_MapViewBar) */
