/*
 Copyright (C) 2025 Kristian Duske
 Copyright (C) 2019 Eric Wasylishen

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

#include <QByteArray>
#include <QJsonParseError>
#include <QJsonValue>

#include "Result.h"

#include "kd/reflection_decl.h"

#include <filesystem>
#include <iosfwd>
#include <unordered_map>

class QString;

namespace tb::ui
{

namespace PreferenceErrors
{

struct NoFilePresent
{
  kdl_reflect_decl_empty(NoFilePresent);
};

struct JsonParseError
{
  QJsonParseError jsonError;

  bool operator==(const JsonParseError& other) const;

  friend std::ostream& operator<<(std::ostream& lhs, const JsonParseError& rhs);
};

struct FileAccessError
{
  kdl_reflect_decl_empty(FileAccessError);
};

struct LockFileError
{
  kdl_reflect_decl_empty(LockFileError);
};

} // namespace PreferenceErrors

using PreferenceValues = std::unordered_map<std::filesystem::path, QJsonValue>;

using ReadPreferencesResult = Result<
  PreferenceValues, // Success case
  PreferenceErrors::NoFilePresent,
  PreferenceErrors::JsonParseError,
  PreferenceErrors::FileAccessError,
  PreferenceErrors::LockFileError>;

using WritePreferencesResult =
  Result<void, PreferenceErrors::FileAccessError, PreferenceErrors::LockFileError>;

ReadPreferencesResult parsePreferencesFromJson(const QByteArray& jsonData);
QByteArray writePreferencesToJson(const PreferenceValues& preferenceValues);


ReadPreferencesResult readPreferencesFromFile(const QString& path);
WritePreferencesResult writePreferencesToFile(
  const QString& path, const PreferenceValues& preferenceValues);

} // namespace tb::ui
