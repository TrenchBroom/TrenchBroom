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

#ifndef TrenchBroom_InfoPanel
#define TrenchBroom_InfoPanel

#include "View/ViewTypes.h"

#include <wx/bitmap.h>
#include <wx/panel.h>

class wxBookCtrlEvent;
class wxButton;
class wxCommandEvent;
class wxSimplebook;
class wxWindow;

namespace TrenchBroom {
    class Logger;

    namespace View {
        class ContainerBar;
        class Console;
        class IssueBrowser;
        class TabBook;
        
        class InfoPanel : public wxPanel {
        private:
            TabBook* m_tabBook;
            Console* m_console;
            IssueBrowser* m_issueBrowser;
        public:
            InfoPanel(wxWindow* parent, MapDocumentWPtr document);
            Console* console() const;
        };
    }
}

#endif /* defined(TrenchBroom_InfoPanel) */
