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

#include "RotateObjectsToolPage.h"

#include "TrenchBroom.h"
#include "View/BorderLine.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/RotateObjectsTool.h"
#include "View/SpinControl.h"
#include "View/ViewConstants.h"

#include <vecmath/vec.h>
#include <vecmath/util.h>

#include <QAbstractButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>

namespace TrenchBroom {
    namespace View {
        RotateObjectsToolPage::RotateObjectsToolPage(QWidget* parent, MapDocumentWPtr document, RotateObjectsTool* tool) :
        QWidget(parent),
        m_document(document),
        m_tool(tool) {
            createGui();
            m_angle->setValue(vm::toDegrees(m_tool->angle()));
        }

        RotateObjectsToolPage::~RotateObjectsToolPage() {

        }

        void RotateObjectsToolPage::setAxis(const vm::axis::type axis) {
            m_axis->setCurrentIndex(static_cast<int>(axis));
        }

        void RotateObjectsToolPage::setRecentlyUsedCenters(const std::vector<vm::vec3>& centers) {
            m_recentlyUsedCentersList->clear();

            for (auto it = centers.rbegin(), end = centers.rend(); it != end; ++it) {
                const auto& center = *it;
                m_recentlyUsedCentersList->addItem(QString::fromStdString(StringUtils::toString(center)));
            }

            if (m_recentlyUsedCentersList->count() > 0)
                m_recentlyUsedCentersList->setCurrentIndex(0);
        }

        void RotateObjectsToolPage::setCurrentCenter(const vm::vec3& center) {
            m_recentlyUsedCentersList->setCurrentText(QString::fromStdString(StringUtils::toString(center)));
        }

        void RotateObjectsToolPage::createGui() {
            auto* centerText = new QLabel(tr("Center"));
            m_recentlyUsedCentersList = new QComboBox();

            m_resetCenterButton = new QPushButton(tr("Reset"));
            m_resetCenterButton->setToolTip(tr("Reset the position of the rotate handle to the center of the current selection."));

            auto* text1 = new QLabel(tr("Rotate objects"));
            auto* text2 = new QLabel(tr("degs about"));
            auto* text3 = new QLabel(tr("axis"));
            m_angle = new SpinControl(this);
            m_angle->setRange(-360.0, 360.0);
            m_angle->setValue(vm::toDegrees(m_tool->angle()));
            // FIXME:
            //m_angle->setDigits(0, 4);

            m_axis = new QComboBox();
            m_axis->addItem("X");
            m_axis->addItem("Y");
            m_axis->addItem("Z");
            m_axis->setCurrentIndex(2);

            m_rotateButton = new QPushButton(tr("Apply"));

            connect(m_recentlyUsedCentersList, &QComboBox::currentTextChanged, this, &RotateObjectsToolPage::OnCenterChanged);
            connect(m_recentlyUsedCentersList, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &RotateObjectsToolPage::OnCenterChanged);
            connect(m_resetCenterButton, &QAbstractButton::clicked, this, &RotateObjectsToolPage::OnResetCenter);
            connect(m_angle, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &RotateObjectsToolPage::OnAngleChanged);
            connect(m_rotateButton, &QAbstractButton::clicked, this, &RotateObjectsToolPage::OnRotate);

            auto* separator = new BorderLine(BorderLine::Direction_Vertical);
            //separator->SetForegroundColour(Colors::separatorColor());

            auto* sizer = new QHBoxLayout();
            sizer->addWidget(centerText, 0, Qt::AlignVCenter);
            sizer->addSpacing(LayoutConstants::NarrowHMargin);
            sizer->addWidget(m_recentlyUsedCentersList, 0, Qt::AlignVCenter);
            sizer->addSpacing(LayoutConstants::NarrowHMargin);
            sizer->addWidget(m_resetCenterButton, 0, Qt::AlignVCenter);
            sizer->addSpacing(LayoutConstants::MediumHMargin);
            sizer->addWidget(separator, 0); //, wxEXPAND | wxTOP | wxBOTTOM, 2);
            sizer->addSpacing(LayoutConstants::NarrowHMargin);
            sizer->addWidget(text1, 0, Qt::AlignVCenter);
            sizer->addSpacing(LayoutConstants::NarrowHMargin);
            sizer->addWidget(m_angle, 0, Qt::AlignVCenter);
            sizer->addSpacing(LayoutConstants::NarrowHMargin);
            sizer->addWidget(text2, 0, Qt::AlignVCenter);
            sizer->addSpacing(LayoutConstants::NarrowHMargin);
            sizer->addWidget(m_axis, 0); //, wxTOP, LayoutConstants::ChoiceTopMargin);
            sizer->addSpacing(LayoutConstants::NarrowHMargin);
            sizer->addWidget(text3, 0, Qt::AlignVCenter);
            sizer->addSpacing(LayoutConstants::NarrowHMargin);
            sizer->addWidget(m_rotateButton, 0, Qt::AlignVCenter);
            //sizer->SetItemMinSize(m_angle, 80, wxDefaultCoord);

            setLayout(sizer);

            updateGui();
        }

        void RotateObjectsToolPage::updateGui() {
            const auto& grid = lock(m_document)->grid();
            m_angle->SetIncrements(vm::toDegrees(grid.angle()), 90.0, 1.0);

            auto document = lock(m_document);
            m_rotateButton->setEnabled(document->hasSelectedNodes());
        }

        void RotateObjectsToolPage::OnCenterChanged() {
            const auto center = vm::vec3::parse(m_recentlyUsedCentersList->currentText().toStdString());
            m_tool->setRotationCenter(center);
        }

        void RotateObjectsToolPage::OnResetCenter() {
            m_tool->resetRotationCenter();
        }

        void RotateObjectsToolPage::OnAngleChanged(double value) {
            const double newAngleDegs = vm::correct(value);
            m_angle->setValue(newAngleDegs);
            m_tool->setAngle(vm::toRadians(newAngleDegs));
        }

        void RotateObjectsToolPage::OnRotate() {
            const auto center = m_tool->rotationCenter();
            const auto axis = getAxis();
            const auto angle = vm::toRadians(m_angle->value());

            auto document = lock(m_document);
            document->rotateObjects(center, axis, angle);
        }

        vm::vec3 RotateObjectsToolPage::getAxis() const {
            switch (m_axis->currentIndex()) {
                case 0:
                    return vm::vec3::pos_x;
                case 1:
                    return vm::vec3::pos_y;
                default:
                    return vm::vec3::pos_z;
            }
        }
    }
}
