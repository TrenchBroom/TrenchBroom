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
#include "View/KeyboardShortcut.h"

#include <QDebug>
#include <QDir>
#include <QFileSystemWatcher>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
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
// PreferenceSerializer

bool PreferenceSerializer::readFromJSON(const QJsonValue& in, bool& out) const
{
  if (!in.isBool())
  {
    return false;
  }
  out = in.toBool();
  return true;
}

bool PreferenceSerializer::readFromJSON(const QJsonValue& in, Color& out) const
{
  if (!in.isString())
  {
    return false;
  }

  if (const auto color = Color::parse(in.toString().toStdString()))
  {
    out = *color;
    return true;
  }

  return false;
}

bool PreferenceSerializer::readFromJSON(const QJsonValue& in, float& out) const
{
  if (!in.isDouble())
  {
    return false;
  }
  out = static_cast<float>(in.toDouble());
  return true;
}

bool PreferenceSerializer::readFromJSON(const QJsonValue& in, int& out) const
{
  if (!in.isDouble())
  {
    return false;
  }
  out = static_cast<int>(in.toDouble());
  return true;
}

bool PreferenceSerializer::readFromJSON(const QJsonValue& in, IO::Path& out) const
{
  if (!in.isString())
  {
    return false;
  }

  out = IO::pathFromQString(in.toString());
  return true;
}

bool PreferenceSerializer::readFromJSON(const QJsonValue& in, QKeySequence& out) const
{
  if (!in.isString())
  {
    return false;
  }
  out = QKeySequence{in.toString(), QKeySequence::PortableText};
  return true;
}

bool PreferenceSerializer::readFromJSON(const QJsonValue& in, QString& out) const
{
  if (!in.isString())
  {
    return false;
  }

  out = in.toString();
  return true;
}

QJsonValue PreferenceSerializer::writeToJSON(const bool in) const
{
  return {in};
}

namespace
{
template <typename T, typename L>
QJsonValue toJson(
  const T& in, const L& serialize = [](QTextStream& lhs, const T& rhs) { lhs << rhs; })
{
  // NOTE: QTextStream's default locale is C, unlike QString::arg()
  auto string = QString{};
  auto stream = QTextStream{&string};
  serialize(stream, in);
  return {string};
}

template <typename T>
QJsonValue toJson(const T& in)
{
  return toJson(in, [](QTextStream& lhs, const T& rhs) { lhs << rhs; });
}
} // namespace

QJsonValue PreferenceSerializer::writeToJSON(const Color& in) const
{
  return toJson(in, [](QTextStream& lhs, const Color& rhs) {
    lhs << rhs.r() << " " << rhs.g() << " " << rhs.b() << " " << rhs.a();
  });
}

QJsonValue PreferenceSerializer::writeToJSON(const float in) const
{
  return {static_cast<double>(in)};
}

QJsonValue PreferenceSerializer::writeToJSON(const int in) const
{
  return {in};
}

QJsonValue PreferenceSerializer::writeToJSON(const IO::Path& in) const
{
  return toJson(in, [](auto& lhs, const auto& rhs) { lhs << IO::pathAsQString(rhs); });
}

QJsonValue PreferenceSerializer::writeToJSON(const QKeySequence& in) const
{
  return {in.toString(QKeySequence::PortableText)};
}

QJsonValue PreferenceSerializer::writeToJSON(const QString& in) const
{
  return toJson(in);
}

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
  m_preferencesFilePath = v2SettingsPath();

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
  writeV2SettingsToPath(m_preferencesFilePath, m_cache)
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

static std::vector<IO::Path> changedKeysForMapDiff(
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
  readV2SettingsFromPath(m_preferencesFilePath)
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
  if (!pref.loadFromJSON(format, jsonValue))
  {
    // FIXME: Log to TB console
    const auto variantValue = jsonValue.toVariant();
    qDebug() << "Failed to load preference " << IO::pathAsQString(pref.path(), "/")
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
  m_cache[pref.path()] = pref.writeToJSON(format);
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

QString v2SettingsPath()
{
  return IO::pathAsQString(
    IO::SystemPaths::userDataDirectory() + IO::Path{"Preferences.json"});
}

static QLockFile getLockFile(const QString& settingsFilePath)
{
  const auto lockFilePath = settingsFilePath + ".lck";
  return QLockFile{lockFilePath};
}

ReadPreferencesResult readV2SettingsFromPath(const QString& path)
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

  return parseV2SettingsFromJSON(contents);
}

WritePreferencesResult writeV2SettingsToPath(
  const QString& path, const std::map<IO::Path, QJsonValue>& v2Prefs)
{
  const auto serialized = writeV2SettingsToJSON(v2Prefs);

  const auto settingsDir = QFileInfo{path}.path();
  if (!QDir().mkpath(settingsDir))
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

  const qint64 written = saveFile.write(serialized);
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

ReadPreferencesResult readV2Settings()
{
  return readV2SettingsFromPath(v2SettingsPath());
}

ReadPreferencesResult parseV2SettingsFromJSON(const QByteArray& jsonData)
{
  auto error = QJsonParseError();
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

QByteArray writeV2SettingsToJSON(const std::map<IO::Path, QJsonValue>& v2Prefs)
{
  auto rootObject = QJsonObject{};
  for (auto [key, val] : v2Prefs)
  {
    rootObject[IO::pathAsQString(key, "/")] = val;
  }

  auto document = QJsonDocument{rootObject};
  return document.toJson(QJsonDocument::Indented);
}
} // namespace TrenchBroom
