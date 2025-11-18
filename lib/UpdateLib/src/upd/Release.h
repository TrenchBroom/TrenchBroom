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

#include <QList>
#include <QString>

#include "upd/Asset.h"

#include <ostream>

namespace upd
{

template <typename Version>
struct Release
{
  Version version;
  bool prerelease;
  bool draft;
  QString name;
  QString browserUrl;
  QList<Asset> assets;

  auto operator<=>(const Release& other) const { return version <=> other.version; }

  bool operator==(const Release& other) const
  {
    return version == other.version && prerelease == other.prerelease
           && draft == other.draft && name == other.name && browserUrl == other.browserUrl
           && assets == other.assets;
  }

  friend std::ostream& operator<<(std::ostream& lhs, const Release& rhs)
  {
    lhs << "Release{";
    lhs << "version: " << rhs.version << ", ";
    lhs << "prerelease: " << rhs.prerelease << ", ";
    lhs << "draft: " << rhs.draft << ", ";
    lhs << "name: " << rhs.name.toStdString() << ", ";
    lhs << "browserUrl: " << rhs.browserUrl.toStdString() << ", ";
    lhs << "assets: [";
    for (int i = 0; i < rhs.assets.size(); ++i)
    {
      lhs << rhs.assets[i];
      if (i < rhs.assets.size() - 1)
      {
        lhs << ", ";
      }
    }
    lhs << "]";
    lhs << "}";
    return lhs;
  }
};

} // namespace upd
