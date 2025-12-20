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

#include "Notifier.h"
#include "Preference.h"
#include "PreferenceStore.h"

#include <any>
#include <functional>
#include <memory>
#include <unordered_map>

namespace tb
{

class PreferenceManager
{
private:
  static std::unique_ptr<PreferenceManager> m_instance;

public:
  template <typename... Args>
  static void createInstance(Args... args)
  {
    m_instance = std::make_unique<PreferenceManager>(std::forward<Args>(args)...);
  }

  static void destroyInstance();

  static PreferenceManager& instance();

private:
  using SaveFunc = std::function<void(const std::any&)>;

  struct PendingState
  {
    std::any value;
    SaveFunc saveFunc;
  };

  std::unique_ptr<PreferenceStore> m_preferenceStore;
  bool m_saveInstantly;

  std::unordered_map<const PreferenceBase*, std::any> m_values;
  std::unordered_map<const PreferenceBase*, PendingState> m_pendingValues;

  NotifierConnection m_notifierConnection;

public:
  Notifier<const std::filesystem::path&> preferenceDidChangeNotifier;

  explicit PreferenceManager(
    std::unique_ptr<PreferenceStore> preferenceStore,
    bool saveInstantly = shouldSaveInstantly());
  ~PreferenceManager();

  template <typename T>
  const T& get(const Preference<T>& preference)
  {
    if (const auto iValue = m_values.find(&preference); iValue != m_values.end())
    {
      return *std::any_cast<T>(&iValue->second);
    }

    auto value = T{};
    if (m_preferenceStore->load(preference.path, value))
    {
      return *std::any_cast<T>(&m_values.emplace(&preference, value).first->second);
    }

    return *std::any_cast<T>(
      &m_values.emplace(&preference, preference.defaultValue).first->second);
  }

  template <typename T>
  const T& getPendingValue(const Preference<T>& preference)
  {
    if (const auto iPendingValue = m_pendingValues.find(&preference);
        iPendingValue != m_pendingValues.end())
    {
      return *std::any_cast<T>(&iPendingValue->second.value);
    }

    return get(preference);
  }

  template <typename T, typename U>
    requires(std::is_convertible_v<U, T>)
  void set(const Preference<T>& preference, U&& value)
  {
    contract_assert(
      preference.persistencePolicy != PreferencePersistencePolicy::ReadOnly);

    auto tValue = T{std::forward<U>(value)};

    if (m_saveInstantly)
    {
      setValueInstantly(preference, std::move(tValue));
    }
    else
    {
      setPendingValue(preference, std::move(tValue));
    }
  }

  template <typename T>
  void resetToDefault(Preference<T>& preference)
  {
    set(preference, preference.defaultValue);
  }

  /**
   * Whether preferences should be saved instantly on the current platform.
   */
  static bool shouldSaveInstantly();

  bool saveInstantly() const;

  void saveChanges();

  void discardChanges();

private:
  template <typename T>
  void setValueInstantly(const Preference<T>& preference, T value)
  {
    if (get(preference) != value)
    {
      if (preference.persistencePolicy == PreferencePersistencePolicy::Persistent)
      {
        m_preferenceStore->save(preference.path, value);
      }

      m_values[&preference] = std::move(value);
      preferenceDidChangeNotifier(preference.path);
    }
  }

  template <typename T>
  void setPendingValue(const Preference<T>& preference, T value)
  {
    m_pendingValues[&preference] = PendingState{
      std::any{std::move(value)},
      [&](const auto& anyValue) {
        setValueInstantly(preference, std::any_cast<T>(anyValue));
      },
    };
  }

  void refreshPersistentValues();
};

template <typename T>
const T& pref(const Preference<T>& preference)
{
  auto& prefs = PreferenceManager::instance();
  return prefs.get(preference);
}

template <typename T, typename U>
void setPref(Preference<T>& preference, U&& value)
{
  auto& prefs = PreferenceManager::instance();
  prefs.set(preference, std::forward<U>(value));
  prefs.saveChanges();
}

void togglePref(Preference<bool>& preference);

} // namespace tb