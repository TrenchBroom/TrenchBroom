/*
 Copyright (C) 2025 Kristian Duske

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

#pragma once

#include <QLabel>

#include "upd/UpdateController.h"

class QString;

namespace upd
{

/**
 * A QLabel that shows the current update state and lets users interact with it via
 * clickable links.
 */
class UpdateIndicator : public QLabel
{
  Q_OBJECT
private:
  UpdateController& m_updateController;

public:
  explicit UpdateIndicator(UpdateController& updateController, QWidget* parent = nullptr);
  ~UpdateIndicator() override;

private:
  void updateUI(const UpdateControllerState& state);
  void linkActivated(const QString& uri);

  void stateChanged(const UpdateControllerState& state);
};

} // namespace upd
