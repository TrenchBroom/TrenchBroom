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

#ifndef TrenchBroom_WelcomeFrame
#define TrenchBroom_WelcomeFrame

#include "IO/Path.h"

#include <wx/frame.h>

class wxButton;
class wxPanel;

namespace TrenchBroom {
    namespace View {
        class RecentDocumentListBox;
        class RecentDocumentSelectedCommand;
        
        class WelcomeFrame : public wxFrame {
        private:
            RecentDocumentListBox* m_recentDocumentListBox;
            wxButton* m_createNewDocumentButton;
            wxButton* m_openOtherDocumentButton;
        public:
            WelcomeFrame();
            
            void OnCreateNewDocumentClicked(wxCommandEvent& event);
            void OnOpenOtherDocumentClicked(wxCommandEvent& event);
            void OnRecentDocumentSelected(RecentDocumentSelectedCommand& event);

            DECLARE_DYNAMIC_CLASS(WelcomeFrame)
        private:
            void createGui();
            wxPanel* createAppPanel(wxWindow* parent);
            void bindEvents();
        };
    }
}

#endif /* defined(TrenchBroom_WelcomeFrame) */
