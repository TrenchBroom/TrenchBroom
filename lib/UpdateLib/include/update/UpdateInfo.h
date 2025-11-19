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
#include <QUrl>

#include "update/Asset.h"
#include "update/Release.h"
#include "update/Version.h"

namespace upd
{

/**
 * Information about an available update.
 */
struct UpdateInfo
{
  /** The currently installed version. */
  QString currentVersion;
  /** The available update version. */
  QString updateVersion;
  /** The name of the updated release. */
  QString updateName;
  /** The URL to the release page in the browser. */
  QUrl browserUrl;
  /** The asset to download. */
  Asset asset;

  friend bool operator==(const UpdateInfo& lhs, const UpdateInfo& rhs);
  friend bool operator!=(const UpdateInfo& lhs, const UpdateInfo& rhs);
  friend std::ostream& operator<<(std::ostream& lhs, const UpdateInfo& rhs);
};

using ChooseAsset = std::function<std::optional<Asset>(const QList<Asset>&)>;

template <typename Version>
std::optional<UpdateInfo> makeUpdateInfo(
  const Version& currentVersion,
  const Release<Version>& release,
  const DescribeVersion<Version>& describeVersion,
  const ChooseAsset& chooseAsset)
{
  if (auto asset = chooseAsset(release.assets))
  {
    return UpdateInfo{
      describeVersion(currentVersion),
      describeVersion(release.version),
      release.name,
      release.browserUrl,
      std::move(*asset),
    };
  }
  return std::nullopt;
}

} // namespace upd
