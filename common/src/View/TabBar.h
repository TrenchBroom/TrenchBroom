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

#ifndef __TrenchBroom__TabBar__
#define __TrenchBroom__TabBar__

#include "View/ContainerBar.h"

#include <vector>

class wxButton;
class wxBookCtrlEvent;
class wxSimplebook;

namespace TrenchBroom {
    namespace View {
        class TabBook;
        class TabBookPage;
        
        class TabBar : public ContainerBar {
        private:
            typedef std::vector<wxButton*> ButtonList;
            
            TabBook* m_tabBook;
            wxSimplebook* m_barBook;
            ButtonList m_buttons;
        public:
            TabBar(wxWindow* parent, TabBook* tabBook);
            
            void addTab(const wxString& title, TabBookPage* bookPage);
            
            void OnButtonClicked(wxCommandEvent& event);
            void OnTabBookPageChanged(wxBookCtrlEvent& event);
        private:
            size_t findButtonIndex(wxWindow* button) const;
            void setButtonActive(int index);
            void setButtonInactive(int index);
        };
    }
}

#endif /* defined(__TrenchBroom__TabBar__) */
