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

#include "Color.h"
#include "Assets/ColorRange.h"
#include "Model/AttributableNode.h"
#include "Model/Entity.h"
#include "Model/EntityColor.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/WorldNode.h"
#include "View/BorderLine.h"
#include "View/ColorButton.h"
#include "View/ColorTable.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"
#include "View/QtUtils.h"

#include <kdl/overload.h>
#include <kdl/vector_set.h>

#include <QColor>
#include <QLabel>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QScrollArea>

namespace TrenchBroom {
    namespace View {
        SmartColorEditor::SmartColorEditor(std::weak_ptr<MapDocument> document, QWidget* parent) :
        SmartAttributeEditor(document, parent),
        m_floatRadio(nullptr),
        m_byteRadio(nullptr),
        m_colorPicker(nullptr),
        m_colorHistory(nullptr) {
            createGui();
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

            auto* colorHistoryScroller = new QScrollArea();
            colorHistoryScroller->setWidget(m_colorHistory);
            colorHistoryScroller->setWidgetResizable(true);
            colorHistoryScroller->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

            auto* leftLayout = new QVBoxLayout();
            leftLayout->setContentsMargins(0, 0, 0, 0);
            leftLayout->setSpacing(LayoutConstants::NarrowVMargin);
            leftLayout->addWidget(rangeTxt);
            leftLayout->addWidget(m_floatRadio);
            leftLayout->addWidget(m_byteRadio);
            leftLayout->addWidget(m_colorPicker);
            leftLayout->addStretch(1);

            auto* outerLayout = new QHBoxLayout();
            outerLayout->setContentsMargins(LayoutConstants::WideHMargin, 0, 0, 0);
            outerLayout->setSpacing(0);
            outerLayout->addLayout(leftLayout);
            outerLayout->addSpacing(LayoutConstants::WideHMargin);
            outerLayout->addWidget(new BorderLine(BorderLine::Direction::Vertical));
            outerLayout->addWidget(colorHistoryScroller, 1);
            setLayout(outerLayout);

            connect(m_floatRadio, &QAbstractButton::clicked, this, &SmartColorEditor::floatRangeRadioButtonClicked);
            connect(m_byteRadio, &QAbstractButton::clicked, this, &SmartColorEditor::byteRangeRadioButtonClicked);
            connect(m_colorPicker, &ColorButton::colorChangedByUser, this, &SmartColorEditor::colorPickerChanged);
            connect(m_colorHistory, &ColorTable::colorTableSelected, this, &SmartColorEditor::colorTableSelected);
        }

        void SmartColorEditor::doUpdateVisual(const std::vector<Model::AttributableNode*>& attributables) {
            ensure(m_floatRadio != nullptr, "floatRadio is null");
            ensure(m_byteRadio != nullptr, "byteRadio is null");
            ensure(m_colorPicker != nullptr, "colorPicker is null");
            ensure(m_colorHistory != nullptr, "colorHistory is null");

            updateColorRange(attributables);
            updateColorHistory();
        }

        void SmartColorEditor::updateColorRange(const std::vector<Model::AttributableNode*>& attributables) {
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

        template <typename Node>
        static std::vector<QColor> collectColors(const std::vector<Node*>& nodes, const std::string& attributeName) {
            struct ColorCmp {
                bool operator()(const QColor& lhs, const QColor& rhs) const {
                    const auto lr = static_cast<float>(lhs.red()) / 255.0f;
                    const auto lg = static_cast<float>(lhs.green()) / 255.0f;
                    const auto lb = static_cast<float>(lhs.blue()) / 255.0f;
                    const auto rr = static_cast<float>(rhs.red()) / 255.0f;
                    const auto rg = static_cast<float>(rhs.green()) / 255.0f;
                    const auto rb = static_cast<float>(rhs.blue()) / 255.0f;

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
    
            kdl::vector_set<QColor, ColorCmp> colors;

            const auto visitAttributableNode = [&](const auto* node) {
                if (const auto* value = node->entity().attribute(attributeName)) {
                    colors.insert(toQColor(Model::parseEntityColor(*value)));
                }
            };

            for (const auto* node : nodes) {
                node->accept(kdl::overload(
                    [&](auto&& thisLambda, const Model::WorldNode* world)   { world->visitChildren(thisLambda); visitAttributableNode(world); },
                    [] (auto&& thisLambda, const Model::LayerNode* layer)   { layer->visitChildren(thisLambda); },
                    [] (auto&& thisLambda, const Model::GroupNode* group)   { group->visitChildren(thisLambda); },
                    [&](const Model::EntityNode* entity)                    { visitAttributableNode(entity); },
                    [] (const Model::BrushNode*)                            {}
                ));
            }

            return colors.get_data();
        }

        void SmartColorEditor::updateColorHistory() {
            m_colorHistory->setColors(collectColors(std::vector<Model::Node*>{document()->world()}, name()));

            const auto selectedColors = collectColors(document()->allSelectedAttributableNodes(), name());
            m_colorHistory->setSelection(selectedColors);
            m_colorPicker->setColor(!selectedColors.empty() ? selectedColors.back() : QColor(Qt::black));
        }

        void SmartColorEditor::setColor(const QColor& color) const {
            const auto colorRange = m_floatRadio->isChecked() ? Assets::ColorRange::Float : Assets::ColorRange::Byte;
            const auto value = Model::entityColorAsString(fromQColor(color), colorRange);
            document()->setAttribute(name(), value);
        }

        void SmartColorEditor::floatRangeRadioButtonClicked() {
            document()->convertEntityColorRange(name(), Assets::ColorRange::Float);
        }

        void SmartColorEditor::byteRangeRadioButtonClicked() {
            document()->convertEntityColorRange(name(), Assets::ColorRange::Byte);
        }

        void SmartColorEditor::colorPickerChanged(const QColor& color) {
            setColor(color);
        }

        void SmartColorEditor::colorTableSelected(QColor color) {
            setColor(color);
        }
    }
}
