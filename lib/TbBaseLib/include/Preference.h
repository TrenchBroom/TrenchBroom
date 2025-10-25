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

#include "kd/reflection_impl.h"

#include <filesystem>
#include <iosfwd>

namespace tb
{

enum class PreferencePersistencePolicy
{
  // The preference is stored in the preference store when it changes
  Persistent,
  // The preference can be changed, but changes are not stored persistently
  Transient,
  // The preference cannot be changed at all
  ReadOnly,
};

std::ostream& operator<<(std::ostream& lhs, PreferencePersistencePolicy rhs);

struct PreferenceBase
{
  std::filesystem::path path;
  PreferencePersistencePolicy persistencePolicy;

  PreferenceBase(
    std::filesystem::path path, PreferencePersistencePolicy persistencePolicy);
  virtual ~PreferenceBase();
};

template <typename T>
struct Preference : public PreferenceBase
{
  T defaultValue;

  kdl_reflect_inline(Preference, path, persistencePolicy, defaultValue);

  Preference(
    std::filesystem::path i_path,
    T i_defaultValue,
    PreferencePersistencePolicy i_persistencePolicy =
      PreferencePersistencePolicy::Persistent)
    : PreferenceBase{std::move(i_path), i_persistencePolicy}
    , defaultValue{std::move(i_defaultValue)}
  {
  }
  virtual ~Preference() { }
};

} // namespace tb
