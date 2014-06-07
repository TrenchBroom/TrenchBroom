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

#ifndef __TrenchBroom__CollapsibleTitledPanel__
#define __TrenchBroom__CollapsibleTitledPanel__

#include "View/TitleBar.h"

class wxStaticText;

namespace TrenchBroom {
    namespace View {
        class BorderLine;
        
        class CollapsibleTitleBar : public TitleBar {
        private:
            wxStaticText* m_stateText;
        public:
            CollapsibleTitleBar(wxWindow* parent, const wxString& title, const wxString& stateText);
            
            void setStateText(const wxString& stateText);
            
            void OnClick(wxMouseEvent& event);
        };
        
        class CollapsibleTitledPanel : public wxPanel {
        private:
            CollapsibleTitleBar* m_titleBar;
            BorderLine* m_divider;
            wxPanel* m_panel;
            bool m_expanded;
        public:
            CollapsibleTitledPanel(wxWindow* parent, const wxString& title, bool initiallyExpanded = true);
            
            wxWindow* getPanel() const;

            void expand();
            void collapse();
            bool expanded() const;
            void setExpanded(bool expanded);
            
            void OnTitleBarClicked(wxMouseEvent& event);
        private:
            void update();
        };
    }
}

#endif /* defined(__TrenchBroom__CollapsibleTitledPanel__) */
