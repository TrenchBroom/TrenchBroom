/*
 Copyright (C) 2010 Kristian Duske

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

#include <QApplication>
#include <QByteArray>
#include <QJsonParseError>
#include <QString>
#include <QThread>
#include <QTimer>

#include "Contracts.h"
#include "Macros.h"
#include "Notifier.h"
#include "Preference.h"
#include "Result.h"

#include "kd/reflection_decl.h"
#include "kd/vector_set.h"

#include <fmt/format.h>
#include <fmt/std.h>

#include <filesystem>
#include <iosfwd>
#include <map>
#include <memory>
#include <vector>

class QTextStream;
class QFileSystemWatcher;

namespace tb
{

class PreferenceManager : public QObject
{
  Q_OBJECT
private:
  static std::unique_ptr<PreferenceManager> m_instance;
  static bool m_initialized;

protected:
  std::map<std::filesystem::path, std::unique_ptr<PreferenceBase>> m_dynamicPreferences;

public:
  Notifier<const std::filesystem::path&> preferenceDidChangeNotifier;

public:
  static PreferenceManager& instance();

  template <typename T>
  static void createInstance()
  {
    m_instance = std::make_unique<T>();
    m_initialized = false;
  }

  static void destroyInstance()
  {
    m_instance.reset();
    m_initialized = false;
  }

  template <typename T>
  Preference<T>& dynamicPreference(const std::filesystem::path& path, T&& defaultValue)
  {
    auto it = m_dynamicPreferences.find(path);
    if (it == std::end(m_dynamicPreferences))
    {
      bool success = false;
      std::tie(it, success) = m_dynamicPreferences.emplace(
        path, std::make_unique<Preference<T>>(path, std::forward<T>(defaultValue)));
      contract_assert(success);
    }

    const auto& prefPtr = it->second;
    auto* prefBase = prefPtr.get();
    auto* pref = dynamic_cast<Preference<T>*>(prefBase);
    contract_assert(pref != nullptr);
    return *pref;
  }

  /**
   * Public API for getting the value of a preference.
   */
  template <typename T>
  const T& get(Preference<T>& preference)
  {
    validatePreference(preference);
    return preference.value();
  }

  /**
   * Public API for setting the value of a preference.
   */
  template <typename T>
  bool set(Preference<T>& preference, const T& value)
  {
    const T previousValue = get(preference);
    if (previousValue == value)
    {
      return false;
    }

    preference.setValue(value);
    preference.setValid(true);

    savePreference(preference);
    if (saveInstantly())
    {
      preferenceDidChangeNotifier(preference.path());
    }

    return true;
  }

  template <typename T>
  void resetToDefault(Preference<T>& preference)
  {
    set(preference, preference.defaultValue());
  }

  virtual void initialize() = 0;

  virtual bool saveInstantly() const = 0;
  virtual void saveChanges() = 0;
  virtual void discardChanges() = 0;

private:
  virtual void validatePreference(PreferenceBase&) = 0;
  virtual void savePreference(PreferenceBase&) = 0;
};

class AppPreferenceManager : public PreferenceManager
{
  Q_OBJECT
private:
  using UnsavedPreferences = kdl::vector_set<PreferenceBase*>;
  using DynamicPreferences =
    std::map<std::filesystem::path, std::unique_ptr<PreferenceBase>>;

  std::filesystem::path m_preferencesFilePath;
  bool m_saveInstantly;
  UnsavedPreferences m_unsavedPreferences;
  QTimer m_saveTimer;

  /**
   * This should always be in sync with what is on disk.
   * Preference objects may have different values if there are unsaved changes.
   * There may also be values in here we don't know how to deserialize; we write them back
   * to disk.
   */
  std::map<std::filesystem::path, QJsonValue> m_cache;
  QFileSystemWatcher* m_fileSystemWatcher = nullptr;
  /**
   * If true, don't try to read/write preferences anymore.
   * This gets set to true if there is a JSON parse error, so
   * we don't clobber the file if the user makes a mistake while editing it by hand.
   */
  bool m_fileReadWriteDisabled = false;

public:
  AppPreferenceManager();
  ~AppPreferenceManager() override;

  void initialize() override;

  bool saveInstantly() const override;
  void saveChanges() override;
  void discardChanges() override;

private:
  void saveChangesImmediately();

  void markAsUnsaved(PreferenceBase& preference);
  void showErrorAndDisableFileReadWrite(const QString& reason, const QString& suggestion);
  void loadCacheFromDisk();
  void invalidatePreferences();
  void loadPreferenceFromCache(PreferenceBase& pref);
  void savePreferenceToCache(PreferenceBase& pref);

private:
  void validatePreference(PreferenceBase&) override;
  void savePreference(PreferenceBase&) override;

  deleteCopyAndMove(AppPreferenceManager);
};

template <typename T>
const T& pref(Preference<T>& preference)
{
  PreferenceManager& prefs = PreferenceManager::instance();
  return prefs.get(preference);
}

/**
 * Sets a preference, and saves the change immediately.
 */
template <typename T>
void setPref(Preference<T>& preference, const T& value)
{
  PreferenceManager& prefs = PreferenceManager::instance();
  prefs.set(preference, value);
  prefs.saveChanges();
}

/**
 * Temporarily sets a preference and resets it when destroyed.
 * Both changes are saved immediately.
 */
template <typename T>
class TemporarilySetPref
{
private:
  Preference<T>& m_preference;
  T m_originalValue;

public:
  TemporarilySetPref(Preference<T>& preference, const T& value)
    : m_preference{preference}
    , m_originalValue{pref(m_preference)}
  {
    setPref(m_preference, value);
  }

  ~TemporarilySetPref() { setPref(m_preference, m_originalValue); }
};

/**
 * Deduction guide.
 */
template <typename T>
TemporarilySetPref(Preference<T>& preference, const T& value) -> TemporarilySetPref<T>;

/**
 * Toggles a bool preference, and saves the change immediately.
 */
void togglePref(Preference<bool>& preference);

/**
 * Resets a preference to its default value, and saves the change immediately.
 */
template <typename T>
void resetPref(Preference<T>& preference)
{
  PreferenceManager& prefs = PreferenceManager::instance();
  prefs.resetToDefault(preference);
  prefs.saveChanges();
}

namespace PreferenceErrors
{

struct NoFilePresent
{
  kdl_reflect_decl_empty(NoFilePresent);
};

struct JsonParseError
{
  QJsonParseError jsonError;

  kdl_reflect_decl(JsonParseError, jsonError);
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

using ReadPreferencesResult = Result<
  std::map<std::filesystem::path, QJsonValue>, // Success case
  PreferenceErrors::NoFilePresent,
  PreferenceErrors::JsonParseError,
  PreferenceErrors::FileAccessError,
  PreferenceErrors::LockFileError>;

using WritePreferencesResult =
  Result<void, PreferenceErrors::FileAccessError, PreferenceErrors::LockFileError>;

std::filesystem::path preferenceFilePath();
ReadPreferencesResult readPreferencesFromFile(const std::filesystem::path& path);
ReadPreferencesResult readPreferences();

WritePreferencesResult writePreferencesToFile(
  const std::filesystem::path& path,
  const std::map<std::filesystem::path, QJsonValue>& prefs);
ReadPreferencesResult parsePreferencesFromJson(const QByteArray& jsonData);
QByteArray writePreferencesToJson(
  const std::map<std::filesystem::path, QJsonValue>& prefs);

} // namespace tb

std::ostream& operator<<(std::ostream& lhs, const QJsonParseError& rhs);
