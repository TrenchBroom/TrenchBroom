/*
 Copyright (C) 2010 Kristian Duske

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

#include "ui/EnableDisableTagCallback.h"

#include <QMenu>

namespace tb::ui
{

EnableDisableTagCallback::EnableDisableTagCallback() = default;

size_t EnableDisableTagCallback::selectOption(const std::vector<std::string>& options)
{
  auto menu = QMenu{};
  for (size_t i = 0; i < options.size(); ++i)
  {
    const auto& option = options[i];
    menu.addAction(
      QString::fromStdString(option), this, [&, i]() { m_selectedOption = i; });
  }

  m_selectedOption = options.size();
  menu.exec(QCursor::pos());
  m_selectedOption = std::min(m_selectedOption, options.size());
  return m_selectedOption;
}

} // namespace tb::ui
