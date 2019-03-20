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

#include "ScaleObjectsToolPage.h"

#include "TrenchBroom.h"
#include "View/BorderLine.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/ScaleObjectsTool.h"
#include "View/SpinControl.h"
#include "View/ViewConstants.h"

#include <vecmath/vec.h>

#include <wx/button.h>
#include <wx/choice.h>
#include <wx/simplebook.h>
#include <wx/sizer.h>
#include <QLabel>
#include <wx/combobox.h>
#include <wx/simplebook.h>

namespace TrenchBroom {
    namespace View {
        ScaleObjectsToolPage::ScaleObjectsToolPage(QWidget* parent, MapDocumentWPtr document) :
        QWidget(parent),
        m_document(document) {
            createGui();
        }

        void ScaleObjectsToolPage::activate() {
            const auto document = lock(m_document);
            const auto suggestedSize = document->hasSelectedNodes() ? document->selectionBounds().size() : vm::vec3::zero;

            m_sizeTextBox->SetValue(StringUtils::toString(suggestedSize));
            m_factorsTextBox->SetValue("1.0 1.0 1.0");
        }

        void ScaleObjectsToolPage::createGui() {
            MapDocumentSPtr document = lock(m_document);

            QLabel* text = new QLabel(this, wxID_ANY, "Scale objects");

            m_book = new wxSimplebook(this);
            m_sizeTextBox = new wxTextCtrl(m_book, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
            m_factorsTextBox = new wxTextCtrl(m_book, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
            m_book->AddPage(m_sizeTextBox, "");
            m_book->AddPage(m_factorsTextBox, "");

            m_sizeTextBox->Bind(wxEVT_TEXT_ENTER, &ScaleObjectsToolPage::OnApply, this);
            m_factorsTextBox->Bind(wxEVT_TEXT_ENTER, &ScaleObjectsToolPage::OnApply, this);

            const QString choices[] = { "to size", "by factors" };
            m_scaleFactorsOrSize = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 2, choices);
            m_scaleFactorsOrSize->Bind(wxEVT_CHOICE, [&](wxCommandEvent& event){
                    const auto selection = m_scaleFactorsOrSize->GetSelection();
                    if (selection != wxNOT_FOUND) {
                        m_book->SetSelection(static_cast<size_t>(selection));
                    }
                });
            m_scaleFactorsOrSize->SetSelection(0);

            m_button = new wxButton(this, wxID_ANY, "Apply", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);

            m_button->Bind(wxEVT_UPDATE_UI, &ScaleObjectsToolPage::OnUpdateButton, this);
            m_button->Bind(wxEVT_BUTTON, &ScaleObjectsToolPage::OnApply, this);

            wxBoxSizer* sizer = new QHBoxLayout();
            sizer->addWidget(text, 0, wxALIGN_CENTER_VERTICAL);
            sizer->addSpacing(LayoutConstants::NarrowHMargin);
            sizer->addWidget(m_scaleFactorsOrSize, 0, wxALIGN_CENTER_VERTICAL);
            sizer->addSpacing(LayoutConstants::NarrowHMargin);
            sizer->addWidget(m_book, 0, wxALIGN_CENTER_VERTICAL);
            sizer->addSpacing(LayoutConstants::NarrowHMargin);
            sizer->addWidget(m_button, 0, wxALIGN_CENTER_VERTICAL);

            SetSizer(sizer);
        }

        bool ScaleObjectsToolPage::canScale() const {
            return lock(m_document)->hasSelectedNodes();
        }

        void ScaleObjectsToolPage::OnUpdateButton(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            event.Enable(canScale());
        }

        vm::vec3 ScaleObjectsToolPage::getScaleFactors() const {
            switch (m_scaleFactorsOrSize->GetSelection()) {
                case 0: {
                    auto document = lock(m_document);
                    const auto desiredSize = vm::vec3::parse(m_sizeTextBox->GetValue().ToStdString());

                    return desiredSize / document->selectionBounds().size();
                }
                default:
                    return vm::vec3::parse(m_factorsTextBox->GetValue().ToStdString());
            }
        }

        void ScaleObjectsToolPage::OnApply(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (!canScale()) {
                return;
            }

            auto document = lock(m_document);
            const auto box = document->selectionBounds();
            const auto scaleFactors = getScaleFactors();

            document->scaleObjects(box.center(), scaleFactors);
        }
    }
}
