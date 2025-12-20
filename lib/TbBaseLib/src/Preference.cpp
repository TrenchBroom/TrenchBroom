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

#include "Preference.h"

namespace tb
{

std::ostream& operator<<(std::ostream& lhs, const PreferencePersistencePolicy rhs)
{
  switch (rhs)
  {
  case PreferencePersistencePolicy::Persistent:
    lhs << "Persistent";
    break;
  case PreferencePersistencePolicy::Transient:
    lhs << "Transient";
    break;
  case PreferencePersistencePolicy::ReadOnly:
    lhs << "ReadOnly";
    break;
  }

  return lhs;
}

PreferenceBase::PreferenceBase(
  std::filesystem::path i_path, PreferencePersistencePolicy i_persistencePolicy)
  : path{std::move(i_path)}
  , persistencePolicy{i_persistencePolicy}
{
}

PreferenceBase::~PreferenceBase() = default;

} // namespace tb
