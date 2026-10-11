/*
 Copyright (C) 2026 Kristian Duske

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

#include "base/Macros.h"

namespace tb::ui
{

/**
 * Disables native dialogs while an instance exists, and restores the previous setting
 * when it is destroyed.
 *
 * Some dialogs such as message boxes are native on some platforms. A native dialog is
 * shown by the platform instead of as a widget, so a test cannot interact with it, and it
 * can block the test until a user closes it.
 */
class DisableNativeDialogs
{
private:
  bool m_wasDisabled;

public:
  DisableNativeDialogs();
  ~DisableNativeDialogs();

  deleteCopyAndMove(DisableNativeDialogs);
};

} // namespace tb::ui
