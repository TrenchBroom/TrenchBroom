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

#include "CurrentGameIndicator.h"

#include <QPixmap>
#include <QString>

#include "io/ResourceUtils.h"
#include "mdl/GameFactory.h"

#include <filesystem>

namespace tb::View
{

CurrentGameIndicator::CurrentGameIndicator(const std::string& gameName, QWidget* parent)
  : DialogHeader{parent}
{
  auto& gameFactory = mdl::GameFactory::instance();

  const auto gamePath = gameFactory.gamePath(gameName);
  auto iconPath = gameFactory.iconPath(gameName);
  if (iconPath.empty())
  {
    iconPath = std::filesystem::path{"DefaultGameIcon.svg"};
  }

  const auto gameIcon = io::loadPixmapResource(iconPath);
  set(QString::fromStdString(gameName), gameIcon);
}

} // namespace tb::View
