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

#include "FaceAttribsEditor.h"

#include "Assets/AssetTypes.h"
#include "Assets/Texture.h"
#include "IO/Path.h"
#include "IO/ResourceUtils.h"
#include "Model/BrushFace.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/Game.h"
#include "Model/GameConfig.h"
#include "Model/MapFormat.h"
#include "Model/World.h"
#include "View/BorderLine.h"
#include "View/FlagsPopupEditor.h"
#include "View/Grid.h"
#include "View/ViewConstants.h"
#include "View/MapDocument.h"
#include "View/SpinControl.h"
#include "View/UVEditor.h"
#include "View/ViewUtils.h"
#include "View/wxUtils.h"

#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QGridLayout>

namespace TrenchBroom {
    namespace View {
        FaceAttribsEditor::FaceAttribsEditor(QWidget* parent, MapDocumentWPtr document, GLContextManager& contextManager) :
        QWidget(parent),
        m_document(document),
        m_uvEditor(nullptr),
        m_xOffsetEditor(nullptr),
        m_yOffsetEditor(nullptr),
        m_xScaleEditor(nullptr),
        m_yScaleEditor(nullptr),
        m_rotationEditor(nullptr),
        m_surfaceValueLabel(nullptr),
        m_surfaceValueEditor(nullptr),
        m_faceAttribsSizer(nullptr),
        m_surfaceFlagsLabel(nullptr),
        m_surfaceFlagsEditor(nullptr),
        m_contentFlagsLabel(nullptr),
        m_contentFlagsEditor(nullptr),
        m_colorLabel(nullptr),
        m_colorEditor(nullptr) {
            createGui(contextManager);
            bindEvents();
            bindObservers();
        }

        FaceAttribsEditor::~FaceAttribsEditor() {
            unbindObservers();
        }

        bool FaceAttribsEditor::cancelMouseDrag() {
            return m_uvEditor->cancelMouseDrag();
        }

        void FaceAttribsEditor::OnXOffsetChanged(double value) {
            Model::ChangeBrushFaceAttributesRequest request;
            request.setXOffset(static_cast<float>(value));

            MapDocumentSPtr document = lock(m_document);
            if (!document->hasSelectedBrushFaces()) {
                return;
            }

            if (!document->setFaceAttributes(request)) {
                ; //event.Veto(); // FIXME: What to do?
            }
        }

        void FaceAttribsEditor::OnYOffsetChanged(double value) {
            Model::ChangeBrushFaceAttributesRequest request;
            request.setYOffset(static_cast<float>(value));

            MapDocumentSPtr document = lock(m_document);
            if (!document->hasSelectedBrushFaces()) {
                return;
            }

            if (!document->setFaceAttributes(request))
                ; //event.Veto(); // FIXME: What to do?
        }

        void FaceAttribsEditor::OnRotationChanged(double value) {
            Model::ChangeBrushFaceAttributesRequest request;
            request.setRotation(static_cast<float>(value));

            MapDocumentSPtr document = lock(m_document);
            if (!document->hasSelectedBrushFaces()) {
                return;
            }

            if (!document->setFaceAttributes(request))
                ; //event.Veto(); // FIXME: What to do?
        }

        void FaceAttribsEditor::OnXScaleChanged(double value) {
            Model::ChangeBrushFaceAttributesRequest request;
            request.setXScale(static_cast<float>(value));

            MapDocumentSPtr document = lock(m_document);
            if (!document->hasSelectedBrushFaces()) {
                return;
            }

            if (!document->setFaceAttributes(request))
                ; //event.Veto(); // FIXME: What to do?
        }

        void FaceAttribsEditor::OnYScaleChanged(double value) {
            Model::ChangeBrushFaceAttributesRequest request;
            request.setYScale(static_cast<float>(value));

            MapDocumentSPtr document = lock(m_document);
            if (!document->hasSelectedBrushFaces()) {
                return;
            }

            if (!document->setFaceAttributes(request))
                ; //event.Veto(); // FIXME: What to do?
        }

        void FaceAttribsEditor::OnSurfaceFlagChanged(size_t index, int setFlag, int mixedFlag) {
            Model::ChangeBrushFaceAttributesRequest request;
            if (setFlag)
                request.setSurfaceFlag(index);
            else
                request.unsetSurfaceFlag(index);

            MapDocumentSPtr document = lock(m_document);
            if (!document->hasSelectedBrushFaces()) {
                return;
            }

            if (!document->setFaceAttributes(request))
                ; //event.Veto(); // FIXME: What to do?
        }

        void FaceAttribsEditor::OnContentFlagChanged(size_t index, int setFlag, int mixedFlag) {
            Model::ChangeBrushFaceAttributesRequest request;
            if (setFlag)
                request.setContentFlag(index);
            else
                request.unsetContentFlag(index);

            MapDocumentSPtr document = lock(m_document);
            if (!document->hasSelectedBrushFaces()) {
                return;
            }

            if (!document->setFaceAttributes(request))
                ; //event.Veto(); // FIXME: What to do?
        }

        void FaceAttribsEditor::OnSurfaceValueChanged(double value) {
            Model::ChangeBrushFaceAttributesRequest request;
            request.setSurfaceValue(static_cast<float>(value));

            MapDocumentSPtr document = lock(m_document);
            if (!document->hasSelectedBrushFaces()) {
                return;
            }

            if (!document->setFaceAttributes(request))
                ; //event.Veto(); // FIXME: What to do?
        }

        void FaceAttribsEditor::OnColorValueChanged(const QString& text) {
            MapDocumentSPtr document = lock(m_document);
            if (!document->hasSelectedBrushFaces()) {
                return;
            }

            const String str = m_colorEditor->text().toStdString();
            if (!StringUtils::isBlank(str)) {
                if (Color::canParse(str)) {
                    Model::ChangeBrushFaceAttributesRequest request;
                    request.setColor(Color::parse(str));
                    document->setFaceAttributes(request);
                }
            } else {
                Model::ChangeBrushFaceAttributesRequest request;
                request.setColor(Color());
                document->setFaceAttributes(request);
            }
        }

        void FaceAttribsEditor::gridDidChange() {
            MapDocumentSPtr document = lock(m_document);
            Grid& grid = document->grid();

            m_xOffsetEditor->SetIncrements(grid.actualSize(), 2.0 * grid.actualSize(), 1.0);
            m_yOffsetEditor->SetIncrements(grid.actualSize(), 2.0 * grid.actualSize(), 1.0);
            m_rotationEditor->SetIncrements(vm::toDegrees(grid.angle()), 90.0, 1.0);
        }

        void FaceAttribsEditor::createGui(GLContextManager& contextManager) {
            m_uvEditor = new UVEditor(this, m_document, contextManager);

            QLabel* textureNameLabel = new QLabel("Texture");
            // FIXME: fonts
            //textureNameLabel->SetFont(textureNameLabel->GetFont().Bold());
            m_textureName = new QLabel("none");

            QLabel* textureSizeLabel = new QLabel("Size");
            // FIXME: fonts
//            textureSizeLabel->SetFont(textureSizeLabel->GetFont().Bold());
            m_textureSize = new QLabel("");

            const double max = std::numeric_limits<double>::max();
            const double min = -max;

            QLabel* xOffsetLabel = new QLabel("X Offset");
            // FIXME: fonts
//            xOffsetLabel->SetFont(xOffsetLabel->GetFont().Bold());
            m_xOffsetEditor = new SpinControl();
            m_xOffsetEditor->SetRange(min, max);
            m_xOffsetEditor->SetDigits(0, 6);

            QLabel* yOffsetLabel = new QLabel("Y Offset");
            // FIXME: fonts
//            yOffsetLabel->SetFont(yOffsetLabel->GetFont().Bold());
            m_yOffsetEditor = new SpinControl();
            m_yOffsetEditor->SetRange(min, max);
            m_yOffsetEditor->SetDigits(0, 6);

            QLabel* xScaleLabel = new QLabel("X Scale");
            // FIXME: fonts
//            xScaleLabel->SetFont(xScaleLabel->GetFont().Bold());
            m_xScaleEditor = new SpinControl();
            m_xScaleEditor->SetRange(min, max);
            m_xScaleEditor->SetIncrements(0.1, 0.25, 0.01);
            m_xScaleEditor->SetDigits(0, 6);

            QLabel* yScaleLabel = new QLabel("Y Scale");
            // FIXME: fonts
//            yScaleLabel->SetFont(yScaleLabel->GetFont().Bold());
            m_yScaleEditor = new SpinControl();
            m_yScaleEditor->SetRange(min, max);
            m_yScaleEditor->SetIncrements(0.1, 0.25, 0.01);
            m_yScaleEditor->SetDigits(0, 6);

            QLabel* rotationLabel = new QLabel("Angle");
            // FIXME: fonts
//            rotationLabel->SetFont(rotationLabel->GetFont().Bold());
            m_rotationEditor = new SpinControl();
            m_rotationEditor->SetRange(min, max);
            m_rotationEditor->SetDigits(0, 6);

            m_surfaceValueLabel = new QLabel("Value");
            // FIXME: fonts
//            m_surfaceValueLabel->SetFont(m_surfaceValueLabel->GetFont().Bold());
            m_surfaceValueEditor = new SpinControl();
            m_surfaceValueEditor->SetRange(min, max);
            m_surfaceValueEditor->SetIncrements(1.0, 10.0, 100.0);
            m_surfaceValueEditor->SetDigits(0, 6);

            m_surfaceFlagsLabel = new QLabel("Surface");
            // FIXME: fonts
//            m_surfaceFlagsLabel->SetFont(m_surfaceFlagsLabel->GetFont().Bold());
            m_surfaceFlagsEditor = new FlagsPopupEditor(this, 2);

            m_contentFlagsLabel = new QLabel("Content");
            // FIXME: fonts
//            m_contentFlagsLabel->SetFont(m_contentFlagsLabel->GetFont().Bold());
            m_contentFlagsEditor = new FlagsPopupEditor(this, 2);

            m_colorLabel = new QLabel("Color");
            // FIXME: fonts
//            m_colorLabel->SetFont(m_colorLabel->GetFont().Bold());
            m_colorEditor = new QLineEdit();

//            const int LabelMargin  = LayoutConstants::NarrowHMargin;
//            const int EditorMargin = LayoutConstants::WideHMargin;
//            const int RowMargin    = LayoutConstants::NarrowVMargin;

            const Qt::Alignment LabelFlags   = Qt::AlignVCenter | Qt::AlignRight; // wxALIGN_RIGHT | Qt::AlignVCenter | wxRIGHT;
            const Qt::Alignment ValueFlags   = Qt::AlignVCenter; //Qt::AlignVCenter | wxRIGHT;
            const Qt::Alignment Editor1Flags = 0;
            const Qt::Alignment Editor2Flags = 0;

            int r = 0;
            int c = 0;

            m_faceAttribsSizer = new QGridLayout();
            m_faceAttribsSizer->addWidget(textureNameLabel,     r,c++, LabelFlags);
            m_faceAttribsSizer->addWidget(m_textureName,        r,c++, ValueFlags);
            m_faceAttribsSizer->addWidget(textureSizeLabel,     r,c++, LabelFlags);
            m_faceAttribsSizer->addWidget(m_textureSize,        r,c++, ValueFlags);
            ++r; c = 0;

            m_faceAttribsSizer->addWidget(xOffsetLabel,         r,c++, LabelFlags);
            m_faceAttribsSizer->addWidget(m_xOffsetEditor,      r,c++, Editor1Flags);
            m_faceAttribsSizer->addWidget(yOffsetLabel,         r,c++, LabelFlags);
            m_faceAttribsSizer->addWidget(m_yOffsetEditor,      r,c++, Editor2Flags);
            ++r; c = 0;

            m_faceAttribsSizer->addWidget(xScaleLabel,          r,c++, LabelFlags);
            m_faceAttribsSizer->addWidget(m_xScaleEditor,       r,c++, Editor1Flags);
            m_faceAttribsSizer->addWidget(yScaleLabel,          r,c++, LabelFlags);
            m_faceAttribsSizer->addWidget(m_yScaleEditor,       r,c++, Editor2Flags);
            ++r; c = 0;

            m_faceAttribsSizer->addWidget(rotationLabel,        r,c++, LabelFlags);
            m_faceAttribsSizer->addWidget(m_rotationEditor,     r,c++, Editor1Flags);
            m_faceAttribsSizer->addWidget(m_surfaceValueLabel,  r,c++, LabelFlags);
            m_faceAttribsSizer->addWidget(m_surfaceValueEditor, r,c++, Editor2Flags);
            ++r; c = 0;

            m_faceAttribsSizer->addWidget(m_surfaceFlagsLabel,  r,c++, LabelFlags);
            m_faceAttribsSizer->addWidget(m_surfaceFlagsEditor, r,c++, 1,3, Editor2Flags);
            ++r; c = 0;

            m_faceAttribsSizer->addWidget(m_contentFlagsLabel,  r,c++, LabelFlags);
            m_faceAttribsSizer->addWidget(m_contentFlagsEditor, r,c++, 1,3, Editor2Flags);
            ++r; c = 0;

            m_faceAttribsSizer->addWidget(m_colorLabel,         r,c++, LabelFlags);
            m_faceAttribsSizer->addWidget(m_colorEditor,        r,c++, 1,3, Editor2Flags);
            ++r; c = 0;

            m_faceAttribsSizer->setColumnStretch(1, 1);
            m_faceAttribsSizer->setColumnStretch(3, 1);

//            m_faceAttribsSizer->SetItemMinSize(m_uvEditor, 100, 100);
//            m_faceAttribsSizer->SetItemMinSize(m_xOffsetEditor, 50, m_xOffsetEditor->GetSize().y);
//            m_faceAttribsSizer->SetItemMinSize(m_yOffsetEditor, 50, m_yOffsetEditor->GetSize().y);
//            m_faceAttribsSizer->SetItemMinSize(m_xScaleEditor, 50, m_xScaleEditor->GetSize().y);
//            m_faceAttribsSizer->SetItemMinSize(m_yScaleEditor, 50, m_yScaleEditor->GetSize().y);
//            m_faceAttribsSizer->SetItemMinSize(m_rotationEditor, 50, m_rotationEditor->GetSize().y);
//            m_faceAttribsSizer->SetItemMinSize(m_surfaceValueEditor, 50, m_rotationEditor->GetSize().y);

            auto* outerSizer = new QVBoxLayout();
            outerSizer->setContentsMargins(0, 0, 0, 0);
            outerSizer->addWidget(m_uvEditor, 1);
            outerSizer->addWidget(new BorderLine(BorderLine::Direction_Horizontal)); //, 0, wxEXPAND);
//            outerSizer->addSpacing(LayoutConstants::MediumVMargin);
            outerSizer->addLayout(m_faceAttribsSizer); //, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::MediumHMargin);
//            outerSizer->addSpacing(LayoutConstants::MediumVMargin);

            setLayout(outerSizer);
        }

        void FaceAttribsEditor::bindEvents() {
            connect(m_xOffsetEditor, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FaceAttribsEditor::OnXOffsetChanged);
            connect(m_yOffsetEditor, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FaceAttribsEditor::OnYOffsetChanged);
            connect(m_xScaleEditor, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FaceAttribsEditor::OnXScaleChanged);
            connect(m_yScaleEditor, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FaceAttribsEditor::OnYScaleChanged);
            connect(m_rotationEditor, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FaceAttribsEditor::OnRotationChanged);
            connect(m_surfaceValueEditor, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FaceAttribsEditor::OnSurfaceValueChanged);
            connect(m_surfaceFlagsEditor, &FlagsPopupEditor::flagChanged, this, &FaceAttribsEditor::OnSurfaceFlagChanged);
            connect(m_contentFlagsEditor, &FlagsPopupEditor::flagChanged, this, &FaceAttribsEditor::OnContentFlagChanged);
            connect(m_colorEditor, &QLineEdit::textEdited, this, &FaceAttribsEditor::OnColorValueChanged);
        }

        void FaceAttribsEditor::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &FaceAttribsEditor::documentWasNewed);
            document->documentWasLoadedNotifier.addObserver(this, &FaceAttribsEditor::documentWasLoaded);
            document->brushFacesDidChangeNotifier.addObserver(this, &FaceAttribsEditor::brushFacesDidChange);
            document->selectionDidChangeNotifier.addObserver(this, &FaceAttribsEditor::selectionDidChange);
            document->textureCollectionsDidChangeNotifier.addObserver(this, &FaceAttribsEditor::textureCollectionsDidChange);
            document->grid().gridDidChangeNotifier.addObserver(this, &FaceAttribsEditor::gridDidChange);
        }

        void FaceAttribsEditor::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &FaceAttribsEditor::documentWasNewed);
                document->documentWasLoadedNotifier.removeObserver(this, &FaceAttribsEditor::documentWasLoaded);
                document->brushFacesDidChangeNotifier.removeObserver(this, &FaceAttribsEditor::brushFacesDidChange);
                document->selectionDidChangeNotifier.removeObserver(this, &FaceAttribsEditor::selectionDidChange);
                document->textureCollectionsDidChangeNotifier.removeObserver(this, &FaceAttribsEditor::textureCollectionsDidChange);
                document->grid().gridDidChangeNotifier.removeObserver(this, &FaceAttribsEditor::gridDidChange);
            }
        }

        void FaceAttribsEditor::documentWasNewed(MapDocument* document) {
            m_faces = document->allSelectedBrushFaces();
            updateControls();
        }

        void FaceAttribsEditor::documentWasLoaded(MapDocument* document) {
            m_faces = document->allSelectedBrushFaces();
            updateControls();
        }

        void FaceAttribsEditor::brushFacesDidChange(const Model::BrushFaceList& faces) {
            MapDocumentSPtr document = lock(m_document);
            m_faces = document->allSelectedBrushFaces();
            updateControls();
        }

        void FaceAttribsEditor::selectionDidChange(const Selection& selection) {
            MapDocumentSPtr document = lock(m_document);
            m_faces = document->allSelectedBrushFaces();
            updateControls();
        }

        void FaceAttribsEditor::textureCollectionsDidChange() {
            updateControls();
        }

        static void disableAndSetPlaceholder(QDoubleSpinBox* box, const QString& text) {
            box->setSpecialValueText(text);
            box->setValue(box->minimum());
            box->setEnabled(false);
        }

        static void setValueOrMulti(QDoubleSpinBox* box, const bool multi, const double value) {
            if (multi) {
                box->setSpecialValueText("multi");
                box->setValue(box->minimum());
            } else {
                box->setSpecialValueText("");
                box->setValue(value);
            }
        }

        void FaceAttribsEditor::updateControls() {
            if (hasSurfaceAttribs()) {
                showSurfaceAttribEditors();
                QStringList surfaceFlagLabels, surfaceFlagTooltips, contentFlagLabels, contentFlagTooltips;
                getSurfaceFlags(surfaceFlagLabels, surfaceFlagTooltips);
                getContentFlags(contentFlagLabels, contentFlagTooltips);
                m_surfaceFlagsEditor->setFlags(surfaceFlagLabels, surfaceFlagTooltips);
                m_contentFlagsEditor->setFlags(contentFlagLabels, contentFlagTooltips);
            } else {
                hideSurfaceAttribEditors();
            }

            if (hasColorAttribs()) {
                showColorAttribEditor();
            } else {
                hideColorAttribEditor();
            }

            if (!m_faces.empty()) {
                bool textureMulti = false;
                bool xOffsetMulti = false;
                bool yOffsetMulti = false;
                bool rotationMulti = false;
                bool xScaleMulti = false;
                bool yScaleMulti = false;
                bool surfaceValueMulti = false;
                bool colorValueMulti = false;

                Assets::Texture* texture = m_faces[0]->texture();
                const float xOffset = m_faces[0]->xOffset();
                const float yOffset = m_faces[0]->yOffset();
                const float rotation = m_faces[0]->rotation();
                const float xScale = m_faces[0]->xScale();
                const float yScale = m_faces[0]->yScale();
                int setSurfaceFlags = m_faces[0]->surfaceFlags();
                int setSurfaceContents = m_faces[0]->surfaceContents();
                int mixedSurfaceFlags = 0;
                int mixedSurfaceContents = 0;
                const float surfaceValue = m_faces[0]->surfaceValue();
                bool hasColorValue = m_faces[0]->hasColor();
                const Color colorValue = m_faces[0]->color();


                for (size_t i = 1; i < m_faces.size(); i++) {
                    Model::BrushFace* face = m_faces[i];
                    textureMulti            |= (texture         != face->texture());
                    xOffsetMulti            |= (xOffset         != face->xOffset());
                    yOffsetMulti            |= (yOffset         != face->yOffset());
                    rotationMulti           |= (rotation        != face->rotation());
                    xScaleMulti             |= (xScale          != face->xScale());
                    yScaleMulti             |= (yScale          != face->yScale());
                    surfaceValueMulti       |= (surfaceValue    != face->surfaceValue());
                    colorValueMulti         |= (colorValue      != face->color());
                    hasColorValue           |= face->hasColor();

                    combineFlags(sizeof(int)*8, face->surfaceFlags(), setSurfaceFlags, mixedSurfaceFlags);
                    combineFlags(sizeof(int)*8, face->surfaceContents(), setSurfaceContents, mixedSurfaceContents);
                }

                m_xOffsetEditor->setEnabled(true);
                m_yOffsetEditor->setEnabled(true);
                m_rotationEditor->setEnabled(true);
                m_xScaleEditor->setEnabled(true);
                m_yScaleEditor->setEnabled(true);
                m_surfaceValueEditor->setEnabled(true);
                m_surfaceFlagsEditor->setEnabled(true);
                m_contentFlagsEditor->setEnabled(true);
                m_colorEditor->setEnabled(true);

                if (textureMulti) {
                    m_textureName->setText("multi");
                    m_textureName->setEnabled(false);
                    m_textureSize->setText("multi");
                    m_textureSize->setEnabled(false);
                } else {
                    const String& textureName = m_faces[0]->textureName();
                    if (textureName == Model::BrushFace::NoTextureName) {
                        m_textureName->setText("none");
                        m_textureName->setEnabled(false);
                        m_textureSize->setText("");
                        m_textureSize->setEnabled(false);
                    } else {
                        if (texture != nullptr) {
                            m_textureName->setText(QString::fromStdString(textureName));
                            m_textureSize->setText(QStringLiteral("%1 * %2").arg(texture->width()).arg(texture->height()));
                            m_textureName->setEnabled(true);
                            m_textureSize->setEnabled(true);
                        } else {
                            m_textureName->setText(QString::fromStdString(textureName) + " (not found)");
                            m_textureName->setEnabled(false);
                            m_textureSize->setEnabled(false);
                        }
                    }
                }
                setValueOrMulti(m_xOffsetEditor, xOffsetMulti, xOffset);
                setValueOrMulti(m_yOffsetEditor, yOffsetMulti, yOffset);
                setValueOrMulti(m_rotationEditor, rotationMulti, rotation);
                setValueOrMulti(m_xScaleEditor, xScaleMulti, xScale);
                setValueOrMulti(m_yScaleEditor, yScaleMulti, yScale);
                setValueOrMulti(m_surfaceValueEditor, surfaceValueMulti, surfaceValue);
                if (hasColorValue) {
                    if (colorValueMulti) {
                        m_colorEditor->setPlaceholderText("multi");
                        m_colorEditor->setText("");
                    } else {
                        m_colorEditor->setPlaceholderText("");
                        m_colorEditor->setText(QString::fromStdString(StringUtils::toString(colorValue)));
                    }
                } else {
                    m_colorEditor->setPlaceholderText("");
                    m_colorEditor->setText("");
                }
                m_surfaceFlagsEditor->setFlagValue(setSurfaceFlags, mixedSurfaceFlags);
                m_contentFlagsEditor->setFlagValue(setSurfaceContents, mixedSurfaceContents);
            } else {
                disableAndSetPlaceholder(m_xOffsetEditor, "n/a");
                disableAndSetPlaceholder(m_yOffsetEditor, "n/a");
                disableAndSetPlaceholder(m_xScaleEditor, "n/a");
                disableAndSetPlaceholder(m_yScaleEditor, "n/a");
                disableAndSetPlaceholder(m_rotationEditor, "n/a");
                disableAndSetPlaceholder(m_surfaceValueEditor, "n/a");

                // m_textureView->setTexture(nullptr);
                m_surfaceFlagsEditor->setEnabled(false);
                m_contentFlagsEditor->setEnabled(false);
                m_colorEditor->setText("");
                m_colorEditor->setPlaceholderText("n/a");
                m_colorEditor->setEnabled(false);
            }
        }


        bool FaceAttribsEditor::hasSurfaceAttribs() const {
            MapDocumentSPtr document = lock(m_document);
            const Model::GameSPtr game = document->game();
            const Model::GameConfig::FlagsConfig& surfaceFlags = game->surfaceFlags();
            const Model::GameConfig::FlagsConfig& contentFlags = game->contentFlags();

            return !surfaceFlags.flags.empty() && !contentFlags.flags.empty();
        }

        void FaceAttribsEditor::showSurfaceAttribEditors() {
            m_surfaceValueLabel->show();
            m_surfaceValueEditor->show();
            m_surfaceFlagsLabel->show();
            m_surfaceFlagsEditor->show();
            m_contentFlagsLabel->show();
            m_contentFlagsEditor->show();
        }

        void FaceAttribsEditor::hideSurfaceAttribEditors() {
            m_surfaceValueLabel->hide();
            m_surfaceValueEditor->hide();
            m_surfaceFlagsLabel->hide();
            m_surfaceFlagsEditor->hide();
            m_contentFlagsLabel->hide();
            m_contentFlagsEditor->hide();
        }

        bool FaceAttribsEditor::hasColorAttribs() const {
            MapDocumentSPtr document = lock(m_document);
            return document->world()->format() == Model::MapFormat::Daikatana;
        }

        void FaceAttribsEditor::showColorAttribEditor() {
            m_colorLabel->show();
            m_colorEditor->show();
        }

        void FaceAttribsEditor::hideColorAttribEditor() {
            m_colorLabel->hide();
            m_colorEditor->hide();
        }

        void getFlags(const Model::GameConfig::FlagConfigList& flags, QStringList& names, QStringList& descriptions);
        void getFlags(const Model::GameConfig::FlagConfigList& flags, QStringList& names, QStringList& descriptions) {
            for (const auto& flag : flags) {
                names.push_back(QString::fromStdString(flag.name));
                descriptions.push_back(QString::fromStdString(flag.description));
            }
        }

        void FaceAttribsEditor::getSurfaceFlags(QStringList& names, QStringList& descriptions) const {
            MapDocumentSPtr document = lock(m_document);
            const Model::GameSPtr game = document->game();
            const Model::GameConfig::FlagsConfig& surfaceFlags = game->surfaceFlags();
            getFlags(surfaceFlags.flags, names, descriptions);
        }

        void FaceAttribsEditor::getContentFlags(QStringList& names, QStringList& descriptions) const {
            MapDocumentSPtr document = lock(m_document);
            const Model::GameSPtr game = document->game();
            const Model::GameConfig::FlagsConfig& contentFlags = game->contentFlags();
            getFlags(contentFlags.flags, names, descriptions);
        }
    }
}
