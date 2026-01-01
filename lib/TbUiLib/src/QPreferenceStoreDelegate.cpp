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

#include "ui/QPreferenceStoreDelegate.h"

#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QJsonObject>
#include <QJsonValue>
#include <QKeySequence>
#include <QLockFile>
#include <QMessageBox>
#include <QSaveFile>
#include <QTimer>

#include "Macros.h"
#include "ui/QPathUtils.h"

#include <unordered_map>

namespace tb::ui
{
namespace
{

std::vector<std::filesystem::path> changedKeysForMapDiff(
  const std::unordered_map<std::filesystem::path, QJsonValue>& before,
  const std::unordered_map<std::filesystem::path, QJsonValue>& after)
{
  auto result = std::vector<std::filesystem::path>{};

  // removes
  for (const auto& [k, v] : before)
  {
    unused(v);
    if (after.find(k) == after.end())
    {
      // removal
      result.push_back(k);
    }
  }

  // adds/updates
  for (auto& [k, v] : after)
  {
    auto beforeIt = before.find(k);
    if (beforeIt == before.end())
    {
      // add
      result.push_back(k);
    }
    else
    {
      // check for update?
      if (beforeIt->second != v)
      {
        result.push_back(k);
      }
    }
  }

  return result;
}

} // namespace

QPreferenceStoreDelegate::QPreferenceStoreDelegate(
  QString preferenceFilePath, std::chrono::milliseconds saveDelay)
  : m_preferenceFilePath{std::move(preferenceFilePath)}
  , m_saveTimer{new QTimer{this}}
  , m_fileSystemWatcher{new QFileSystemWatcher{this}}
{
  m_saveTimer->setSingleShot(true);
  m_saveTimer->setInterval(saveDelay);

  connect(m_saveTimer, &QTimer::timeout, this, [&]() {
    qDebug() << "Saving preferences after timeout";
    saveChangesImmediately();
  });

  loadCache();

  if (m_fileSystemWatcher->addPath(m_preferenceFilePath))
  {
    connect(
      m_fileSystemWatcher,
      &QFileSystemWatcher::QFileSystemWatcher::fileChanged,
      this,
      [&]() {
        qDebug() << "Loading preferences after file change";
        loadCache();
      });
  }
}

QPreferenceStoreDelegate::~QPreferenceStoreDelegate()
{
  if (m_saveTimer->isActive())
  {
    m_saveTimer->stop();
    saveChangesImmediately();
  }
}

bool QPreferenceStoreDelegate::load(const std::filesystem::path& path, bool& value)
{
  if (const auto iValue = m_cache.find(path); iValue != m_cache.end())
  {
    const auto& jsonValue = iValue->second;
    if (jsonValue.isBool())
    {
      value = jsonValue.toBool();
      return true;
    }
  }

  return false;
}

bool QPreferenceStoreDelegate::load(const std::filesystem::path& path, int& value)
{
  if (const auto iValue = m_cache.find(path); iValue != m_cache.end())
  {
    const auto& jsonValue = iValue->second;
    if (jsonValue.isDouble())
    {
      value = static_cast<int>(jsonValue.toDouble());
      return true;
    }
  }

  return false;
}

bool QPreferenceStoreDelegate::load(const std::filesystem::path& path, float& value)
{
  if (const auto iValue = m_cache.find(path); iValue != m_cache.end())
  {
    const auto& jsonValue = iValue->second;
    if (jsonValue.isDouble())
    {
      value = static_cast<float>(jsonValue.toDouble());
      return true;
    }
  }

  return false;
}

bool QPreferenceStoreDelegate::load(const std::filesystem::path& path, std::string& value)
{
  if (const auto iValue = m_cache.find(path); iValue != m_cache.end())
  {
    const auto& jsonValue = iValue->second;
    if (jsonValue.isString())
    {
      value = jsonValue.toString().toStdString();
      return true;
    }
  }

  return false;
}

bool QPreferenceStoreDelegate::load(
  const std::filesystem::path& path, std::filesystem::path& value)
{
  if (const auto iValue = m_cache.find(path); iValue != m_cache.end())
  {
    const auto& jsonValue = iValue->second;
    if (jsonValue.isString())
    {
      value = pathFromQString(jsonValue.toString());
      return true;
    }
  }

  return false;
}

bool QPreferenceStoreDelegate::load(const std::filesystem::path& path, Color& value)
{
  auto str = std::string{};
  if (load(path, str))
  {
    return Color::parse(str) | kdl::transform([&](const auto& color) {
             value = color;
             return true;
           })
           | kdl::transform_error([](const auto&) { return false; }) | kdl::value();
  }

  return false;
}

bool QPreferenceStoreDelegate::load(
  const std::filesystem::path& path, QKeySequence& value)
{
  if (const auto iValue = m_cache.find(path); iValue != m_cache.end())
  {
    const auto& jsonValue = iValue->second;
    if (jsonValue.isString())
    {
      value = QKeySequence::fromString(jsonValue.toString(), QKeySequence::PortableText);
      return true;
    }
  }

  return false;
}

void QPreferenceStoreDelegate::save(const std::filesystem::path& path, const bool value)
{
  m_cache.emplace(path, QJsonValue{value});
  triggerSaveChanges();
}

void QPreferenceStoreDelegate::save(const std::filesystem::path& path, const float value)
{
  m_cache.emplace(path, QJsonValue{static_cast<double>(value)});
  triggerSaveChanges();
}

void QPreferenceStoreDelegate::save(const std::filesystem::path& path, const int value)
{
  m_cache.emplace(path, QJsonValue{value});
  triggerSaveChanges();
}

void QPreferenceStoreDelegate::save(
  const std::filesystem::path& path, const std::string& value)
{
  m_cache.emplace(path, QJsonValue{QString::fromStdString(value)});
  triggerSaveChanges();
}

void QPreferenceStoreDelegate::save(
  const std::filesystem::path& path, const std::filesystem::path& value)
{
  m_cache.emplace(path, QJsonValue{pathAsQString(value)});
  triggerSaveChanges();
}

void QPreferenceStoreDelegate::save(const std::filesystem::path& path, const Color& value)
{
  save(path, value.toString());
}

void QPreferenceStoreDelegate::save(
  const std::filesystem::path& path, const QKeySequence& value)
{
  m_cache.emplace(path, QJsonValue{value.toString(QKeySequence::PortableText)});
  triggerSaveChanges();
}

void QPreferenceStoreDelegate::loadCache()
{
  if (m_fileReadWriteDisabled)
  {
    return;
  }

  const auto previousCache = m_cache;

  // Reload m_cache
  readPreferencesFromFile(m_preferenceFilePath)
    | kdl::transform([&](auto valueCache) { m_cache = std::move(valueCache); })
    | kdl::transform_error(kdl::overload(
      [&](const PreferenceErrors::FileAccessError&) {
        // This happens e.g. if you don't have read permissions for
        // m_preferencesFilePath
        showErrorAndDisableFileReadWrite(
          tr("A file IO error occurred while attempting to read the preference file:"),
          tr("ensure the file is readable"));
      },
      [&](const PreferenceErrors::LockFileError&) {
        // This happens if the lock file couldn't be acquired
        showErrorAndDisableFileReadWrite(
          tr("Could not acquire lock file for reading the preference file:"),
          tr("check for stale lock files"));
      },
      [&](const PreferenceErrors::JsonParseError&) {
        showErrorAndDisableFileReadWrite(
          tr("A JSON parsing error occurred while reading the preference file:"),
          tr("fix the JSON, or backup and delete the file"));
      },
      [&](const PreferenceErrors::NoFilePresent&) { m_cache = {}; }));

  const auto changedPreferencePaths = changedKeysForMapDiff(previousCache, m_cache);
  preferencesWereReloadedNotifier(changedPreferencePaths);
}

void QPreferenceStoreDelegate::triggerSaveChanges()
{
  if (m_saveTimer->isActive())
  {
    m_saveTimer->stop();
  }

  if (!m_fileReadWriteDisabled)
  {
    m_saveTimer->start();
  }
}

void QPreferenceStoreDelegate::saveChangesImmediately()
{
  writePreferencesToFile(m_preferenceFilePath, m_cache)
    | kdl::transform_error(kdl::overload(
      [&](const PreferenceErrors::FileAccessError&) {
        // This happens e.g. if you don't have read permissions for
        // m_preferencesFilePath
        showErrorAndDisableFileReadWrite(
          tr("A file IO error occurred while attempting to write the preference file:"),
          tr("ensure the file is writable"));
      },
      [&](const PreferenceErrors::LockFileError&) {
        // This happens if the lock file couldn't be acquired
        showErrorAndDisableFileReadWrite(
          tr("Could not acquire lock file for reading the preference file:"),
          tr("check for stale lock files"));
      }));
}

void QPreferenceStoreDelegate::showErrorAndDisableFileReadWrite(
  const QString& reason, const QString& suggestion)
{
  m_fileReadWriteDisabled = true;

  const auto message =
    tr(
      "%1\n\n"
      "%2\n\nPlease correct the problem (%3) and restart TrenchBroom.\n"
      "Further settings changes will not be saved this session.")
      .arg(reason)
      .arg(m_preferenceFilePath)
      .arg(suggestion);

  QTimer::singleShot(0, [=] {
    auto dialog = QMessageBox(
      QMessageBox::Icon::Critical, tr("TrenchBroom"), message, QMessageBox::Ok);
    dialog.exec();
  });
}

} // namespace tb::ui
