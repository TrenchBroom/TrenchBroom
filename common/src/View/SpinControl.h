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

#pragma once

#include <QDoubleSpinBox>

namespace TrenchBroom
{
namespace View
{
class SpinControl : public QDoubleSpinBox
{
  Q_OBJECT
private:
  double m_regularIncrement;
  double m_shiftIncrement;
  double m_ctrlIncrement;
  int m_minDigits;
  int m_maxDigits;

public:
  explicit SpinControl(QWidget* parent = nullptr);

public: // QDoubleSpinBox overrides
  void stepBy(int steps) override;

  QString textFromValue(double val) const override;

public:
  void setIncrements(
    double regularIncrement, double shiftIncrement, double ctrlIncrement);
  void setDigits(int minDigits, int maxDigits);

private:
  void updateTooltip();
};
} // namespace View
} // namespace TrenchBroom
