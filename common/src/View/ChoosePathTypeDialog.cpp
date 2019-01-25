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

#include "ChoosePathTypeDialog.h"

#include "IO/SystemPaths.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <wx/gbsizer.h>
#include <wx/panel.h>
#include <wx/radiobut.h>
#include <QLabel>

namespace TrenchBroom {
    namespace View {
        wxIMPLEMENT_DYNAMIC_CLASS(ChoosePathTypeDialog, wxDialog)
        
        ChoosePathTypeDialog::ChoosePathTypeDialog() :
        wxDialog(nullptr, wxID_ANY, "Path Type"),
        m_absPath(""),
        m_docRelativePath(""),
        m_gameRelativePath(""),
        m_appRelativePath("") {
            Create();
        }

        
        ChoosePathTypeDialog::ChoosePathTypeDialog(QWidget* parent, const IO::Path& absPath, const IO::Path& docPath, const IO::Path& gamePath) :
        wxDialog(parent, wxID_ANY, "Path Type"),
        m_absPath(absPath),
        m_docRelativePath(makeRelativePath(absPath, docPath.deleteLastComponent())),
        m_gameRelativePath(makeRelativePath(absPath, gamePath)),
        m_appRelativePath(makeRelativePath(absPath, IO::SystemPaths::appDirectory())) {
            Create();
        }
        
        bool ChoosePathTypeDialog::Create() {
            setWindowIcon(this);

            QWidget* panel = new QWidget(this);
            
            QLabel* infoText = new QLabel(panel, wxID_ANY, "Paths can be stored either as absolute paths or as relative paths. Please choose how you want to store this path.");
            infoText->Wrap(370);
            
            m_absRadio = new wxRadioButton(panel, wxID_ANY, "Absolute");
            m_absRadio->SetFont(m_absRadio->GetFont().MakeBold());
            m_absRadio->SetValue(true);
            QLabel* absolutePathText = new QLabel(panel, wxID_ANY, m_absPath.asString(), wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_MIDDLE);
            
            m_docRelativeRadio = new wxRadioButton(panel, wxID_ANY, "Relative to map file");
            m_docRelativeRadio->SetFont(m_docRelativeRadio->GetFont().MakeBold());
            if (m_docRelativePath.isEmpty())
                m_docRelativeRadio->Enable(false);
            QLabel* mapRelativePathText = new QLabel(panel, wxID_ANY, m_docRelativePath.isEmpty() ? "Could not build a path." : m_docRelativePath.asString(), wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_MIDDLE);
            
            m_appRelativeRadio = new wxRadioButton(panel, wxID_ANY, "Relative to application executable");
            m_appRelativeRadio->SetFont(m_appRelativeRadio->GetFont().MakeBold());
            if (m_appRelativePath.isEmpty())
                m_appRelativeRadio->Enable(false);
            QLabel* appRelativePathText = new QLabel(panel, wxID_ANY, m_appRelativePath.isEmpty() ? "Could not build a path." : m_appRelativePath.asString(), wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_MIDDLE);
            
            m_gameRelativeRadio = new wxRadioButton(panel, wxID_ANY, "Relative to game directory");
            if (m_gameRelativePath.isEmpty())
                m_gameRelativeRadio->Enable(false);
            m_gameRelativeRadio->SetFont(m_gameRelativeRadio->GetFont().MakeBold());
            QLabel* gameRelativePathText = new QLabel(panel, wxID_ANY, m_gameRelativePath.isEmpty() ? "Could not build a path." : m_gameRelativePath.asString(), wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_MIDDLE);
            
#if defined __APPLE__
            absolutePathText->SetFont(*wxSMALL_FONT);
            mapRelativePathText->SetFont(*wxSMALL_FONT);
            appRelativePathText->SetFont(*wxSMALL_FONT);
            gameRelativePathText->SetFont(*wxSMALL_FONT);
#endif
            
            wxGridBagSizer* innerSizer = new wxGridBagSizer();
            
            innerSizer->Add(infoText, wxGBPosition(0, 0), wxGBSpan(1, 2));
            innerSizer->Add(1, 2 * LayoutConstants::WideVMargin, wxGBPosition(1,0), wxGBSpan(1,2));
            
            innerSizer->Add(m_absRadio, wxGBPosition(2, 0), wxGBSpan(1, 2));
            innerSizer->Add(18, 1, wxGBPosition(3, 0), wxGBSpan(1, 1));
            innerSizer->Add(absolutePathText, wxGBPosition(3, 1), wxGBSpan(1, 1));
            innerSizer->Add(1, LayoutConstants::WideVMargin, wxGBPosition(4,0), wxGBSpan(1,2));
            
            innerSizer->Add(m_docRelativeRadio, wxGBPosition(5, 0), wxGBSpan(1, 2));
            innerSizer->Add(18, 1, wxGBPosition(6, 0), wxGBSpan(1, 1));
            innerSizer->Add(mapRelativePathText, wxGBPosition(6, 1), wxGBSpan(1, 1));
            innerSizer->Add(1, LayoutConstants::WideVMargin, wxGBPosition(7,0), wxGBSpan(1,2));
            
            innerSizer->Add(m_appRelativeRadio, wxGBPosition(8, 0), wxGBSpan(1, 2));
            innerSizer->Add(18, 1, wxGBPosition(9, 0), wxGBSpan(1, 1));
            innerSizer->Add(appRelativePathText, wxGBPosition(9, 1), wxGBSpan(1, 1));
            innerSizer->Add(1, LayoutConstants::WideVMargin, wxGBPosition(10,0), wxGBSpan(1,2));
            
            innerSizer->Add(m_gameRelativeRadio, wxGBPosition(11, 0), wxGBSpan(1, 2));
            innerSizer->Add(18, 1, wxGBPosition(12, 0), wxGBSpan(1, 1));
            innerSizer->Add(gameRelativePathText, wxGBPosition(12, 1), wxGBSpan(1, 1));
            
            auto* panelSizer = new QVBoxLayout();
            panelSizer->Add(innerSizer, 1, wxEXPAND | wxALL, LayoutConstants::DialogOuterMargin);
            panel->SetSizerAndFit(panelSizer);
            
            wxSizer* buttonSizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL);
            auto* outerSizer = new QVBoxLayout();
            outerSizer->Add(panel, 1, wxEXPAND);
            outerSizer->Add(wrapDialogButtonSizer(buttonSizer, this), 0, wxEXPAND);
            
            SetSizerAndFit(outerSizer);
            CentreOnParent();
            
            Bind(wxEVT_CLOSE_WINDOW, &ChoosePathTypeDialog::OnClose, this);
            
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

        void ChoosePathTypeDialog::OnClose(wxCloseEvent& event) {
            if (GetParent() != nullptr)
                GetParent()->Raise();
            event.Skip();
        }

        IO::Path ChoosePathTypeDialog::makeRelativePath(const IO::Path& absPath, const IO::Path& newRootPath) {
            if (!newRootPath.canMakeRelative(absPath))
                return IO::Path("");
            return newRootPath.makeRelative(absPath);
        }
    }
}
