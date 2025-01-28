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

#include "UpdateInfo.h"

namespace upd
{

bool operator==(const UpdateInfo& lhs, const UpdateInfo& rhs)
{
  return lhs.currentVersion == rhs.currentVersion
         && lhs.updateVersion == rhs.updateVersion && lhs.updateName == rhs.updateName
         && lhs.browserUrl == rhs.browserUrl && lhs.asset == rhs.asset;
}

bool operator!=(const UpdateInfo& lhs, const UpdateInfo& rhs)
{
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& lhs, const UpdateInfo& rhs)
{
  lhs << "UpdateInfo{";
  lhs << "currentVersion: " << rhs.currentVersion.toStdString() << ", ";
  lhs << "updateVersion: " << rhs.updateVersion.toStdString() << ", ";
  lhs << "updateName: " << rhs.updateName.toStdString() << ", ";
  lhs << "browserUrl: " << rhs.browserUrl.toString().toStdString() << ", ";
  lhs << "asset: " << rhs.asset;
  lhs << "}";
  return lhs;
}

} // namespace upd
