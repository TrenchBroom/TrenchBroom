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

#include <QGuiApplication>

#include "View/QtUtils.h"

#include <kdl/string_utils.h>

#include <cassert>

namespace TrenchBroom
{
namespace View
{
SpinControl::SpinControl(QWidget* parent)
  : QDoubleSpinBox(parent)
  , m_regularIncrement(1.0)
  , m_shiftIncrement(2.0)
  , m_ctrlIncrement(4.0)
  , m_minDigits(0)
  , m_maxDigits(6)
{
  setKeyboardTracking(false);
  updateTooltip();
}

void SpinControl::stepBy(int steps)
{
  if (QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)
  {
    setSingleStep(m_shiftIncrement);
  }
  else if (QGuiApplication::keyboardModifiers() & Qt::ControlModifier)
  {
    setSingleStep(m_ctrlIncrement);
    // QAbstractSpinBox steps by +/-10 if control is held (on most platforms; see #3373)
    steps = (steps > 0) ? 1 : -1;
  }
  else
  {
    setSingleStep(m_regularIncrement);
  }

  QDoubleSpinBox::stepBy(steps);
}

QString SpinControl::textFromValue(const double val) const
{
  auto str = QDoubleSpinBox::textFromValue(val);

  if (m_minDigits < m_maxDigits)
  {
    const auto zero = locale().zeroDigit();
    while (str.length() > m_minDigits && str[str.length() - 1] == zero)
    {
      str.chop(1);
    }
    const auto dec = locale().decimalPoint();
    if (!str.isEmpty() && str[str.length() - 1] == dec)
    {
      assert(m_minDigits == 0);
      str.chop(1);
    }
  }

  return str;
}

void SpinControl::setIncrements(
  const double regularIncrement, const double shiftIncrement, const double ctrlIncrement)
{
  m_regularIncrement = regularIncrement;
  m_shiftIncrement = shiftIncrement;
  m_ctrlIncrement = ctrlIncrement;
  updateTooltip();
}

void SpinControl::setDigits(const int /* minDigits */, const int maxDigits)
{
  setDecimals(maxDigits);
}

void SpinControl::updateTooltip()
{
  setToolTip(tr("Increment: %1 (%2: %3, %4: %5)")
               .arg(QString::fromStdString(kdl::str_to_string(m_regularIncrement)))
               .arg(nativeModifierLabel(Qt::SHIFT))
               .arg(QString::fromStdString(kdl::str_to_string(m_shiftIncrement)))
               .arg(nativeModifierLabel(Qt::CTRL))
               .arg(QString::fromStdString(kdl::str_to_string(m_ctrlIncrement))));
}
} // namespace View
} // namespace TrenchBroom
