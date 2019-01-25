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

#ifndef TrenchBroom_PopupButton
#define TrenchBroom_PopupButton

#include <wx/panel.h>

class wxToggleButton;
class wxCommandEvent;
class wxShowEvent;
class QWidget;

namespace TrenchBroom {
    namespace View {
        class PopupWindow;
        
        class PopupButton : public QWidget {
        private:
            wxToggleButton* m_button;
            PopupWindow* m_window;
        public:
            PopupButton(QWidget* parent, const QString& caption);
            
            QWidget* GetPopupWindow() const;

            void OnButtonToggled(wxCommandEvent& event);
            void OnPopupShow(wxShowEvent& event);
            bool Enable(bool enable = true) override;
        };
    }
}

#endif /* defined(TrenchBroom_PopupButton) */
