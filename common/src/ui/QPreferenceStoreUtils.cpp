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

#include "QPreferenceStoreUtils.h"

#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QJsonObject>
#include <QKeySequence>
#include <QLockFile>
#include <QMessageBox>
#include <QSaveFile>
#include <QTimer>

#include "io/PathQt.h"

#include "kd/reflection_impl.h"

#include <ostream>

namespace tb::ui
{
namespace
{

QLockFile getLockFile(const QString& preferenceFilePath)
{
  return QLockFile{preferenceFilePath + ".lck"};
}

} // namespace

namespace PreferenceErrors
{

kdl_reflect_impl(NoFilePresent);
kdl_reflect_impl(FileAccessError);
kdl_reflect_impl(LockFileError);

bool JsonParseError::operator==(const JsonParseError& other) const
{
  return jsonError.offset == other.jsonError.offset
         && jsonError.error == other.jsonError.error;
}

std::ostream& operator<<(std::ostream& lhs, const JsonParseError& rhs)
{
  return lhs << rhs.jsonError.errorString().toStdString();
}

} // namespace PreferenceErrors

ReadPreferencesResult parsePreferencesFromJson(const QByteArray& jsonData)
{
  auto error = QJsonParseError{};
  const auto document = QJsonDocument::fromJson(jsonData, &error);

  if (error.error != QJsonParseError::NoError || !document.isObject())
  {
    return PreferenceErrors::JsonParseError{error};
  }

  const auto object = document.object();
  auto result = PreferenceValues{};
  for (auto it = object.constBegin(); it != object.constEnd(); ++it)
  {
    result[io::pathFromQString(it.key())] = it.value();
  }
  return result;
}

QByteArray writePreferencesToJson(const PreferenceValues& preferenceValues)
{
  auto rootObject = QJsonObject{};
  for (const auto& [path, preferenceValue] : preferenceValues)
  {
    rootObject[io::pathAsGenericQString(path)] = preferenceValue;
  }

  auto document = QJsonDocument{rootObject};
  return document.toJson(QJsonDocument::Indented);
}

ReadPreferencesResult readPreferencesFromFile(const QString& path)
{
  auto lockFile = getLockFile(path);
  if (!lockFile.lock())
  {
    return PreferenceErrors::LockFileError{};
  }

  auto file = QFile{path};
  if (!file.exists())
  {
    return PreferenceErrors::NoFilePresent{};
  }

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    return PreferenceErrors::FileAccessError{};
  }

  const auto contents = file.readAll();

  file.close();
  lockFile.unlock();

  return parsePreferencesFromJson(contents);
}


WritePreferencesResult writePreferencesToFile(
  const QString& path, const PreferenceValues& preferenceValues)
{
  const auto json = writePreferencesToJson(preferenceValues);

  const auto dirPath = QFileInfo{path}.path();
  if (!QDir{}.mkpath(dirPath))
  {
    return PreferenceErrors::FileAccessError{};
  }

  auto lockFile = getLockFile(path);
  if (!lockFile.lock())
  {
    return PreferenceErrors::LockFileError{};
  }

  auto saveFile = QSaveFile{path};
  if (!saveFile.open(QIODevice::WriteOnly))
  {
    return PreferenceErrors::FileAccessError{};
  }

  const auto written = saveFile.write(json);
  if (written != static_cast<qint64>(json.size()))
  {
    return PreferenceErrors::FileAccessError{};
  }

  if (!saveFile.commit())
  {
    return PreferenceErrors::FileAccessError{};
  }

  return kdl::void_success;
}

} // namespace tb::ui
