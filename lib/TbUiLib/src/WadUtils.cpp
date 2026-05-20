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

#include "ui/WadUtils.h"

#include "PreferenceManager.h"
#include "mdl/Entity.h"
#include "mdl/GameConfig.h"
#include "mdl/GameInfo.h"
#include "mdl/Map.h"
#include "mdl/Map_Entities.h"
#include "mdl/WadPropertyUtils.h"
#include "mdl/WorldNode.h"
#include "ui/ChoosePathTypeDialog.h"
#include "ui/QPathUtils.h"

namespace tb::ui
{
namespace
{

std::vector<std::string> getWadPaths(
  const mdl::Entity& entity, const std::string& wadPropertyKey)
{
  if (const auto* wadPathsStr = entity.property(wadPropertyKey))
  {
    return mdl::splitWadProperty(*wadPathsStr);
  }
  return {};
}

} // namespace

bool addWadPaths(const QStringList& pathQStrs, mdl::Map& map, QWidget* dialogParent)
{
  if (const auto wadPropertyKey = map.gameInfo().gameConfig.materialConfig.property)
  {
    const auto gamePath = pref(map.gameInfo().gamePathPreference);
    auto pathDialog = ChoosePathTypeDialog{
      dialogParent, pathFromQString(pathQStrs.front()), map.path(), gamePath};

    if (pathDialog.exec() == QDialog::Accepted)
    {
      auto wadPaths = getWadPaths(map.worldNode().entity(), *wadPropertyKey);
      std::ranges::transform(
        pathQStrs, std::back_inserter(wadPaths), [&](const auto& pathQStr) {
          return convertToPathType(
                   pathDialog.pathType(), pathFromQString(pathQStr), map.path(), gamePath)
            .generic_string();
        });

      return setEntityProperty(map, *wadPropertyKey, mdl::joinWadProperty(wadPaths));
    }
  }

  return false;
}

} // namespace tb::ui
