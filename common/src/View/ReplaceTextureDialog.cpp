/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "ReplaceTextureDialog.h"

#include "Assets/Texture.h"
#include "Model/BrushFace.h"
#include "Model/CollectMatchingBrushFacesVisitor.h"
#include "Model/World.h"
#include "View/BorderLine.h"
#include "View/MapDocument.h"
#include "View/TextureBrowser.h"
#include "View/TitledPanel.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <wx/button.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        ReplaceTextureDialog::ReplaceTextureDialog(wxWindow* parent, MapDocumentWPtr document, GLContextManager& contextManager) :
        wxDialog(parent, wxID_ANY, "Replace Texture", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxSTAY_ON_TOP),
        m_document(document),
        m_subjectBrowser(NULL),
        m_replacementBrowser(NULL) {
            createGui(contextManager);
        }
        
        void ReplaceTextureDialog::OnReplace(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const Assets::Texture* subject = m_subjectBrowser->selectedTexture();
            ensure(subject != NULL, "subject is null");

            Assets::Texture* replacement = m_replacementBrowser->selectedTexture();
            ensure(replacement != NULL, "replacement is null");
            
            MapDocumentSPtr document = lock(m_document);
            const Model::BrushFaceList faces = getApplicableFaces();
            
            if (faces.empty()) {
                wxMessageBox("None of the selected faces has the selected texture", "Replace Failed", wxOK | wxCENTRE, this);
                return;
            }
            
            Transaction transaction(document, "Replace Textures");
            document->select(faces);
            document->setTexture(replacement);
            
            StringStream msg;
            msg << "Replaced texture '" << subject->name() << "' with '" << replacement->name() << "' on " << faces.size() << " faces.";
            wxMessageBox(msg.str(), "Replace succeeded", wxOK | wxCENTRE, this);
        }
        
        Model::BrushFaceList ReplaceTextureDialog::getApplicableFaces() const {
            MapDocumentSPtr document = lock(m_document);
            Model::BrushFaceList faces = document->allSelectedBrushFaces();
            if (faces.empty()) {
                Model::CollectBrushFacesVisitor collect;
                document->world()->acceptAndRecurse(collect);
                faces = collect.faces();
            }
            
            const Assets::Texture* subject = m_subjectBrowser->selectedTexture();
            ensure(subject != NULL, "subject is null");
            
            Model::BrushFaceList result;
            
            Model::BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                if (face->texture() == subject)
                    result.push_back(face);
            }
            return result;
        }
        
        void ReplaceTextureDialog::OnUpdateReplaceButton(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            const Assets::Texture* subject = m_subjectBrowser->selectedTexture();
            const Assets::Texture* replacement = m_replacementBrowser->selectedTexture();
            event.Enable(subject != NULL && replacement != NULL);
        }

        void ReplaceTextureDialog::createGui(GLContextManager& contextManager) {
            setWindowIcon(this);

            TitledPanel* subjectPanel = new TitledPanel(this, "Find");
            m_subjectBrowser = new TextureBrowser(subjectPanel->getPanel(), m_document, contextManager);
            m_subjectBrowser->setHideUnused(true);
            
            wxSizer* subjectPanelSizer = new wxBoxSizer(wxVERTICAL);
            subjectPanelSizer->Add(m_subjectBrowser, 1, wxEXPAND);
            subjectPanel->getPanel()->SetSizer(subjectPanelSizer);
            
            TitledPanel* replacementPanel = new TitledPanel(this, "Replace with");
            m_replacementBrowser = new TextureBrowser(replacementPanel->getPanel(), m_document, contextManager);
            
            wxSizer* replacementPanelSizer = new wxBoxSizer(wxVERTICAL);
            replacementPanelSizer->Add(m_replacementBrowser, 1, wxEXPAND);
            replacementPanel->getPanel()->SetSizer(replacementPanelSizer);
            
            wxSizer* upperSizer = new wxBoxSizer(wxHORIZONTAL);
            upperSizer->Add(subjectPanel, 1, wxEXPAND);
            upperSizer->Add(new BorderLine(this, BorderLine::Direction_Vertical), 0, wxEXPAND);
            upperSizer->Add(replacementPanel, 1, wxEXPAND);

            wxButton* replaceButton = new wxButton(this, wxID_OK, "Replace");
            replaceButton->SetToolTip("Perform replacement on all selected faces");
            wxButton* closeButton = new wxButton(this, wxID_CANCEL, "Close");
            closeButton->SetToolTip("Close this window");
            
            replaceButton->Bind(wxEVT_BUTTON, &ReplaceTextureDialog::OnReplace, this);
            replaceButton->Bind(wxEVT_UPDATE_UI, &ReplaceTextureDialog::OnUpdateReplaceButton, this);
            
            wxStdDialogButtonSizer* buttonSizer = new wxStdDialogButtonSizer();
            buttonSizer->SetAffirmativeButton(replaceButton);
            buttonSizer->SetCancelButton(closeButton);
            buttonSizer->Realize();
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(upperSizer, 1, wxEXPAND);
            outerSizer->Add(wrapDialogButtonSizer(buttonSizer, this), 0, wxEXPAND);
            SetSizer(outerSizer);
            
            SetMinSize(wxSize(650, 450));
            SetSize(wxSize(650, 450));
        }
    }
}
