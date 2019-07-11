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

#include "SmartColorEditor.h"

#include "CollectionUtils.h"
#include "Model/AttributableNode.h"
#include "Model/Entity.h"
#include "Model/EntityColor.h"
#include "Model/World.h"
#include "View/BorderLine.h"
#include "View/ColorTable.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <QRadioButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

#include <iomanip>

namespace TrenchBroom {
    namespace View {
        SmartColorEditor::SmartColorEditor(View::MapDocumentWPtr document, QWidget* parent) :
        SmartAttributeEditor(document, parent),
        m_floatRadio(nullptr),
        m_byteRadio(nullptr),
        m_colorPicker(nullptr),
        m_colorHistory(nullptr) {
            createGui();
        }

        void SmartColorEditor::OnFloatRangeRadioButton() {
            document()->convertEntityColorRange(name(), Assets::ColorRange::Float);
        }

        void SmartColorEditor::OnByteRangeRadioButton() {
            document()->convertEntityColorRange(name(), Assets::ColorRange::Byte);
        }

        void SmartColorEditor::OnColorPickerChanged(const QColor& color) {
            setColor(color);
        }

        void SmartColorEditor::OnColorTableSelected(QColor color) {
            setColor(color);
        }

        void SmartColorEditor::createGui() {
            assert(m_floatRadio == nullptr);
            assert(m_byteRadio == nullptr);
            assert(m_colorPicker == nullptr);
            assert(m_colorHistory == nullptr);

            auto* rangeTxt = new QLabel(tr("Color range"));
            makeEmphasized(rangeTxt);

            m_floatRadio = new QRadioButton(tr("Float [0,1]"));
            m_byteRadio = new QRadioButton(tr("Byte [0,255]"));
            m_colorPicker = new ColorButton();
            m_colorHistory = new ColorTable(ColorHistoryCellSize);

            auto* leftSizer = new QVBoxLayout();
            leftSizer->setContentsMargins(0, 0, 0, 0);
            leftSizer->addWidget(rangeTxt);
            leftSizer->addWidget(m_floatRadio);
            leftSizer->addWidget(m_byteRadio);
            leftSizer->addWidget(m_colorPicker);
            leftSizer->addStretch(1);

            auto* outerSizer = new QHBoxLayout();
            outerSizer->setContentsMargins(0, 0, 0, 0);
            outerSizer->addSpacing(LayoutConstants::WideHMargin);
            outerSizer->addLayout(leftSizer);
            outerSizer->addSpacing(LayoutConstants::WideHMargin);
            outerSizer->addWidget(new BorderLine(BorderLine::Direction_Vertical), 0);
            outerSizer->addWidget(m_colorHistory, 1);
            setLayout(outerSizer);

            connect(m_floatRadio, &QAbstractButton::clicked, this, &SmartColorEditor::OnFloatRangeRadioButton);
            connect(m_byteRadio, &QAbstractButton::clicked, this, &SmartColorEditor::OnByteRangeRadioButton);
            connect(m_colorPicker, &ColorButton::colorChanged, this, &SmartColorEditor::OnColorPickerChanged);
            connect(m_colorHistory, &ColorTable::colorTableSelected, this, &SmartColorEditor::OnColorTableSelected);
        }

        void SmartColorEditor::doUpdateVisual(const Model::AttributableNodeList& attributables) {
            ensure(m_floatRadio != nullptr, "floatRadio is null");
            ensure(m_byteRadio != nullptr, "byteRadio is null");
            ensure(m_colorPicker != nullptr, "colorPicker is null");
            ensure(m_colorHistory != nullptr, "colorHistory is null");

            updateColorRange(attributables);
            updateColorHistory();
        }

        void SmartColorEditor::updateColorRange(const Model::AttributableNodeList& attributables) {
            const auto range = detectColorRange(name(), attributables);
            if (range == Assets::ColorRange::Float) {
                m_floatRadio->setChecked(true);
                m_byteRadio->setChecked(false);
            } else if (range == Assets::ColorRange::Byte) {
                m_floatRadio->setChecked(false);
                m_byteRadio->setChecked(true);
            } else {
                m_floatRadio->setChecked(false);
                m_byteRadio->setChecked(false);
            }
        }

        struct ColorCmp {
            bool operator()(const QColor& lhs, const QColor& rhs) const {
                const auto lr = lhs.red() / 255.0f;
                const auto lg = lhs.green() / 255.0f;
                const auto lb = lhs.blue() / 255.0f;
                const auto rr = rhs.red() / 255.0f;
                const auto rg = rhs.green() / 255.0f;
                const auto rb = rhs.blue() / 255.0f;

                float lh, ls, lbr, rh, rs, rbr;
                Color::rgbToHSB(lr, lg, lb, lh, ls, lbr);
                Color::rgbToHSB(rr, rg, rb, rh, rs, rbr);

                if (lh < rh) {
                    return true;
                } else if (lh > rh) {
                    return false;
                } else if (ls < rs) {
                    return true;
                } else if (ls > rs) {
                    return false;
                } else if (lbr < rbr) {
                    return true;
                } else {
                    return false;
                }
            }
        };

        class SmartColorEditor::CollectColorsVisitor : public Model::ConstNodeVisitor {
        private:
            const Model::AttributeName& m_name;
            std::vector<QColor> m_colors;
        public:
            CollectColorsVisitor(const Model::AttributeName& name) : m_name(name) {}

            const std::vector<QColor>& colors() const { return m_colors; }
        private:
            void doVisit(const Model::World* world) override   { visitAttributableNode(world); }
            void doVisit(const Model::Layer* layer) override   {}
            void doVisit(const Model::Group* group) override   {}
            void doVisit(const Model::Entity* entity) override { visitAttributableNode(entity); stopRecursion(); }
            void doVisit(const Model::Brush* brush) override   {}

            void visitAttributableNode(const Model::AttributableNode* attributable) {
                static const auto NullValue("");
                const auto& value = attributable->attribute(m_name, NullValue);
                if (value != NullValue)
                    addColor(Model::parseEntityColor(value));
            }

            void addColor(const QColor& color) {
                VectorUtils::setInsert(m_colors, color, ColorCmp());
            }
        };

        void SmartColorEditor::updateColorHistory() {
            CollectColorsVisitor collectAllColors(name());
            document()->world()->acceptAndRecurse(collectAllColors);
            m_colorHistory->setColors(collectAllColors.colors());

            CollectColorsVisitor collectSelectedColors(name());
            const auto nodes = document()->allSelectedAttributableNodes();
            Model::Node::accept(std::begin(nodes), std::end(nodes), collectSelectedColors);

            const auto& selectedColors = collectSelectedColors.colors();
            m_colorHistory->setSelection(selectedColors);

            const QColor color = !selectedColors.empty() ? selectedColors.back() : QColor(Qt::black);
            m_colorPicker->setColor(color);
        }

        void SmartColorEditor::setColor(const QColor& color) const {
            const auto colorRange = m_floatRadio->isChecked() ? Assets::ColorRange::Float : Assets::ColorRange::Byte;
            const auto value = Model::entityColorAsString(color, colorRange);
            document()->setAttribute(name(), value);
        }
    }
}
