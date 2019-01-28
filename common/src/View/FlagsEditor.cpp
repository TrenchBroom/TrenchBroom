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

#include "FlagsEditor.h"

#include "View/ViewConstants.h"
#include "Macros.h"

#include <QGridLayout>
#include <QCheckBox>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        FlagsEditor::FlagsEditor(QWidget* parent, const size_t numCols) :
        QWidget(parent),
        m_numCols(numCols) {
            assert(m_numCols > 0);
        }
        
        void FlagsEditor::setFlags(const QStringList& labels, const QStringList& tooltips) {
            QList<int> values;
            values.reserve(labels.size());

            for (int i = 0; i < labels.size(); ++i) {
                values.push_back(1 << i);
            }
            setFlags(values, labels, tooltips);
        }

        void FlagsEditor::setFlags(const QList<int>& values, const QStringList& labels, const QStringList& tooltips) {
            const size_t count = static_cast<size_t>(values.size());

            const size_t numRows = count / m_numCols;

            auto* sizer = new QGridLayout();
            // deletes the old checkboxes
            setLayout(sizer);

            sizer->setHorizontalSpacing(LayoutConstants::WideHMargin);
            sizer->setVerticalSpacing(0);

            m_values.clear();
            m_checkBoxes.clear();

            m_values.resize(count);
            m_checkBoxes.resize(count);

            for (size_t row = 0; row < numRows; ++row) {
                for (size_t col = 0; col < m_numCols; ++col) {
                    const size_t index = col * numRows + row;
                    if (index < count) {
                        auto* checkBox = new QCheckBox();
                        m_checkBoxes[index] = checkBox;
                        m_values[index] = values[index];

                        const int indexInt = static_cast<int>(index);
                        const int rowInt = static_cast<int>(row);
                        const int colInt = static_cast<int>(col);

                        checkBox->setTristate(true);
                        checkBox->setText(indexInt < labels.size() ? labels[indexInt] : QString::number(1 << index));
                        checkBox->setToolTip(indexInt < tooltips.size() ? tooltips[indexInt] : "");
                        connect(checkBox, &QCheckBox::stateChanged, this, [=](int state){
                            emit flagChanged(index, this->getSetFlagValue(), this->getMixedFlagValue());
                        });

                        sizer->addWidget(checkBox, rowInt, colInt);
                    }
                }
            }
        }
        
        void FlagsEditor::setFlagValue(const int on, const int mixed) {
            for (size_t i = 0; i < m_checkBoxes.size(); ++i) {
                QCheckBox* checkBox = m_checkBoxes[i];
                const int value = m_values[i];
                const bool isMixed = (mixed & value) != 0;
                const bool isChecked = (on & value) != 0;
                if (isMixed)
                    checkBox->setCheckState(Qt::PartiallyChecked);
                else if (isChecked)
                    checkBox->setCheckState(Qt::Checked);
                else
                    checkBox->setCheckState(Qt::Unchecked);
            }
        }
        
        size_t FlagsEditor::getNumFlags() const {
            return m_checkBoxes.size();
        }

        bool FlagsEditor::isFlagSet(const size_t index) const {
            ensure(index < m_checkBoxes.size(), "index out of range");
            return m_checkBoxes[index]->checkState() == Qt::Checked;
        }
        
        bool FlagsEditor::isFlagMixed(const size_t index) const {
            ensure(index < m_checkBoxes.size(), "index out of range");
            return m_checkBoxes[index]->checkState() == Qt::PartiallyChecked;
        }

        int FlagsEditor::getSetFlagValue() const {
            int value = 0;
            for (size_t i = 0; i < m_checkBoxes.size(); ++i) {
                if (isFlagSet(i))
                    value |= m_values[i];
            }
            return value;
        }
        
        int FlagsEditor::getMixedFlagValue() const {
            int value = 0;
            for (size_t i = 0; i < m_checkBoxes.size(); ++i) {
                if (isFlagMixed(i))
                    value |= m_values[i];
            }
            return value;
        }

        QString FlagsEditor::getFlagLabel(const size_t index) const {
            ensure(index < m_checkBoxes.size(), "index out of range");
            return m_checkBoxes[index]->text();
        }

        int FlagsEditor::lineHeight() const {
            assert(!m_checkBoxes.empty());
            return m_checkBoxes.front()->frameSize().height();
        }
    }
}
