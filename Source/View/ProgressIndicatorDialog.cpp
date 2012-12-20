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

#include "ProgressIndicatorDialog.h"

#include <wx/wx.h>
#include <wx/progdlg.h>

namespace TrenchBroom {
    namespace View {
        void ProgressIndicatorDialog::doReset() {
            m_dialog->Update(0);
        }
        
        void ProgressIndicatorDialog::doUpdate() {
            m_dialog->Update(static_cast<int>(percent()));
        }

        ProgressIndicatorDialog::ProgressIndicatorDialog() {
            m_dialog = new wxProgressDialog("Progress", "Please wait...", 100, NULL, wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_SMOOTH);
        }
        
        ProgressIndicatorDialog::~ProgressIndicatorDialog() {
            m_dialog->Close();
            m_dialog->Destroy();
        }
        
        void ProgressIndicatorDialog::setText(const String& text) {
            m_dialog->Update(static_cast<int>(percent()), text);
        }
        
        void ProgressIndicatorDialog::pulse() {
            m_dialog->Pulse();
        }
    }
}
