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

#include "AboutFrame.h"

#include "StringUtils.h"
#include "View/AppInfoPanel.h"

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include <iostream>

namespace TrenchBroom {
    namespace View {
        AboutFrame* AboutFrame::instance = NULL;

        void AboutFrame::showAboutFrame() {
            if (AboutFrame::instance == NULL) {
                AboutFrame::instance = new AboutFrame();
                AboutFrame::instance->Show();
            } else {
                AboutFrame::instance->Raise();
            }
        }
        
        AboutFrame::~AboutFrame() {
            instance = NULL;
        }

        void AboutFrame::OnClickUrl(wxMouseEvent& event) {
            if (IsBeingDeleted()) return;

            const wxVariant* var = static_cast<wxVariant*>(event.GetEventUserData());
            const wxString url = var->GetString();
            ::wxLaunchDefaultBrowser(url);
        }

        AboutFrame::AboutFrame() :
        wxFrame(NULL, wxID_ANY, "About TrenchBroom", wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN) {
            createGui();
            CenterOnScreen();
        }
        
        void AboutFrame::createGui() {
            SetBackgroundColour(*wxWHITE);
            AppInfoPanel* infoPanel = new AppInfoPanel(this);
            
            wxStaticText* creditsText = new wxStaticText(this, wxID_ANY, "");
            creditsText->SetBackgroundColour(*wxWHITE);
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
                                 "CMake (Cross platform build manager)\n"
                                 "Pandoc (Universal document converter)\n"
                                 "Source Sans Pro (Font)\n");

            /*
            wxStaticText* creditsText = new wxStaticText(creditsPanel, wxID_ANY, "Developed by Kristian Duske");
            creditsText->SetFont(creditsText->GetFont().Bold());
            
            wxStaticText* creditsUrl = createURLText(creditsPanel, "kristianduske.com/trenchbroom", "TrenchBroom Website", "http://www.kristianduske.com/trenchbroom");
            
            wxStaticText* thirdPartyHeader = new wxStaticText(creditsPanel, wxID_ANY, "3rd Party Libraries, Tools and Assets");
            thirdPartyHeader->SetFont(thirdPartyHeader->GetFont().Bold());
            
            wxStaticText* wxWidgetsText = createURLText(creditsPanel, "wxWidgets", "Cross-Platform GUI Library", "http://www.wxwidgets.org");
            wxStaticText* freetypeText = createURLText(creditsPanel, "FreeType", "The FreeType Project", "http://www.freetype.org");
            wxStaticText* freeimageText = createURLText(creditsPanel, "FreeImage", "The FreeImage Project", "http://freeimage.sourceforge.net");
            wxStaticText* glewText = createURLText(creditsPanel, "GLEW", "The OpenGL Extension Wrangler Library", "http://glew.sourceforge.net");
            wxStaticText* gtestText = createURLText(creditsPanel, "Google Test", "Google C++ Testing Framework", "http://code.google.com/p/googletest/");
            wxStaticText* gmockText = createURLText(creditsPanel, "Google Mock", "Google C++ Mocking Framework", "http://code.google.com/p/googlemock/");
            wxStaticText* cmakeText = createURLText(creditsPanel, "CMake", "Cross-Platform Make", "http://www.cmake.org");
            wxStaticText* fontText = createURLText(creditsPanel, "Source Sans Pro", "Sans serif font family for user interface environments", "https://github.com/adobe/source-sans-pro");
            wxStaticText* pandocText = createURLText(creditsPanel, "Pandoc", "Universal Document Converter", "http://www.pandoc.org/");
            
            wxSizer* creditsSizer = new wxBoxSizer(wxVERTICAL);
            creditsSizer->AddSpacer(20);
            creditsSizer->Add(creditsText);
            creditsSizer->Add(creditsUrl);
            creditsSizer->AddSpacer(20);
            creditsSizer->Add(thirdPartyHeader);
            creditsSizer->Add(wxWidgetsText);
            creditsSizer->Add(freetypeText);
            creditsSizer->Add(freeimageText);
            creditsSizer->Add(glewText);
            creditsSizer->Add(gtestText);
            creditsSizer->Add(gmockText);
            creditsSizer->Add(cmakeText);
            creditsSizer->Add(pandocText);
            creditsSizer->Add(fontText);
            creditsPanel->SetSizer(creditsSizer);
             */
            
            wxSizer* outerSizer = new wxBoxSizer(wxHORIZONTAL);
            outerSizer->AddSpacer(50);
            outerSizer->Add(infoPanel, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 20);
            outerSizer->AddSpacer(50);
            outerSizer->Add(creditsText, 1, wxEXPAND | wxTOP | wxBOTTOM, 20);
            outerSizer->AddSpacer(50);
            
            SetSizerAndFit(outerSizer);
        }

        wxStaticText* AboutFrame::createURLText(wxWindow* parent, const String& text, const String& tooltip, const String& url) {
            wxStaticText* statText = new wxStaticText(parent, wxID_ANY, text);
            statText->SetFont(statText->GetFont().Underlined());
            statText->SetForegroundColour(*wxBLUE);
            statText->SetToolTip(tooltip);
            statText->SetCursor(wxCURSOR_HAND);
            statText->Bind(wxEVT_LEFT_UP, &AboutFrame::OnClickUrl, this, wxID_ANY, wxID_ANY, new wxVariant(url));
            return statText;
        }
    }
}
