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

#include "UVEditor.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "View/MapDocument.h"
#include "View/UVView.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QAbstractButton>
#include <QSpinBox>

namespace TrenchBroom {
    namespace View {
        UVEditor::UVEditor(QWidget* parent, MapDocumentWPtr document, GLContextManager& contextManager) :
        QWidget(parent),
        m_document(document),
        m_uvView(nullptr),
        m_xSubDivisionEditor(nullptr),
        m_ySubDivisionEditor(nullptr) {
            createGui(contextManager);
            bindObservers();
        }

        UVEditor::~UVEditor() {
            unbindObservers();
        }

        bool UVEditor::cancelMouseDrag() {
            return m_uvView->cancelDrag();
        }

        void UVEditor::OnResetTexture() {
            Model::ChangeBrushFaceAttributesRequest request;
            request.resetAll();

            MapDocumentSPtr document = lock(m_document);
            document->setFaceAttributes(request);
        }

        void UVEditor::OnFlipTextureH() {
            Model::ChangeBrushFaceAttributesRequest request;
            request.mulXScale(-1.0f);

            MapDocumentSPtr document = lock(m_document);
            document->setFaceAttributes(request);
        }

        void UVEditor::OnFlipTextureV() {
            Model::ChangeBrushFaceAttributesRequest request;
            request.mulYScale(-1.0f);

            MapDocumentSPtr document = lock(m_document);
            document->setFaceAttributes(request);
        }

        void UVEditor::OnRotateTextureCCW() {
            Model::ChangeBrushFaceAttributesRequest request;
            request.addRotation(90.0f);

            MapDocumentSPtr document = lock(m_document);
            document->setFaceAttributes(request);
        }

        void UVEditor::OnRotateTextureCW() {
            Model::ChangeBrushFaceAttributesRequest request;
            request.addRotation(-90.0f);

            MapDocumentSPtr document = lock(m_document);
            document->setFaceAttributes(request);
        }

        void UVEditor::updateButtons() {
            MapDocumentSPtr document = lock(m_document);
            const bool enabled = !document->allSelectedBrushFaces().empty();

            m_resetTextureButton->setEnabled(enabled);
            m_flipTextureHButton->setEnabled(enabled);
            m_flipTextureVButton->setEnabled(enabled);
            m_rotateTextureCCWButton->setEnabled(enabled);
            m_rotateTextureCWButton->setEnabled(enabled);
        }

        void UVEditor::OnSubDivisionChanged() {
            const int x = m_xSubDivisionEditor->value();
            const int y = m_ySubDivisionEditor->value();
            m_uvView->setSubDivisions(vm::vec2i(x, y));
        }

        void UVEditor::createGui(GLContextManager& contextManager) {
            m_uvView = new UVView(m_document, contextManager);
            m_windowContainer = m_uvView->widgetContainer();

            m_resetTextureButton = createBitmapButton("ResetTexture.png", tr("Reset texture alignment"), this);
            m_flipTextureHButton = createBitmapButton("FlipTextureH.png", tr("Flip texture X axis"), this);
            m_flipTextureVButton = createBitmapButton("FlipTextureV.png", tr("Flip texture Y axis"), this);
            m_rotateTextureCCWButton = createBitmapButton("RotateTextureCCW.png",
                                                          tr("Rotate texture 90° counter-clockwise"), this);
            m_rotateTextureCWButton = createBitmapButton("RotateTextureCW.png", tr("Rotate texture 90° clockwise"),
                                                         this);

            connect(m_resetTextureButton, &QAbstractButton::clicked, this, &UVEditor::OnResetTexture);

            connect(m_flipTextureHButton, &QAbstractButton::clicked, this, &UVEditor::OnFlipTextureH);
            connect(m_flipTextureVButton, &QAbstractButton::clicked, this, &UVEditor::OnFlipTextureV);
            connect(m_rotateTextureCCWButton, &QAbstractButton::clicked, this, &UVEditor::OnRotateTextureCCW);
            connect(m_rotateTextureCWButton, &QAbstractButton::clicked, this, &UVEditor::OnRotateTextureCW);

            QLabel* gridLabel = new QLabel("Grid ");
            // FIXME:
//            gridLabel->SetFont(gridLabel->GetFont().Bold());
            m_xSubDivisionEditor = new QSpinBox(); //(this, wxID_ANY, "1", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER | wxALIGN_RIGHT);
            m_xSubDivisionEditor->setRange(1, 16);
            m_xSubDivisionEditor->setValue(1);

            m_ySubDivisionEditor = new QSpinBox(); //(this, wxID_ANY, "1", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER | wxALIGN_RIGHT);
            m_ySubDivisionEditor->setRange(1, 16);
            m_ySubDivisionEditor->setValue(1);

            connect(m_xSubDivisionEditor, QOverload<int>::of(&QSpinBox::valueChanged), this, &UVEditor::OnSubDivisionChanged);
            connect(m_ySubDivisionEditor, QOverload<int>::of(&QSpinBox::valueChanged), this, &UVEditor::OnSubDivisionChanged);

            auto* bottomSizer = new QHBoxLayout();
            bottomSizer->addWidget(m_resetTextureButton,                   0, Qt::AlignVCenter);// | wxRIGHT, LayoutConstants::NarrowHMargin);
            bottomSizer->addWidget(m_flipTextureHButton,                   0, Qt::AlignVCenter);// | wxRIGHT, LayoutConstants::NarrowHMargin);
            bottomSizer->addWidget(m_flipTextureVButton,                   0, Qt::AlignVCenter);// | wxRIGHT, LayoutConstants::NarrowHMargin);
            bottomSizer->addWidget(m_rotateTextureCCWButton,               0, Qt::AlignVCenter);// | wxRIGHT, LayoutConstants::NarrowHMargin);
            bottomSizer->addWidget(m_rotateTextureCWButton,                0, Qt::AlignVCenter);// | wxRIGHT, LayoutConstants::NarrowHMargin);
            bottomSizer->addStretch(1);
            bottomSizer->addWidget(gridLabel,                              0, Qt::AlignVCenter);
            bottomSizer->addWidget(new QLabel("X:"), 0, Qt::AlignVCenter);// | wxRIGHT, LayoutConstants::NarrowHMargin);
            bottomSizer->addWidget(m_xSubDivisionEditor,                   0, Qt::AlignVCenter);// | wxRIGHT, LayoutConstants::MediumHMargin);
            bottomSizer->addWidget(new QLabel("Y:"), 0, Qt::AlignVCenter);// | wxRIGHT, LayoutConstants::NarrowHMargin);
            bottomSizer->addWidget(m_ySubDivisionEditor,                   0, Qt::AlignVCenter);
//            bottomSizer->SetItemMinSize(m_xSubDivisionEditor, 50, m_xSubDivisionEditor->GetSize().y);
//            bottomSizer->SetItemMinSize(m_ySubDivisionEditor, 50, m_ySubDivisionEditor->GetSize().y);

            auto* outerSizer = new QVBoxLayout();
            outerSizer->setContentsMargins(0, 0, 0, 0);
            outerSizer->addWidget(m_windowContainer, 1); //, wxEXPAND);
//            outerSizer->addSpacing(LayoutConstants::NarrowVMargin);
            outerSizer->addLayout(bottomSizer); //, wxLEFT | wxRIGHT | wxEXPAND, LayoutConstants::MediumHMargin);
//            outerSizer->addSpacing(LayoutConstants::NarrowVMargin);

            setLayout(outerSizer);

            updateButtons();
        }

        void UVEditor::selectionDidChange(const Selection& selection) {
            updateButtons();
        }

        void UVEditor::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->selectionDidChangeNotifier.addObserver(this, &UVEditor::selectionDidChange);
        }

        void UVEditor::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->selectionDidChangeNotifier.removeObserver(this, &UVEditor::selectionDidChange);
            }
        }
    }
}
