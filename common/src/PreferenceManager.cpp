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

#include "PreferenceManager.h"

#include "IO/PathQt.h"
#include "IO/SystemPaths.h"
#include "Preferences.h"
#include "View/Actions.h"

#include <QDebug>
#include <QDir>
#include <QFileSystemWatcher>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLockFile>
#include <QMessageBox>
#include <QSaveFile>
#if defined(Q_OS_WIN)
#include <QSettings>
#endif
#include <QStandardPaths>
#include <QStringBuilder>
#include <QTimer>

#include <string>
#include <vector>

namespace TrenchBroom
{
// PreferenceManager

std::unique_ptr<PreferenceManager> PreferenceManager::m_instance;
bool PreferenceManager::m_initialized = false;

PreferenceManager& PreferenceManager::instance()
{
  ensure(m_instance != nullptr, "Preference manager is set");
  if (!m_initialized)
  {
    m_instance->initialize();
    m_initialized = true;
  }
  return *m_instance;
}

namespace
{
bool shouldSaveInstantly()
{
#if defined __APPLE__
  return true;
#else
  return false;
#endif
}
} // namespace

AppPreferenceManager::AppPreferenceManager()
  : m_saveInstantly{shouldSaveInstantly()}
  , m_fileSystemWatcher{nullptr}
  , m_fileReadWriteDisabled{false}
{
  m_saveTimer.setSingleShot(true);
  connect(&m_saveTimer, &QTimer::timeout, this, [this] {
    qDebug() << "Saving preferences";
    saveChangesImmediately();
  });
}

AppPreferenceManager::~AppPreferenceManager()
{
  if (m_saveTimer.isActive())
  {
    m_saveTimer.stop();
    saveChangesImmediately();
  }
}

void AppPreferenceManager::initialize()
{
  m_preferencesFilePath = preferenceFilePath();

  loadCacheFromDisk();

  m_fileSystemWatcher = new QFileSystemWatcher{this};
  if (m_fileSystemWatcher->addPath(m_preferencesFilePath))
  {
    connect(
      m_fileSystemWatcher,
      &QFileSystemWatcher::QFileSystemWatcher::fileChanged,
      this,
      [this]() {
        qDebug() << "Reloading preferences after file change";
        loadCacheFromDisk();
      });
  }
}

bool AppPreferenceManager::saveInstantly() const
{
  return m_saveInstantly;
}

void AppPreferenceManager::saveChanges()
{
  if (m_unsavedPreferences.empty())
  {
    return;
  }

  for (auto* pref : m_unsavedPreferences)
  {
    savePreferenceToCache(*pref);
    preferenceDidChangeNotifier(pref->path());
  }
  m_unsavedPreferences.clear();

  if (!m_fileReadWriteDisabled)
  {
    m_saveTimer.start(500);
  }
}

void AppPreferenceManager::discardChanges()
{
  if (m_saveTimer.isActive())
  {
    m_saveTimer.stop();
    saveChangesImmediately();
  }
  m_unsavedPreferences.clear();
  invalidatePreferences();
}

void AppPreferenceManager::saveChangesImmediately()
{
  writePreferencesToFile(m_preferencesFilePath, m_cache)
    .transform_error(kdl::overload(
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

void AppPreferenceManager::markAsUnsaved(PreferenceBase& preference)
{
  m_unsavedPreferences.insert(&preference);
}

void AppPreferenceManager::showErrorAndDisableFileReadWrite(
  const QString& reason, const QString& suggestion)
{
  m_fileReadWriteDisabled = true;

  const auto message =
    tr(
      "%1\n\n"
      "%2\n\nPlease correct the problem (%3) and restart TrenchBroom.\n"
      "Further settings changes will not be saved this session.")
      .arg(reason)
      .arg(m_preferencesFilePath)
      .arg(suggestion);

  QTimer::singleShot(0, [=] {
    auto dialog = QMessageBox(
      QMessageBox::Icon::Critical, tr("TrenchBroom"), message, QMessageBox::Ok);
    dialog.exec();
  });
}

namespace
{
std::vector<IO::Path> changedKeysForMapDiff(
  const std::map<IO::Path, QJsonValue>& before,
  const std::map<IO::Path, QJsonValue>& after)
{
  auto result = kdl::vector_set<IO::Path>{};

  // removes
  for (auto& [k, v] : before)
  {
    unused(v);
    if (after.find(k) == after.end())
    {
      // removal
      result.insert(k);
    }
  }

  // adds/updates
  for (auto& [k, v] : after)
  {
    auto beforeIt = before.find(k);
    if (beforeIt == before.end())
    {
      // add
      result.insert(k);
    }
    else
    {
      // check for update?
      if (beforeIt->second != v)
      {
        result.insert(k);
      }
    }
  }

  return result.release_data();
}
} // namespace

/**
 * Reloads m_cache from the .json file,
 * marks all Preference<T> objects as needing deserialization next time they're
 * accessed, and emits preferenceDidChangeNotifier as needed.
 */
void AppPreferenceManager::loadCacheFromDisk()
{
  if (m_fileReadWriteDisabled)
  {
    return;
  }

  const auto oldPrefs = m_cache;

  // Reload m_cache
  readPreferencesFromFile(m_preferencesFilePath)
    .transform(
      [&](std::map<IO::Path, QJsonValue>&& prefs) { m_cache = std::move(prefs); })
    .transform_error(kdl::overload(
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

  invalidatePreferences();

  // Emit preferenceDidChangeNotifier for any changed preferences
  const auto changedKeys = changedKeysForMapDiff(oldPrefs, m_cache);
  for (const auto& changedPath : changedKeys)
  {
    preferenceDidChangeNotifier(changedPath);
  }
}

void AppPreferenceManager::invalidatePreferences()
{
  // Force all currently known Preference<T> objects to deserialize from m_cache next
  // time they are accessed Note, because new Preference<T> objects can be created at
  // runtime, we need this sort of lazy loading system.
  for (auto* pref : Preferences::staticPreferences())
  {
    pref->setValid(false);
  }
  for (auto& [path, prefPtr] : m_dynamicPreferences)
  {
    unused(path);
    prefPtr->setValid(false);
  }
}

/**
 * Updates the given PreferenceBase from m_cache.
 */
void AppPreferenceManager::loadPreferenceFromCache(PreferenceBase& pref)
{
  const auto format = PreferenceSerializer{};

  auto it = m_cache.find(pref.path());
  if (it == m_cache.end())
  {
    // no value set, use the default value
    pref.resetToDefault();
    pref.setValid(true);
    return;
  }

  const auto jsonValue = it->second;
  if (!pref.loadFromJson(format, jsonValue))
  {
    // FIXME: Log to TB console
    const auto variantValue = jsonValue.toVariant();
    qDebug() << "Failed to load preference " << IO::pathAsGenericQString(pref.path())
             << " from JSON value: " << variantValue.toString() << " ("
             << variantValue.typeName() << ")";

    pref.resetToDefault();

    // Replace the invalid value in the cache with the default
    savePreferenceToCache(pref);

    // FIXME: trigger writing to disk
  }
  pref.setValid(true);
}

void AppPreferenceManager::savePreferenceToCache(PreferenceBase& pref)
{
  if (pref.isDefault())
  {
    // Just remove the key/value from the cache if it's already at the default value
    auto it = m_cache.find(pref.path());
    if (it != m_cache.end())
    {
      m_cache.erase(it);
    }
    return;
  }

  const auto format = PreferenceSerializer{};
  m_cache[pref.path()] = pref.writeToJson(format);
}

void AppPreferenceManager::validatePreference(PreferenceBase& preference)
{
  ensure(
    qApp->thread() == QThread::currentThread(),
    "PreferenceManager can only be used on the main thread");

  if (!preference.valid())
  {
    loadPreferenceFromCache(preference);
  }
}

void AppPreferenceManager::savePreference(PreferenceBase& preference)
{
  ensure(
    qApp->thread() == QThread::currentThread(),
    "PreferenceManager can only be used on the main thread");

  markAsUnsaved(preference);

  if (saveInstantly())
  {
    saveChanges();
  }
}

// helpers

void togglePref(Preference<bool>& preference)
{
  auto& prefs = PreferenceManager::instance();
  prefs.set(preference, !prefs.get(preference));
  prefs.saveChanges();
}

QString preferenceFilePath()
{
  return IO::pathAsQString(
    IO::SystemPaths::userDataDirectory() / IO::Path{"Preferences.json"});
}

namespace
{
QLockFile getLockFile(const QString& preferenceFilePath)
{
  const auto lockFilePath = preferenceFilePath + ".lck";
  return QLockFile{lockFilePath};
}
} // namespace

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
  const QString& path, const std::map<IO::Path, QJsonValue>& prefs)
{
  const auto serialized = writePreferencesToJson(prefs);

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

  const auto written = saveFile.write(serialized);
  if (written != static_cast<qint64>(serialized.size()))
  {
    return PreferenceErrors::FileAccessError{};
  }

  if (!saveFile.commit())
  {
    return PreferenceErrors::FileAccessError{};
  }

  return kdl::void_success;
}

ReadPreferencesResult readPreferences()
{
  return readPreferencesFromFile(preferenceFilePath());
}

ReadPreferencesResult parsePreferencesFromJson(const QByteArray& jsonData)
{
  auto error = QJsonParseError{};
  const auto document = QJsonDocument::fromJson(jsonData, &error);

  if (error.error != QJsonParseError::NoError || !document.isObject())
  {
    return PreferenceErrors::JsonParseError{error};
  }

  const auto object = document.object();
  auto result = std::map<IO::Path, QJsonValue>{};
  for (auto it = object.constBegin(); it != object.constEnd(); ++it)
  {
    result[IO::pathFromQString(it.key())] = it.value();
  }
  return result;
}

QByteArray writePreferencesToJson(const std::map<IO::Path, QJsonValue>& prefs)
{
  auto rootObject = QJsonObject{};
  for (auto [key, val] : prefs)
  {
    rootObject[IO::pathAsGenericQString(key)] = val;
  }

  auto document = QJsonDocument{rootObject};
  return document.toJson(QJsonDocument::Indented);
}
} // namespace TrenchBroom
