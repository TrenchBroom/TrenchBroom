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

#include "PathDialog.h"

#include "IO/FileManager.h"
#include "Utility/Preferences.h"
#include "View/LayoutConstants.h"

#include <wx/gbsizer.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/stdpaths.h>

namespace TrenchBroom {
    namespace View {
        BEGIN_EVENT_TABLE(PathDialog, wxDialog)
        EVT_BUTTON(wxID_OK, PathDialog::OnOkClicked)
        EVT_BUTTON(wxID_CANCEL, PathDialog::OnCancelClicked)
        END_EVENT_TABLE()

        PathDialog::PathDialog(wxWindow* parent, const String& path, const String& mapPath) :
        wxDialog(parent, wxID_ANY, wxT("Path")) {
            IO::FileManager fileManager;
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            const String appPath = wxStandardPaths::Get().GetExecutablePath().ToStdString();
            const String& quakePath = prefs.getString(Preferences::QuakePath);

            if (fileManager.isAbsolutePath(path)) {
                m_absolutePath = path;
            } else {
                if (!mapPath.empty())
                    m_absolutePath = fileManager.makeAbsolute(path, mapPath);
                if (!fileManager.exists(m_absolutePath))
                    m_absolutePath = fileManager.makeAbsolute(path, appPath);
                if (!fileManager.exists(m_absolutePath))
                    m_absolutePath = fileManager.makeAbsolute(path, quakePath);
            }

            if (fileManager.exists(m_absolutePath)) {
                if (!mapPath.empty())
                    m_mapRelativePath = fileManager.makeRelative(m_absolutePath, mapPath);
                m_appRelativePath = fileManager.makeRelative(m_absolutePath, appPath);
                m_quakeRelativePath = fileManager.makeRelative(m_absolutePath, quakePath);
            }
            
            wxStaticBox* box = new wxStaticBox(this, wxID_ANY, wxT(""));
            wxStaticText* infoText = new wxStaticText(box, wxID_ANY, wxT("Paths can be stored either as absolute paths or as relative paths. Please choose how you want to store this path."));
            
            m_absolute = new wxRadioButton(box, wxID_ANY, wxT("Absolute"));
            m_absolute->SetFont(m_absolute->GetFont().MakeBold());
            m_absolute->SetValue(true);
            wxStaticText* absolutePathText = new wxStaticText(box, wxID_ANY, m_absolutePath, wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_MIDDLE);

            m_relativeToMap = new wxRadioButton(box, wxID_ANY, wxT("Relative to map file"));
            m_relativeToMap->SetFont(m_relativeToMap->GetFont().MakeBold());
            if (m_mapRelativePath.empty())
                m_relativeToMap->Enable(false);
            wxStaticText* mapRelativePathText = new wxStaticText(box, wxID_ANY, m_mapRelativePath.empty() ? "Please save this map first" : m_mapRelativePath, wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_MIDDLE);
            
            m_relativeToApp = new wxRadioButton(box, wxID_ANY, wxT("Relative to application executable"));
            m_relativeToApp->SetFont(m_relativeToApp->GetFont().MakeBold());
            wxStaticText* appRelativePathText = new wxStaticText(box, wxID_ANY, m_appRelativePath, wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_MIDDLE);
            
            m_relativeToQuake = new wxRadioButton(box, wxID_ANY, wxT("Relative to Quake directory"));
            m_relativeToQuake->SetFont(m_relativeToQuake->GetFont().MakeBold());
            wxStaticText* quakeRelativePathText = new wxStaticText(box, wxID_ANY, m_quakeRelativePath, wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_MIDDLE);

#if defined __APPLE__
            absolutePathText->SetFont(*wxSMALL_FONT);
            mapRelativePathText->SetFont(*wxSMALL_FONT);
            appRelativePathText->SetFont(*wxSMALL_FONT);
            quakeRelativePathText->SetFont(*wxSMALL_FONT);
#endif
            
            wxGridBagSizer* innerSizer = new wxGridBagSizer();
            
            innerSizer->Add(infoText, wxGBPosition(0, 0), wxGBSpan(1, 2), wxLEFT | wxTOP | wxRIGHT, LayoutConstants::StaticBoxInnerMargin);
            innerSizer->Add(1, 2 * LayoutConstants::ControlVerticalMargin, wxGBPosition(1,0), wxGBSpan(1,2));
            
            innerSizer->Add(m_absolute, wxGBPosition(2, 0), wxGBSpan(1, 2), wxLEFT | wxRIGHT, LayoutConstants::StaticBoxInnerMargin);
            innerSizer->Add(18, 1, wxGBPosition(3, 0), wxGBSpan(1, 1), wxLEFT, LayoutConstants::StaticBoxInnerMargin);
            innerSizer->Add(absolutePathText, wxGBPosition(3, 1), wxGBSpan(1, 1), wxRIGHT, LayoutConstants::StaticBoxInnerMargin);
            innerSizer->Add(1, LayoutConstants::ControlVerticalMargin, wxGBPosition(4,0), wxGBSpan(1,2));
            
            innerSizer->Add(m_relativeToMap, wxGBPosition(5, 0), wxGBSpan(1, 2), wxLEFT | wxRIGHT, LayoutConstants::StaticBoxInnerMargin);
            innerSizer->Add(18, 1, wxGBPosition(6, 0), wxGBSpan(1, 1), wxLEFT, LayoutConstants::StaticBoxInnerMargin);
            innerSizer->Add(mapRelativePathText, wxGBPosition(6, 1), wxGBSpan(1, 1), wxRIGHT, LayoutConstants::StaticBoxInnerMargin);
            innerSizer->Add(1, LayoutConstants::ControlVerticalMargin, wxGBPosition(7,0), wxGBSpan(1,2));
            
            innerSizer->Add(m_relativeToApp, wxGBPosition(8, 0), wxGBSpan(1, 2), wxLEFT | wxRIGHT, LayoutConstants::StaticBoxInnerMargin);
            innerSizer->Add(18, 1, wxGBPosition(9, 0), wxGBSpan(1, 1), wxLEFT, LayoutConstants::StaticBoxInnerMargin);
            innerSizer->Add(appRelativePathText, wxGBPosition(9, 1), wxGBSpan(1, 1), wxRIGHT, LayoutConstants::StaticBoxInnerMargin);
            innerSizer->Add(1, LayoutConstants::ControlVerticalMargin, wxGBPosition(10,0), wxGBSpan(1,2));
            
            innerSizer->Add(m_relativeToQuake, wxGBPosition(11, 0), wxGBSpan(1, 2), wxLEFT | wxRIGHT, LayoutConstants::StaticBoxInnerMargin);
            innerSizer->Add(18, 1, wxGBPosition(12, 0), wxGBSpan(1, 1), wxLEFT, LayoutConstants::StaticBoxInnerMargin);
            innerSizer->Add(quakeRelativePathText, wxGBPosition(12, 1), wxGBSpan(1, 1), wxRIGHT | wxBOTTOM, LayoutConstants::StaticBoxInnerMargin);

            int maxWidth = 350;
            maxWidth = std::max(maxWidth, absolutePathText->GetSize().x);
            maxWidth = std::max(maxWidth, mapRelativePathText->GetSize().x);
            maxWidth = std::max(maxWidth, appRelativePathText->GetSize().x);
            maxWidth = std::max(maxWidth, quakeRelativePathText->GetSize().x);
            infoText->Wrap(maxWidth);
            
            box->SetSizerAndFit(innerSizer);

            wxSizer* buttonSizer = CreateButtonSizer(wxOK | wxCANCEL);
            SetAffirmativeId(wxOK);
            SetEscapeId(wxCANCEL);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(box, 1, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, LayoutConstants::DialogOuterMargin);
            outerSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, LayoutConstants::DialogButtonMargin);

            SetSizerAndFit(outerSizer);
            if (parent != NULL) {
                wxPoint position = parent->GetPosition();
                position.x += (parent->GetSize().x - GetSize().x) / 2;
                position.y += (parent->GetSize().y - GetSize().y) / 2;
                SetPosition(position);
            }
        }

        const String& PathDialog::path() const {
            if (m_relativeToMap->GetValue())
                return m_mapRelativePath;
            if (m_relativeToApp->GetValue())
                return m_appRelativePath;
            if (m_relativeToQuake->GetValue())
                return m_quakeRelativePath;
            return m_absolutePath;
        }

        
        void PathDialog::OnOkClicked(wxCommandEvent& event) {
            EndModal(wxID_OK);
        }
        
        void PathDialog::OnCancelClicked(wxCommandEvent& event) {
            EndModal(wxID_CANCEL);
        }
    }
}
