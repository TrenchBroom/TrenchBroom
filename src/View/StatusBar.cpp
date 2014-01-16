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

#include "StatusBar.h"

#include "IO/Path.h"
#include "IO/ResourceUtils.h"
#include "Model/IssueManager.h"
#include "View/Console.h"
#include "View/Grid.h"
#include "View/ImagePanel.h"
#include "View/MapDocument.h"

#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        StatusBar::StatusBar(wxWindow* parent, MapDocumentWPtr document, Console* console) :
        wxPanel(parent),
        m_document(document),
        m_gridSize(32),
        m_textureLock(true),
        m_issueCount(0) {
            wxStaticLine* line = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
         
            m_positionIconPanel = new ImagePanel(this);
            m_positionIconPanel->SetImage(IO::loadImageResource(IO::Path("images/Position.png")));
            m_positionText = new wxStaticText(this, wxID_ANY, "");
            m_positionText->SetMinSize(wxSize(100, wxDefaultSize.y));
            
            m_gridIconPanel = new ImagePanel(this);
            m_gridIconPanel->SetImage(IO::loadImageResource(IO::Path("images/Grid.png")));
            m_gridSizeText = new wxStaticText(this, wxID_ANY, "Size 32");
            m_gridSizeText->SetMinSize(wxSize(70, wxDefaultSize.y));
            
            m_textureLockOn = IO::loadImageResource(IO::Path("images/TextureLockOn.png"));
            m_textureLockOff = IO::loadImageResource(IO::Path("images/TextureLockOff.png"));
            m_textureLockIconPanel = new ImagePanel(this);
            m_textureLockIconPanel->SetImage(m_textureLockOn);
            m_textureLockText = new wxStaticText(this, wxID_ANY, "Texture Lock On");
            m_textureLockText->SetMinSize(wxSize(120, wxDefaultSize.y));
            
            m_issuesIconPanel = new ImagePanel(this);
            m_issuesIconPanel->SetImage(IO::loadImageResource(IO::Path("images/IssueBrowser.png")));
            m_issuesText = new wxStaticText(this, wxID_ANY, "0 Issues");
            m_issuesText->SetMinSize(wxSize(120, wxDefaultSize.y));
            
            m_message = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxELLIPSIZE_END | wxST_NO_AUTORESIZE);
            
            wxSizer* innerSizer = new wxBoxSizer(wxHORIZONTAL);
            innerSizer->AddSpacer(1);
            innerSizer->Add(m_positionIconPanel);
            innerSizer->Add(m_positionText);
            innerSizer->AddSpacer(1);
            innerSizer->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), 0, wxEXPAND);
            innerSizer->AddSpacer(1);
            
            innerSizer->Add(m_gridIconPanel);
            innerSizer->AddSpacer(1);
            innerSizer->Add(m_gridSizeText);
            innerSizer->AddSpacer(1);
            innerSizer->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), 0, wxEXPAND);
            
            innerSizer->Add(m_textureLockIconPanel);
            innerSizer->Add(m_textureLockText);
            innerSizer->AddSpacer(1);
            innerSizer->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), 0, wxEXPAND);
            innerSizer->AddSpacer(1);

            innerSizer->Add(m_issuesIconPanel);
            innerSizer->AddSpacer(1);
            innerSizer->Add(m_issuesText);
            innerSizer->AddSpacer(1);
            innerSizer->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), 0, wxEXPAND);
            innerSizer->AddSpacer(1);

            innerSizer->Add(m_message, 1, wxEXPAND);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(line, 0, wxEXPAND);
            outerSizer->Add(innerSizer, 1, wxEXPAND);
            outerSizer->SetItemMinSize(innerSizer, wxDefaultSize.x, 18);
            SetSizerAndFit(outerSizer);
            
            console->logNotifier.addObserver(this, &StatusBar::log);
            
            Bind(wxEVT_IDLE, &StatusBar::OnIdle, this);
        }

        void StatusBar::log(Logger::LogLevel level, const String& message) {
            m_message->SetLabel(message);
        }

        void StatusBar::OnIdle(wxIdleEvent& event) {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                
                const Grid& grid = document->grid();
                if (grid.actualSize() != m_gridSize) {
                    m_gridSizeText->SetLabel(wxString("Size ") << grid.actualSize());
                    m_gridSize = grid.actualSize();
                }
                
                if (document->textureLock() != m_textureLock) {
                    if (document->textureLock()) {
                        m_textureLockIconPanel->SetImage(m_textureLockOn);
                        m_textureLockText->SetLabel(wxString("Texture Lock On"));
                        m_textureLock = true;
                    } else {
                        m_textureLockIconPanel->SetImage(m_textureLockOff);
                        m_textureLockText->SetLabel(wxString("Texture Lock Off"));
                        m_textureLock = false;
                    }
                }
                
                const Model::IssueManager& issueManager = document->issueManager();
                if (issueManager.issueCount() != m_issueCount) {
                    m_issuesText->SetLabel(wxString("") << issueManager.issueCount() << " Issues");
                    m_issueCount = issueManager.issueCount();
                }
            }
        }
    }
}
