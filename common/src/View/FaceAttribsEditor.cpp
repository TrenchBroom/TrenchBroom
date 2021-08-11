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

#include "Color.h"
#include "Assets/Texture.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/Game.h"
#include "Model/GameConfig.h"
#include "Model/MapFormat.h"
#include "Model/WorldNode.h"
#include "View/BorderLine.h"
#include "View/FlagsPopupEditor.h"
#include "View/Grid.h"
#include "View/ViewConstants.h"
#include "View/MapDocument.h"
#include "View/SignalDelayer.h"
#include "View/SpinControl.h"
#include "View/UVEditor.h"
#include "View/ViewUtils.h"
#include "View/QtUtils.h"

#include <kdl/memory_utils.h>
#include <kdl/string_format.h>
#include <kdl/string_utils.h>

#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include <memory>
#include <string>

#include <QtGlobal>
#include <QAbstractButton>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

namespace TrenchBroom {
    namespace View {
        FaceAttribsEditor::FaceAttribsEditor(std::weak_ptr<MapDocument> document, GLContextManager& contextManager, QWidget* parent) :
        QWidget(parent),
        m_document(std::move(document)),
        m_uvEditor(nullptr),
        m_textureName(nullptr),
        m_textureSize(nullptr),
        m_xOffsetEditor(nullptr),
        m_yOffsetEditor(nullptr),
        m_xScaleEditor(nullptr),
        m_yScaleEditor(nullptr),
        m_rotationEditor(nullptr),
        m_surfaceValueLabel(nullptr),
        m_surfaceValueEditorLayout(nullptr),        
        m_surfaceValueEditor(nullptr),
        m_surfaceValueUnsetButton(nullptr),
        m_surfaceFlagsLabel(nullptr),
        m_surfaceFlagsEditorLayout(nullptr),
        m_surfaceFlagsEditor(nullptr),
        m_surfaceFlagsUnsetButton(nullptr),
        m_contentFlagsLabel(nullptr),
        m_contentFlagsEditorLayout(nullptr),
        m_contentFlagsEditor(nullptr),
        m_contentFlagsUnsetButton(nullptr),
        m_colorLabel(nullptr),
        m_colorEditorLayout(nullptr),
        m_colorEditor(nullptr),
        m_colorUnsetButton(nullptr),
        m_updateControlsSignalDelayer{new SignalDelayer{this}} {
            createGui(contextManager);
            bindEvents();
            connectObservers();
            updateIncrements();
        }

        bool FaceAttribsEditor::cancelMouseDrag() {
            return m_uvEditor->cancelMouseDrag();
        }

        void FaceAttribsEditor::xOffsetChanged(const double value) {
            auto document = kdl::mem_lock(m_document);
            if (!document->hasAnySelectedBrushFaces()) {
                return;
            }

            Model::ChangeBrushFaceAttributesRequest request;
            request.setXOffset(static_cast<float>(value));
            if (!document->setFaceAttributes(request)) {
                updateControls();
            }
        }

        void FaceAttribsEditor::yOffsetChanged(const double value) {
            auto document = kdl::mem_lock(m_document);
            if (!document->hasAnySelectedBrushFaces()) {
                return;
            }

            Model::ChangeBrushFaceAttributesRequest request;
            request.setYOffset(static_cast<float>(value));
            if (!document->setFaceAttributes(request)) {
                updateControls();
            }
        }

        void FaceAttribsEditor::rotationChanged(const double value) {
            auto document = kdl::mem_lock(m_document);
            if (!document->hasAnySelectedBrushFaces()) {
                return;
            }

            Model::ChangeBrushFaceAttributesRequest request;
            request.setRotation(static_cast<float>(value));
            if (!document->setFaceAttributes(request)) {
                updateControls();
            }
        }

        void FaceAttribsEditor::xScaleChanged(const double value) {
            auto document = kdl::mem_lock(m_document);
            if (!document->hasAnySelectedBrushFaces()) {
                return;
            }

            Model::ChangeBrushFaceAttributesRequest request;
            request.setXScale(static_cast<float>(value));
            if (!document->setFaceAttributes(request)) {
                updateControls();
            }
        }

        void FaceAttribsEditor::yScaleChanged(const double value) {
            auto document = kdl::mem_lock(m_document);
            if (!document->hasAnySelectedBrushFaces()) {
                return;
            }

            Model::ChangeBrushFaceAttributesRequest request;
            request.setYScale(static_cast<float>(value));
            if (!document->setFaceAttributes(request)) {
                updateControls();
            }
        }

        void FaceAttribsEditor::surfaceFlagChanged(const size_t /* index */, const int value, const int setFlag, const int /* mixedFlag */) {
            auto document = kdl::mem_lock(m_document);
            if (!document->hasAnySelectedBrushFaces()) {
                return;
            }

            Model::ChangeBrushFaceAttributesRequest request;
            if (setFlag & value) {
                request.setSurfaceFlags(value);
            } else {
                request.unsetSurfaceFlags(value);
            }
            if (!document->setFaceAttributes(request)) {
                updateControls();
            }
        }

        void FaceAttribsEditor::contentFlagChanged(const size_t /* index */, const int value, const int setFlag, const int /* mixedFlag */) {
            auto document = kdl::mem_lock(m_document);
            if (!document->hasAnySelectedBrushFaces()) {
                return;
            }

            Model::ChangeBrushFaceAttributesRequest request;
            if (setFlag & value) {
                request.setContentFlags(value);
            } else {
                request.unsetContentFlags(value);
            }
            if (!document->setFaceAttributes(request)) {
                updateControls();
            }
        }

        void FaceAttribsEditor::surfaceValueChanged(const double value) {
            auto document = kdl::mem_lock(m_document);
            if (!document->hasAnySelectedBrushFaces()) {
                return;
            }

            Model::ChangeBrushFaceAttributesRequest request;
            request.setSurfaceValue(static_cast<float>(value));
            if (!document->setFaceAttributes(request)) {
                updateControls();
            }
        }

        void FaceAttribsEditor::colorValueChanged(const QString& /* text */) {
            auto document = kdl::mem_lock(m_document);
            if (!document->hasAnySelectedBrushFaces()) {
                return;
            }

            const std::string str = m_colorEditor->text().toStdString();
            if (!kdl::str_is_blank(str)) {
                if (const auto color = Color::parse(str)) {
                    Model::ChangeBrushFaceAttributesRequest request;
                    request.setColor(*color);
                    if (!document->setFaceAttributes(request)) {
                        updateControls();
                    }
                }
            } else {
                Model::ChangeBrushFaceAttributesRequest request;
                request.setColor(Color());
                if (!document->setFaceAttributes(request)) {
                    updateControls();
                }
            }
        }

        void FaceAttribsEditor::surfaceFlagsUnset() {
            auto document = kdl::mem_lock(m_document);
            if (!document->hasAnySelectedBrushFaces()) {
                return;
            }

            Model::ChangeBrushFaceAttributesRequest request;
            request.replaceSurfaceFlags(std::nullopt);
            if (!document->setFaceAttributes(request)) {
                updateControls();
            }
        }

        void FaceAttribsEditor::contentFlagsUnset() {
            auto document = kdl::mem_lock(m_document);
            if (!document->hasAnySelectedBrushFaces()) {
                return;
            }

            Model::ChangeBrushFaceAttributesRequest request;
            request.replaceContentFlags(std::nullopt);
            if (!document->setFaceAttributes(request)) {
                updateControls();
            }
        }

        void FaceAttribsEditor::surfaceValueUnset() {
            auto document = kdl::mem_lock(m_document);
            if (!document->hasAnySelectedBrushFaces()) {
                return;
            }

            Model::ChangeBrushFaceAttributesRequest request;
            request.setSurfaceValue(std::nullopt);
            if (!document->setFaceAttributes(request)) {
                updateControls();
            }
        }

        void FaceAttribsEditor::colorValueUnset() {
            auto document = kdl::mem_lock(m_document);
            if (!document->hasAnySelectedBrushFaces()) {
                return;
            }

            Model::ChangeBrushFaceAttributesRequest request;
            request.setColor(std::nullopt);
            if (!document->setFaceAttributes(request)) {
                updateControls();
            }
        }

        void FaceAttribsEditor::updateIncrements() {
            auto document = kdl::mem_lock(m_document);
            Grid& grid = document->grid();

            m_xOffsetEditor->setIncrements(grid.actualSize(), 2.0 * grid.actualSize(), 1.0);
            m_yOffsetEditor->setIncrements(grid.actualSize(), 2.0 * grid.actualSize(), 1.0);
            m_rotationEditor->setIncrements(vm::to_degrees(grid.angle()), 90.0, 1.0);
        }

        static QWidget* createUnsetButtonLayout(QWidget* expandWidget, QWidget* button) {
            auto* wrapper = new QWidget();
            auto* rowLayout = new QHBoxLayout();
            rowLayout->setContentsMargins(0, 0, 0, 0);
            rowLayout->setSpacing(LayoutConstants::NarrowHMargin);
            rowLayout->addWidget(expandWidget, 1);
            rowLayout->addWidget(button);
            wrapper->setLayout(rowLayout);
            return wrapper;
        }

        void FaceAttribsEditor::createGui(GLContextManager& contextManager) {
            m_uvEditor = new UVEditor(m_document, contextManager);

            auto* textureNameLabel = new QLabel("Texture");
            makeEmphasized(textureNameLabel);
            m_textureName = new QLabel("none");

            auto* textureSizeLabel = new QLabel("Size");
            makeEmphasized(textureSizeLabel);
            m_textureSize = new QLabel("");

            const auto max = std::numeric_limits<double>::max();
            const auto min = -max;

            auto* xOffsetLabel = new QLabel("X Offset");
            makeEmphasized(xOffsetLabel);
            m_xOffsetEditor = new SpinControl();
            m_xOffsetEditor->setRange(min, max);
            m_xOffsetEditor->setDigits(0, 6);

            auto* yOffsetLabel = new QLabel("Y Offset");
            makeEmphasized(yOffsetLabel);
            m_yOffsetEditor = new SpinControl();
            m_yOffsetEditor->setRange(min, max);
            m_yOffsetEditor->setDigits(0, 6);

            auto* xScaleLabel = new QLabel("X Scale");
            makeEmphasized(xScaleLabel);
            m_xScaleEditor = new SpinControl();
            m_xScaleEditor->setRange(min, max);
            m_xScaleEditor->setIncrements(0.1, 0.25, 0.01);
            m_xScaleEditor->setDigits(0, 6);

            auto* yScaleLabel = new QLabel("Y Scale");
            makeEmphasized(yScaleLabel);
            m_yScaleEditor = new SpinControl();
            m_yScaleEditor->setRange(min, max);
            m_yScaleEditor->setIncrements(0.1, 0.25, 0.01);
            m_yScaleEditor->setDigits(0, 6);

            auto* rotationLabel = new QLabel("Angle");
            makeEmphasized(rotationLabel);
            m_rotationEditor = new SpinControl();
            m_rotationEditor->setRange(min, max);
            m_rotationEditor->setDigits(0, 6);

            m_surfaceValueLabel = new QLabel("Value");
            makeEmphasized(m_surfaceValueLabel);
            m_surfaceValueEditor = new SpinControl();
            m_surfaceValueEditor->setRange(min, max);
            m_surfaceValueEditor->setIncrements(1.0, 10.0, 100.0);
            m_surfaceValueEditor->setDigits(0, 6);
            m_surfaceValueUnsetButton = createBitmapButton("Unset.svg", tr("Unset surface value"));
            m_surfaceValueEditorLayout = createUnsetButtonLayout(m_surfaceValueEditor, m_surfaceValueUnsetButton);

            m_surfaceFlagsLabel = new QLabel("Surface");
            makeEmphasized(m_surfaceFlagsLabel);
            m_surfaceFlagsEditor = new FlagsPopupEditor(2, this);
            m_surfaceFlagsUnsetButton = createBitmapButton("Unset.svg", tr("Unset surface flags"));
            m_surfaceFlagsEditorLayout = createUnsetButtonLayout(m_surfaceFlagsEditor, m_surfaceFlagsUnsetButton);

            m_contentFlagsLabel = new QLabel("Content");
            makeEmphasized(m_contentFlagsLabel);
            m_contentFlagsEditor = new FlagsPopupEditor(2, this);
            m_contentFlagsUnsetButton = createBitmapButton("Unset.svg", tr("Unset content flags"));
            m_contentFlagsEditorLayout = createUnsetButtonLayout(m_contentFlagsEditor, m_contentFlagsUnsetButton);

            m_colorLabel = new QLabel("Color");
            makeEmphasized(m_colorLabel);
            m_colorEditor = new QLineEdit();
            m_colorUnsetButton = createBitmapButton("Unset.svg", tr("Unset color"));
            m_colorEditorLayout = createUnsetButtonLayout(m_colorEditor, m_colorUnsetButton);

            const Qt::Alignment LabelFlags   = Qt::AlignVCenter | Qt::AlignRight;
            const Qt::Alignment ValueFlags   = Qt::AlignVCenter;

            auto* faceAttribsLayout = new QGridLayout();
            faceAttribsLayout->setContentsMargins(
                LayoutConstants::NarrowHMargin,
                LayoutConstants::MediumVMargin,
                LayoutConstants::NarrowHMargin,
                LayoutConstants::MediumVMargin);
            faceAttribsLayout->setHorizontalSpacing(LayoutConstants::MediumHMargin);
            faceAttribsLayout->setVerticalSpacing(LayoutConstants::MediumVMargin);

            int r = 0;
            int c = 0;

            faceAttribsLayout->addWidget(textureNameLabel,     r,c++, LabelFlags);
            faceAttribsLayout->addWidget(m_textureName,        r,c++, ValueFlags);
            faceAttribsLayout->addWidget(textureSizeLabel,     r,c++, LabelFlags);
            faceAttribsLayout->addWidget(m_textureSize,        r,c++, ValueFlags);
            ++r; c = 0;

            faceAttribsLayout->addWidget(xOffsetLabel,         r,c++, LabelFlags);
            faceAttribsLayout->addWidget(m_xOffsetEditor,      r,c++);
            faceAttribsLayout->addWidget(yOffsetLabel,         r,c++, LabelFlags);
            faceAttribsLayout->addWidget(m_yOffsetEditor,      r,c++);
            ++r; c = 0;

            faceAttribsLayout->addWidget(xScaleLabel,          r,c++, LabelFlags);
            faceAttribsLayout->addWidget(m_xScaleEditor,       r,c++);
            faceAttribsLayout->addWidget(yScaleLabel,          r,c++, LabelFlags);
            faceAttribsLayout->addWidget(m_yScaleEditor,       r,c++);
            ++r; c = 0;

            faceAttribsLayout->addWidget(rotationLabel,        r,c++, LabelFlags);
            faceAttribsLayout->addWidget(m_rotationEditor,     r,c++);
            faceAttribsLayout->addWidget(m_surfaceValueLabel,  r,c++, LabelFlags);
            faceAttribsLayout->addWidget(m_surfaceValueEditorLayout, r,c++);
            ++r; c = 0;

            faceAttribsLayout->addWidget(m_surfaceFlagsLabel,  r,c++, LabelFlags);
            faceAttribsLayout->addWidget(m_surfaceFlagsEditorLayout, r,c++, 1,3);
            ++r; c = 0;

            faceAttribsLayout->addWidget(m_contentFlagsLabel,  r,c++, LabelFlags);
            faceAttribsLayout->addWidget(m_contentFlagsEditorLayout, r,c++, 1,3);
            ++r; c = 0;

            faceAttribsLayout->addWidget(m_colorLabel,         r,c++, LabelFlags);
            faceAttribsLayout->addWidget(m_colorEditorLayout,  r,c++, 1,3);
            ++r; c = 0;

            faceAttribsLayout->setColumnStretch(1, 1);
            faceAttribsLayout->setColumnStretch(3, 1);

            auto* outerLayout = new QVBoxLayout();
            outerLayout->setContentsMargins(0, 0, 0, 0);
            outerLayout->setSpacing(LayoutConstants::NarrowVMargin);
            outerLayout->addWidget(m_uvEditor, 1);
            outerLayout->addWidget(new BorderLine());
            outerLayout->addLayout(faceAttribsLayout);

            setLayout(outerLayout);
        }

        void FaceAttribsEditor::bindEvents() {
            connect(m_xOffsetEditor, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                &FaceAttribsEditor::xOffsetChanged);
            connect(m_yOffsetEditor, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                &FaceAttribsEditor::yOffsetChanged);
            connect(m_xScaleEditor, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                &FaceAttribsEditor::xScaleChanged);
            connect(m_yScaleEditor, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                &FaceAttribsEditor::yScaleChanged);
            connect(m_rotationEditor, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                &FaceAttribsEditor::rotationChanged);
            connect(m_surfaceValueEditor, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                &FaceAttribsEditor::surfaceValueChanged);
            connect(m_surfaceFlagsEditor, &FlagsPopupEditor::flagChanged, this, &FaceAttribsEditor::surfaceFlagChanged);
            connect(m_contentFlagsEditor, &FlagsPopupEditor::flagChanged, this, &FaceAttribsEditor::contentFlagChanged);
            connect(m_colorEditor, &QLineEdit::textEdited, this, &FaceAttribsEditor::colorValueChanged);
            connect(m_surfaceValueUnsetButton, &QAbstractButton::clicked, this, &FaceAttribsEditor::surfaceValueUnset);
            connect(m_surfaceFlagsUnsetButton, &QAbstractButton::clicked, this, &FaceAttribsEditor::surfaceFlagsUnset);
            connect(m_contentFlagsUnsetButton, &QAbstractButton::clicked, this, &FaceAttribsEditor::contentFlagsUnset);
            connect(m_colorUnsetButton, &QAbstractButton::clicked, this, &FaceAttribsEditor::colorValueUnset);
            connect(m_updateControlsSignalDelayer, &SignalDelayer::processSignal, this, &FaceAttribsEditor::updateControls);
        }

        void FaceAttribsEditor::connectObservers() {
            auto document = kdl::mem_lock(m_document);
            m_notifierConnection += document->documentWasNewedNotifier.connect(this, &FaceAttribsEditor::documentWasNewed);
            m_notifierConnection += document->documentWasLoadedNotifier.connect(this, &FaceAttribsEditor::documentWasLoaded);
            m_notifierConnection += document->nodesDidChangeNotifier.connect(this, &FaceAttribsEditor::nodesDidChange);
            m_notifierConnection += document->brushFacesDidChangeNotifier.connect(this, &FaceAttribsEditor::brushFacesDidChange);
            m_notifierConnection += document->selectionDidChangeNotifier.connect(this, &FaceAttribsEditor::selectionDidChange);
            m_notifierConnection += document->textureCollectionsDidChangeNotifier.connect(this, &FaceAttribsEditor::textureCollectionsDidChange);
            m_notifierConnection += document->grid().gridDidChangeNotifier.connect(this, &FaceAttribsEditor::updateIncrements);
        }

        void FaceAttribsEditor::documentWasNewed(MapDocument*) {
            updateControls();
        }

        void FaceAttribsEditor::documentWasLoaded(MapDocument*) {
            updateControls();
        }

        void FaceAttribsEditor::nodesDidChange(const std::vector<Model::Node*>&) {
            updateControlsDelayed();
        }

        void FaceAttribsEditor::brushFacesDidChange(const std::vector<Model::BrushFaceHandle>&) {
            updateControlsDelayed();
        }

        void FaceAttribsEditor::selectionDidChange(const Selection&) {
            updateControlsDelayed();
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
            // block signals emitted when updating the editor values
            const QSignalBlocker blockXOffsetEditor(m_xOffsetEditor);
            const QSignalBlocker blockYOffsetEditor(m_yOffsetEditor);
            const QSignalBlocker blockRotationEditor(m_rotationEditor);
            const QSignalBlocker blockXScaleEditor(m_xScaleEditor);
            const QSignalBlocker blockYScaleEditor(m_yScaleEditor);
            const QSignalBlocker blockSurfaceValueEditor(m_surfaceValueEditor);
            const QSignalBlocker blockSurfaceFlagsEditor(m_surfaceFlagsEditor);
            const QSignalBlocker blockContentFlagsEditor(m_contentFlagsEditor);
            const QSignalBlocker blockColorEditor(m_colorEditor);

            if (hasSurfaceFlags()) {
                showSurfaceFlagsEditor();
                QList<int> values;
                QStringList surfaceFlagLabels, surfaceFlagTooltips;
                getSurfaceFlags(values, surfaceFlagLabels, surfaceFlagTooltips);
                m_surfaceFlagsEditor->setFlags(values, surfaceFlagLabels, surfaceFlagTooltips);
            } else {
                hideSurfaceFlagsEditor();
            }

            if (hasContentFlags()) {
                showContentFlagsEditor();
                QList<int> values;
                QStringList contentFlagLabels, contentFlagTooltips;
                getContentFlags(values, contentFlagLabels, contentFlagTooltips);
                m_contentFlagsEditor->setFlags(values, contentFlagLabels, contentFlagTooltips);
            } else {
                hideContentFlagsEditor();
            }

            if (hasColorAttribs()) {
                showColorAttribEditor();
            } else {
                hideColorAttribEditor();
            }

            const auto faceHandles = kdl::mem_lock(m_document)->allSelectedBrushFaces();
            if (!faceHandles.empty()) {
                bool textureMulti = false;
                bool xOffsetMulti = false;
                bool yOffsetMulti = false;
                bool rotationMulti = false;
                bool xScaleMulti = false;
                bool yScaleMulti = false;
                bool surfaceValueMulti = false;
                bool colorValueMulti = false;

                const Model::BrushFace& firstFace = faceHandles[0].face();
                const std::string& textureName = firstFace.attributes().textureName();
                const float xOffset = firstFace.attributes().xOffset();
                const float yOffset = firstFace.attributes().yOffset();
                const float rotation = firstFace.attributes().rotation();
                const float xScale = firstFace.attributes().xScale();
                const float yScale = firstFace.attributes().yScale();
                int setSurfaceFlags = firstFace.surfaceFlags();
                int setSurfaceContents = firstFace.surfaceContents();
                int mixedSurfaceFlags = 0;
                int mixedSurfaceContents = 0;
                const float surfaceValue = firstFace.surfaceValue();                
                const std::optional<Color> colorValue = firstFace.attributes().color();
                bool hasSurfaceValue = firstFace.attributes().surfaceValue().has_value();
                bool hasSurfaceFlags = firstFace.attributes().surfaceFlags().has_value();
                bool hasSurfaceContents = firstFace.attributes().surfaceContents().has_value();
                bool hasColorValue = firstFace.attributes().hasColor();

                for (size_t i = 1; i < faceHandles.size(); i++) {
                    const Model::BrushFace& face = faceHandles[i].face();
                    textureMulti            |= (textureName     != face.attributes().textureName());
                    xOffsetMulti            |= (xOffset         != face.attributes().xOffset());
                    yOffsetMulti            |= (yOffset         != face.attributes().yOffset());
                    rotationMulti           |= (rotation        != face.attributes().rotation());
                    xScaleMulti             |= (xScale          != face.attributes().xScale());
                    yScaleMulti             |= (yScale          != face.attributes().yScale());
                    surfaceValueMulti       |= (surfaceValue    != face.surfaceValue());
                    colorValueMulti         |= (colorValue      != face.attributes().color());
                    hasSurfaceValue         |= face.attributes().surfaceValue().has_value();
                    hasSurfaceFlags         |= face.attributes().surfaceFlags().has_value();
                    hasSurfaceContents      |= face.attributes().surfaceContents().has_value();
                    hasColorValue           |= face.attributes().hasColor();

                    combineFlags(sizeof(int)*8, face.surfaceFlags(), setSurfaceFlags, mixedSurfaceFlags);
                    combineFlags(sizeof(int)*8, face.surfaceContents(), setSurfaceContents, mixedSurfaceContents);
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
                    if (textureName == Model::BrushFaceAttributes::NoTextureName) {
                        m_textureName->setText("none");
                        m_textureName->setEnabled(false);
                        m_textureSize->setText("");
                        m_textureSize->setEnabled(false);
                    } else {
                        const Assets::Texture* texture = firstFace.texture();
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
                setValueOrMulti(m_xOffsetEditor, xOffsetMulti, static_cast<double>(xOffset));
                setValueOrMulti(m_yOffsetEditor, yOffsetMulti, static_cast<double>(yOffset));
                setValueOrMulti(m_rotationEditor, rotationMulti, static_cast<double>(rotation));
                setValueOrMulti(m_xScaleEditor, xScaleMulti, static_cast<double>(xScale));
                setValueOrMulti(m_yScaleEditor, yScaleMulti, static_cast<double>(yScale));
                setValueOrMulti(m_surfaceValueEditor, surfaceValueMulti, static_cast<double>(surfaceValue));
                if (hasColorValue) {
                    if (colorValueMulti) {
                        m_colorEditor->setPlaceholderText("multi");
                        m_colorEditor->setText("");
                    } else {
                        m_colorEditor->setPlaceholderText("");
                        m_colorEditor->setText(QString::fromStdString(kdl::str_to_string(*colorValue)));
                    }
                } else {
                    m_colorEditor->setPlaceholderText("");
                    m_colorEditor->setText("");
                }
                m_surfaceFlagsEditor->setFlagValue(setSurfaceFlags, mixedSurfaceFlags);
                m_contentFlagsEditor->setFlagValue(setSurfaceContents, mixedSurfaceContents);

                m_surfaceValueUnsetButton->setEnabled(hasSurfaceValue);
                m_surfaceFlagsUnsetButton->setEnabled(hasSurfaceFlags);
                m_contentFlagsUnsetButton->setEnabled(hasSurfaceContents);
                m_colorUnsetButton->setEnabled(hasColorValue);
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

                m_surfaceValueUnsetButton->setEnabled(false);
                m_surfaceFlagsUnsetButton->setEnabled(false);
                m_contentFlagsUnsetButton->setEnabled(false);
                m_colorUnsetButton->setEnabled(false);
            }
        }

        void FaceAttribsEditor::updateControlsDelayed() {
            m_updateControlsSignalDelayer->queueSignal();
        }

        bool FaceAttribsEditor::hasSurfaceFlags() const {
            auto document = kdl::mem_lock(m_document);
            const auto game = document->game();
            return !game->surfaceFlags().flags.empty();
        }

        bool FaceAttribsEditor::hasContentFlags() const {
            auto document = kdl::mem_lock(m_document);
            const auto game = document->game();
            return !game->contentFlags().flags.empty();
        }

        void FaceAttribsEditor::showSurfaceFlagsEditor() {
            m_surfaceValueLabel->show();
            m_surfaceValueEditorLayout->show();
            m_surfaceFlagsLabel->show();
            m_surfaceFlagsEditorLayout->show();
        }

        void FaceAttribsEditor::showContentFlagsEditor() {
            m_contentFlagsLabel->show();
            m_contentFlagsEditorLayout->show();
        }

        void FaceAttribsEditor::hideSurfaceFlagsEditor() {
            m_surfaceValueLabel->hide();
            m_surfaceValueEditorLayout->hide();
            m_surfaceFlagsLabel->hide();
            m_surfaceFlagsEditorLayout->hide();
        }

        void FaceAttribsEditor::hideContentFlagsEditor() {
            m_contentFlagsLabel->hide();
            m_contentFlagsEditorLayout->hide();
        }

        bool FaceAttribsEditor::hasColorAttribs() const {
            auto document = kdl::mem_lock(m_document);
            return document->world()->mapFormat() == Model::MapFormat::Daikatana;
        }

        void FaceAttribsEditor::showColorAttribEditor() {
            m_colorLabel->show();
            m_colorEditorLayout->show();
        }

        void FaceAttribsEditor::hideColorAttribEditor() {
            m_colorLabel->hide();
            m_colorEditorLayout->hide();
        }

        void getFlags(const std::vector<Model::FlagConfig>& flags, QList<int>& values, QStringList& names, QStringList& descriptions);
        void getFlags(const std::vector<Model::FlagConfig>& flags, QList<int>& values, QStringList& names, QStringList& descriptions) {
            for (const auto& flag : flags) {
                values.push_back(flag.value);
                names.push_back(QString::fromStdString(flag.name));
                descriptions.push_back(QString::fromStdString(flag.description));
            }
        }

        void FaceAttribsEditor::getSurfaceFlags(QList<int>& values, QStringList& names, QStringList& descriptions) const {
            auto document = kdl::mem_lock(m_document);
            const auto game = document->game();
            const Model::FlagsConfig& surfaceFlags = game->surfaceFlags();
            getFlags(surfaceFlags.flags, values, names, descriptions);
        }

        void FaceAttribsEditor::getContentFlags(QList<int>& values, QStringList& names, QStringList& descriptions) const {
            auto document = kdl::mem_lock(m_document);
            const auto game = document->game();
            const Model::FlagsConfig& contentFlags = game->contentFlags();
            getFlags(contentFlags.flags, values, names, descriptions);
        }
    }
}
