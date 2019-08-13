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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "SpinControl.h"

#include <QDebug>
#include <QGuiApplication>

namespace TrenchBroom {
    namespace View {
        SpinControl::SpinControl(QWidget* parent) :
        QDoubleSpinBox(parent),
        m_regularIncrement(1.0),
        m_shiftIncrement(2.0),
        m_ctrlIncrement(4.0),
        m_minDigits(0),
        m_maxDigits(6) {}

        void SpinControl::stepBy(int steps) {
            if (QGuiApplication::keyboardModifiers() & Qt::ShiftModifier) {
                setSingleStep(m_shiftIncrement);
            } else if (QGuiApplication::keyboardModifiers() & Qt::ControlModifier) {
                setSingleStep(m_ctrlIncrement);
                steps /= 10; // QAbstractSpinBox spins by 10 steps if control is held
            } else {
                setSingleStep(m_regularIncrement);
            }

            qDebug() << "stepping by " << steps << " with step value " << singleStep();

            QDoubleSpinBox::stepBy(steps);
        }

        QString SpinControl::textFromValue(const double val) const {
            auto str = QDoubleSpinBox::textFromValue(val);

            if (m_minDigits < m_maxDigits) {
                const auto zero = locale().zeroDigit();
                while (str.length() > m_minDigits && str.back() == zero) {
                    str.chop(1);
                }
                const auto dec = locale().decimalPoint();
                if (str.back() == dec) {
                    assert(m_minDigits == 0);
                    str.chop(1);
                }
            }

            return str;
        }

        void SpinControl::setIncrements(double regularIncrement, double shiftIncrement, double ctrlIncrement) {
            m_regularIncrement = regularIncrement;
            m_shiftIncrement = shiftIncrement;
            m_ctrlIncrement = ctrlIncrement;
        }

        void SpinControl::setDigits(unsigned int minDigits, unsigned int maxDigits) {
            setDecimals(maxDigits);
        }
    }
}
