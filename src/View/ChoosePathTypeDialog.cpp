/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "ChoosePathTypeDialog.h"

#include "IO/SystemPaths.h"
#include "View/ViewConstants.h"

#include <wx/gbsizer.h>
#include <wx/radiobut.h>
#include <wx/statbox.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        IMPLEMENT_DYNAMIC_CLASS(ChoosePathTypeDialog, wxDialog)
        
        ChoosePathTypeDialog::ChoosePathTypeDialog() :
        wxDialog(NULL, wxID_ANY, _("Path Type")),
        m_absPath(""),
        m_docRelativePath(""),
        m_gameRelativePath(""),
        m_appRelativePath("") {
            Create();
        }

        
        ChoosePathTypeDialog::ChoosePathTypeDialog(wxWindow* parent, const IO::Path& absPath, const IO::Path& docPath, const IO::Path& gamePath) :
        wxDialog(parent, wxID_ANY, _("Path Type")),
        m_absPath(absPath),
        m_docRelativePath(makeRelativePath(absPath, docPath.deleteLastComponent())),
        m_gameRelativePath(makeRelativePath(absPath, gamePath)),
        m_appRelativePath(makeRelativePath(absPath, IO::SystemPaths::appDirectory())) {
            Create();
        }
        
        bool ChoosePathTypeDialog::Create() {
            wxStaticBox* box = new wxStaticBox(this, wxID_ANY, _(""));
            wxStaticText* infoText = new wxStaticText(box, wxID_ANY, _("Paths can be stored either as absolute paths or as relative paths. Please choose how you want to store this path."));
            
            m_absRadio = new wxRadioButton(box, wxID_ANY, _("Absolute"));
            m_absRadio->SetFont(m_absRadio->GetFont().MakeBold());
            m_absRadio->SetValue(true);
            wxStaticText* absolutePathText = new wxStaticText(box, wxID_ANY, m_absPath.asString(), wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_MIDDLE);
            
            m_docRelativeRadio = new wxRadioButton(box, wxID_ANY, _("Relative to map file"));
            m_docRelativeRadio->SetFont(m_docRelativeRadio->GetFont().MakeBold());
            if (m_docRelativePath.isEmpty())
                m_docRelativeRadio->Enable(false);
            wxStaticText* mapRelativePathText = new wxStaticText(box, wxID_ANY, m_docRelativePath.isEmpty() ? _("Please save this map first.") : m_docRelativePath.asString(), wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_MIDDLE);
            
            m_appRelativeRadio = new wxRadioButton(box, wxID_ANY, _("Relative to application executable"));
            m_appRelativeRadio->SetFont(m_appRelativeRadio->GetFont().MakeBold());
            wxStaticText* appRelativePathText = new wxStaticText(box, wxID_ANY, m_appRelativePath.asString(), wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_MIDDLE);
            
            m_gameRelativeRadio = new wxRadioButton(box, wxID_ANY, _("Relative to game directory"));
            if (m_gameRelativePath.isEmpty())
                m_gameRelativeRadio->Enable(false);
            m_gameRelativeRadio->SetFont(m_gameRelativeRadio->GetFont().MakeBold());
            wxStaticText* gameRelativePathText = new wxStaticText(box, wxID_ANY, m_gameRelativePath.isEmpty() ? _("Please set the game path first.") : m_gameRelativePath.asString(), wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_MIDDLE);
            
#if defined __APPLE__
            absolutePathText->SetFont(*wxSMALL_FONT);
            mapRelativePathText->SetFont(*wxSMALL_FONT);
            appRelativePathText->SetFont(*wxSMALL_FONT);
            gameRelativePathText->SetFont(*wxSMALL_FONT);
#endif
            
            wxGridBagSizer* innerSizer = new wxGridBagSizer();
            
            innerSizer->Add(0, LayoutConstants::StaticBoxTopMargin, wxGBPosition(0, 0), wxGBSpan(1, 2));
            innerSizer->Add(infoText, wxGBPosition(1, 0), wxGBSpan(1, 2), wxLEFT | wxRIGHT, LayoutConstants::StaticBoxSideMargin);
            innerSizer->Add(1, 2 * LayoutConstants::ControlVerticalMargin, wxGBPosition(2,0), wxGBSpan(1,2));
            
            innerSizer->Add(m_absRadio, wxGBPosition(3, 0), wxGBSpan(1, 2), wxLEFT | wxRIGHT, LayoutConstants::StaticBoxSideMargin);
            innerSizer->Add(18, 1, wxGBPosition(4, 0), wxGBSpan(1, 1), wxLEFT, LayoutConstants::StaticBoxSideMargin);
            innerSizer->Add(absolutePathText, wxGBPosition(4, 1), wxGBSpan(1, 1), wxRIGHT, LayoutConstants::StaticBoxSideMargin);
            innerSizer->Add(1, LayoutConstants::ControlVerticalMargin, wxGBPosition(5,0), wxGBSpan(1,2));
            
            innerSizer->Add(m_docRelativeRadio, wxGBPosition(6, 0), wxGBSpan(1, 2), wxLEFT | wxRIGHT, LayoutConstants::StaticBoxSideMargin);
            innerSizer->Add(18, 1, wxGBPosition(7, 0), wxGBSpan(1, 1), wxLEFT, LayoutConstants::StaticBoxSideMargin);
            innerSizer->Add(mapRelativePathText, wxGBPosition(7, 1), wxGBSpan(1, 1), wxRIGHT, LayoutConstants::StaticBoxSideMargin);
            innerSizer->Add(1, LayoutConstants::ControlVerticalMargin, wxGBPosition(8,0), wxGBSpan(1,2));
            
            innerSizer->Add(m_appRelativeRadio, wxGBPosition(9, 0), wxGBSpan(1, 2), wxLEFT | wxRIGHT, LayoutConstants::StaticBoxSideMargin);
            innerSizer->Add(18, 1, wxGBPosition(10, 0), wxGBSpan(1, 1), wxLEFT, LayoutConstants::StaticBoxSideMargin);
            innerSizer->Add(appRelativePathText, wxGBPosition(10, 1), wxGBSpan(1, 1), wxRIGHT, LayoutConstants::StaticBoxSideMargin);
            innerSizer->Add(1, LayoutConstants::ControlVerticalMargin, wxGBPosition(11,0), wxGBSpan(1,2));
            
            innerSizer->Add(m_gameRelativeRadio, wxGBPosition(12, 0), wxGBSpan(1, 2), wxLEFT | wxRIGHT, LayoutConstants::StaticBoxSideMargin);
            innerSizer->Add(18, 1, wxGBPosition(13, 0), wxGBSpan(1, 1), wxLEFT, LayoutConstants::StaticBoxSideMargin);
            innerSizer->Add(gameRelativePathText, wxGBPosition(13, 1), wxGBSpan(1, 1), wxRIGHT, LayoutConstants::StaticBoxSideMargin);
            innerSizer->Add(0, LayoutConstants::StaticBoxBottomMargin, wxGBPosition(14, 0), wxGBSpan(1, 2));
            
            int maxWidth = 350;
            maxWidth = std::max(maxWidth, absolutePathText->GetSize().x);
            maxWidth = std::max(maxWidth, mapRelativePathText->GetSize().x);
            maxWidth = std::max(maxWidth, appRelativePathText->GetSize().x);
            maxWidth = std::max(maxWidth, gameRelativePathText->GetSize().x);
            infoText->Wrap(maxWidth);
            
            box->SetSizerAndFit(innerSizer);
            
            wxSizer* buttonSizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL);
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(box, 1, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, LayoutConstants::DialogOuterMargin);
            outerSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, LayoutConstants::DialogButtonMargin);
            
            SetSizerAndFit(outerSizer);
            CentreOnParent();
            return true;
       }

        const IO::Path& ChoosePathTypeDialog::path() const {
            if (m_docRelativeRadio->GetValue())
                return m_docRelativePath;
            if (m_appRelativeRadio->GetValue())
                return m_appRelativePath;
            if (m_gameRelativeRadio->GetValue())
                return m_gameRelativePath;
            return m_absPath;
        }

        IO::Path ChoosePathTypeDialog::makeRelativePath(const IO::Path& absPath, const IO::Path& newRootPath) {
            if (newRootPath.isEmpty())
                return IO::Path("");
            return newRootPath.makeRelative(absPath);
        }
    }
}
