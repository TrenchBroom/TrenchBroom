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

#include "AboutDialog.h"

#include "StringUtils.h"
#include "View/AppInfoPanel.h"
#include "View/wxUtils.h"

#include <wx/button.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include <iostream>

namespace TrenchBroom {
    namespace View {
        AboutDialog* AboutDialog::instance = NULL;

        void AboutDialog::showAboutDialog() {
            if (AboutDialog::instance == NULL) {
                AboutDialog::instance = new AboutDialog();
                AboutDialog::instance->Show();
            } else {
                AboutDialog::instance->Raise();
            }
        }
        
        void AboutDialog::closeAboutDialog() {
            if (AboutDialog::instance != NULL)
                AboutDialog::instance->Close();
        }

        AboutDialog::~AboutDialog() {
            instance = NULL;
        }

        void AboutDialog::OnClickUrl(wxMouseEvent& event) {
            if (IsBeingDeleted()) return;

            const wxVariant* var = static_cast<wxVariant*>(event.GetEventUserData());
            const wxString url = var->GetString();
            ::wxLaunchDefaultBrowser(url);
        }

        AboutDialog::AboutDialog() :
        wxDialog(NULL, wxID_ANY, "About TrenchBroom", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxDIALOG_NO_PARENT | wxSTAY_ON_TOP) {
            createGui();
            CenterOnScreen();
        }
        
        void AboutDialog::createGui() {
            setWindowIcon(this);
            
            AppInfoPanel* infoPanel = new AppInfoPanel(this);
            
            wxStaticText* creditsText = new wxStaticText(this, wxID_ANY, "");
            creditsText->SetLabelMarkup("<b>Developed by Kristian Duske</b>\n"
                                 "kristianduske.com/trenchbroom\n\n"
                                 "<b>Contributors</b>\n"
                                 "Corey Jones (Documentation)\n"
                                 "Eric Wasylishen (Code, bug fixes)\n"
                                 "Jonas Lund (Bug fixes)\n"
                                 "negke (FGD files)\n"
                                 "Philipp Nahratow (Bug fixes, Linux builds)\n"
                                 "rebb (Shaders, bug fixes)\n"
                                 "Rohit Nirmal (Bug fixes)\n"
                                 "Scampie (Documentation)\n\n"
                                 "<b>3rd Party Libraries, Tools and Assets</b>\n"
                                 "wxWidgets (Cross platform GUI library)\n"
                                 "FreeType (Font rendering library)\n"
                                 "FreeImage (Image loading & manipulation library)\n"
                                 "GLEW (OpenGL extension library)\n"
                                 "Google Test (C++ testing framework)\n"
                                 "Google Mock (C++ mocking framework)\n"
                                 "StackWalker (C++ stack trace analyzer)\n"
                                 "CMake (Cross platform build manager)\n"
                                 "Pandoc (Universal document converter)\n"
                                 "Source Sans Pro (Font)\n");

            wxSizer* outerSizer = new wxBoxSizer(wxHORIZONTAL);
            outerSizer->AddSpacer(50);
            outerSizer->Add(infoPanel, 0, wxALIGN_CENTER | wxBOTTOM, 20);
            outerSizer->AddSpacer(50);
            outerSizer->Add(creditsText, 1, wxEXPAND | wxTOP | wxBOTTOM, 20);
            outerSizer->AddSpacer(50);
            
            SetSizerAndFit(outerSizer);

            wxAcceleratorEntry entries[1];
            entries[0] = wxAcceleratorEntry(wxACCEL_NORMAL, WXK_ESCAPE, wxID_CANCEL);
            SetAcceleratorTable(wxAcceleratorTable(1, entries));
            
            Bind(wxEVT_MENU, &AboutDialog::OnCancel, this, wxID_CANCEL);
            Bind(wxEVT_CLOSE_WINDOW, &AboutDialog::OnClose, this);
        }

        wxStaticText* AboutDialog::createURLText(wxWindow* parent, const String& text, const String& tooltip, const String& url) {
            wxStaticText* statText = new wxStaticText(parent, wxID_ANY, text);
            statText->SetFont(statText->GetFont().Underlined());
            statText->SetForegroundColour(*wxBLUE);
            statText->SetToolTip(tooltip);
            statText->SetCursor(wxCURSOR_HAND);
            statText->Bind(wxEVT_LEFT_UP, &AboutDialog::OnClickUrl, this, wxID_ANY, wxID_ANY, new wxVariant(url));
            return statText;
        }

        void AboutDialog::OnCancel(wxCommandEvent& event) {
            Close();
        }

        void AboutDialog::OnClose(wxCloseEvent& event) {
            Destroy();
        }
    }
}
