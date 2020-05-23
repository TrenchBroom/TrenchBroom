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

#include "Model/BrushFaceHandle.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "View/MapDocument.h"
#include "View/UVView.h"
#include "View/ViewConstants.h"
#include "View/QtUtils.h"

#include <kdl/memory_utils.h>

#include <QtGlobal>
#include <QLabel>
#include <QHBoxLayout>
#include <QAbstractButton>
#include <QSpinBox>

namespace TrenchBroom {
    namespace View {
        UVEditor::UVEditor(std::weak_ptr<MapDocument> document, GLContextManager& contextManager, QWidget* parent) :
        QWidget(parent),
        m_document(std::move(document)),
        m_uvView(nullptr),
        m_xSubDivisionEditor(nullptr),
        m_ySubDivisionEditor(nullptr),
        m_resetTextureButton(nullptr),
        m_flipTextureHButton(nullptr),
        m_flipTextureVButton(nullptr),
        m_rotateTextureCCWButton(nullptr),
        m_rotateTextureCWButton(nullptr) {
            createGui(contextManager);
            bindObservers();
        }

        UVEditor::~UVEditor() {
            unbindObservers();
        }

        bool UVEditor::cancelMouseDrag() {
            return m_uvView->cancelDrag();
        }

        void UVEditor::updateButtons() {
            auto document = kdl::mem_lock(m_document);
            const bool enabled = !document->allSelectedBrushFaces().empty();

            m_resetTextureButton->setEnabled(enabled);
            m_flipTextureHButton->setEnabled(enabled);
            m_flipTextureVButton->setEnabled(enabled);
            m_rotateTextureCCWButton->setEnabled(enabled);
            m_rotateTextureCWButton->setEnabled(enabled);
        }

        void UVEditor::createGui(GLContextManager& contextManager) {
            m_uvView = new UVView(m_document, contextManager);

            m_resetTextureButton = createBitmapButton("ResetTexture.svg", tr("Reset texture alignment"), this);
            m_flipTextureHButton = createBitmapButton("FlipTextureH.svg", tr("Flip texture X axis"), this);
            m_flipTextureVButton = createBitmapButton("FlipTextureV.svg", tr("Flip texture Y axis"), this);
            m_rotateTextureCCWButton = createBitmapButton("RotateTextureCCW.svg",
                                                          tr("Rotate texture 90° counter-clockwise"), this);
            m_rotateTextureCWButton = createBitmapButton("RotateTextureCW.svg", tr("Rotate texture 90° clockwise"),
                                                         this);

            connect(m_resetTextureButton, &QAbstractButton::clicked, this, &UVEditor::resetTextureClicked);

            connect(m_flipTextureHButton, &QAbstractButton::clicked, this, &UVEditor::flipTextureHClicked);
            connect(m_flipTextureVButton, &QAbstractButton::clicked, this, &UVEditor::flipTextureVClicked);
            connect(m_rotateTextureCCWButton, &QAbstractButton::clicked, this, &UVEditor::rotateTextureCCWClicked);
            connect(m_rotateTextureCWButton, &QAbstractButton::clicked, this, &UVEditor::rotateTextureCWClicked);

            auto* gridLabel = new QLabel("Grid ");
            makeEmphasized(gridLabel);
            m_xSubDivisionEditor = new QSpinBox(); //(this, wxID_ANY, "1", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER | wxALIGN_RIGHT);
            m_xSubDivisionEditor->setRange(1, 16);
            m_xSubDivisionEditor->setValue(1);

            m_ySubDivisionEditor = new QSpinBox(); //(this, wxID_ANY, "1", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER | wxALIGN_RIGHT);
            m_ySubDivisionEditor->setRange(1, 16);
            m_ySubDivisionEditor->setValue(1);

            connect(m_xSubDivisionEditor, QOverload<int>::of(&QSpinBox::valueChanged), this,
                &UVEditor::subDivisionChanged);
            connect(m_ySubDivisionEditor, QOverload<int>::of(&QSpinBox::valueChanged), this,
                &UVEditor::subDivisionChanged);

            auto* bottomLayout = new QHBoxLayout();
            bottomLayout->setContentsMargins(LayoutConstants::NarrowHMargin, 0, LayoutConstants::NarrowHMargin, 0);
            bottomLayout->setSpacing(LayoutConstants::NarrowHMargin);
            bottomLayout->addWidget(m_resetTextureButton);
            bottomLayout->addWidget(m_flipTextureHButton);
            bottomLayout->addWidget(m_flipTextureVButton);
            bottomLayout->addWidget(m_rotateTextureCCWButton);
            bottomLayout->addWidget(m_rotateTextureCWButton);
            bottomLayout->addStretch();
            bottomLayout->addWidget(gridLabel);
            bottomLayout->addWidget(new QLabel("X:"));
            bottomLayout->addWidget(m_xSubDivisionEditor);
            bottomLayout->addSpacing(LayoutConstants::MediumHMargin - LayoutConstants::NarrowHMargin);
            bottomLayout->addWidget(new QLabel("Y:"));
            bottomLayout->addWidget(m_ySubDivisionEditor);

            auto* outerLayout = new QVBoxLayout();
            outerLayout->setContentsMargins(0, 0, 0, 0);
            outerLayout->setSpacing(LayoutConstants::NarrowVMargin);
            outerLayout->addWidget(m_uvView, 1);
            outerLayout->addLayout(bottomLayout);
            setLayout(outerLayout);

            updateButtons();
        }

        void UVEditor::selectionDidChange(const Selection&) {
            updateButtons();
        }

        void UVEditor::bindObservers() {
            auto document = kdl::mem_lock(m_document);
            document->selectionDidChangeNotifier.addObserver(this, &UVEditor::selectionDidChange);
        }

        void UVEditor::unbindObservers() {
            if (!kdl::mem_expired(m_document)) {
                auto document = kdl::mem_lock(m_document);
                document->selectionDidChangeNotifier.removeObserver(this, &UVEditor::selectionDidChange);
            }
        }


        void UVEditor::resetTextureClicked() {
            Model::ChangeBrushFaceAttributesRequest request;
            request.resetAll();

            auto document = kdl::mem_lock(m_document);
            document->setFaceAttributes(request);
        }

        void UVEditor::flipTextureHClicked() {
            Model::ChangeBrushFaceAttributesRequest request;
            request.mulXScale(-1.0f);

            auto document = kdl::mem_lock(m_document);
            document->setFaceAttributes(request);
        }

        void UVEditor::flipTextureVClicked() {
            Model::ChangeBrushFaceAttributesRequest request;
            request.mulYScale(-1.0f);

            auto document = kdl::mem_lock(m_document);
            document->setFaceAttributes(request);
        }

        void UVEditor::rotateTextureCCWClicked() {
            Model::ChangeBrushFaceAttributesRequest request;
            request.addRotation(90.0f);

            auto document = kdl::mem_lock(m_document);
            document->setFaceAttributes(request);
        }

        void UVEditor::rotateTextureCWClicked() {
            Model::ChangeBrushFaceAttributesRequest request;
            request.addRotation(-90.0f);

            auto document = kdl::mem_lock(m_document);
            document->setFaceAttributes(request);
        }

        void UVEditor::subDivisionChanged() {
            const int x = m_xSubDivisionEditor->value();
            const int y = m_ySubDivisionEditor->value();
            m_uvView->setSubDivisions(vm::vec2i(x, y));
        }
    }
}
