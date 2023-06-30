/*
 Copyright (C) 2010-2017 Kristian Duske

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

// FIXME: there must not be dependencies from Assets or Model or Renderer to Qt
#include <QMetaType>

#include <filesystem>
#include <string>

namespace TrenchBroom
{
namespace Assets
{
class EntityDefinitionFileSpec
{
private:
  enum class Type
  {
    Builtin,
    External,
    Unset
  };

  Type m_type;
  std::filesystem::path m_path;

public:
  EntityDefinitionFileSpec();

  static EntityDefinitionFileSpec parse(const std::string& str);
  static EntityDefinitionFileSpec builtin(const std::filesystem::path& path);
  static EntityDefinitionFileSpec external(const std::filesystem::path& path);
  static EntityDefinitionFileSpec unset();

  friend bool operator<(
    const EntityDefinitionFileSpec& lhs, const EntityDefinitionFileSpec& rhs);
  friend bool operator==(
    const EntityDefinitionFileSpec& lhs, const EntityDefinitionFileSpec& rhs);
  friend bool operator!=(
    const EntityDefinitionFileSpec& lhs, const EntityDefinitionFileSpec& rhs);

  bool valid() const;
  bool builtin() const;
  bool external() const;

  const std::filesystem::path& path() const;

  std::string asString() const;

private:
  EntityDefinitionFileSpec(Type type, const std::filesystem::path& path);
};
} // namespace Assets
} // namespace TrenchBroom

// Allow storing this class in a QVariant
Q_DECLARE_METATYPE(TrenchBroom::Assets::EntityDefinitionFileSpec)
