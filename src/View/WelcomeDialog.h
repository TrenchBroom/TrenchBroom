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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__WelcomeDialog__
#define __TrenchBroom__WelcomeDialog__

#include "IO/Path.h"

#include <wx/dialog.h>

class wxButton;
class wxPanel;

namespace TrenchBroom {
    namespace View {
        class RecentDocumentListBox;
        class RecentDocumentSelectedCommand;
        
        class WelcomeDialog : public wxDialog {
        public:
            static const int CreateNewDocument = 666;
            static const int OpenDocument = 667;
        private:
            RecentDocumentListBox* m_recentDocumentListBox;
            wxButton* m_createNewDocumentButton;
            wxButton* m_openOtherDocumentButton;
            IO::Path m_documentPath;
        public:
            WelcomeDialog();
            
            const IO::Path& documentPath() const;
            
            void OnCreateNewDocumentClicked(wxCommandEvent& event);
            void OnOpenOtherDocumentClicked(wxCommandEvent& event);
            void OnRecentDocumentSelected(RecentDocumentSelectedCommand& event);
        private:
            void createGui();
            wxPanel* createAppPanel(wxWindow* parent);
            void bindEvents();
        };
    }
}

#endif /* defined(__TrenchBroom__WelcomeDialog__) */
