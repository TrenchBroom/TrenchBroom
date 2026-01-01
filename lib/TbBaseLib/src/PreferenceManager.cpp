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

#include "PreferenceManager.h"

#include "kd/contracts.h"

namespace tb
{
namespace
{

void dropPersistentValues(auto& values)
{
  auto iValue = values.begin();
  while (iValue != values.end())
  {
    const auto& [preference, value] = *iValue;
    if (preference->persistencePolicy == PreferencePersistencePolicy::Persistent)
    {
      iValue = values.erase(iValue);
    }
    else
    {
      iValue++;
    }
  }
}

} // namespace

std::unique_ptr<PreferenceManager> PreferenceManager::m_instance;

void PreferenceManager::destroyInstance()
{
  m_instance.reset();
}

PreferenceManager& PreferenceManager::instance()
{
  contract_assert(m_instance != nullptr);

  return *m_instance;
}

PreferenceManager::PreferenceManager(
  std::unique_ptr<PreferenceStore> preferenceStore, const bool saveInstantly)
  : m_preferenceStore{std::move(preferenceStore)}
  , m_saveInstantly{saveInstantly}
{
  contract_assert(m_preferenceStore != nullptr);

  m_notifierConnection += m_preferenceStore->preferencesWereReloadedNotifier.connect(
    [&](const std::vector<std::filesystem::path>& changedPreferencePaths) {
      refreshPersistentValues();

      for (const auto& path : changedPreferencePaths)
      {
        preferenceDidChangeNotifier(path);
      }
    });
}

PreferenceManager::~PreferenceManager() = default;

bool PreferenceManager::shouldSaveInstantly()
{
//#ifdef __APPLE__
  return true;
//#else
//  return false;
//#endif
}

bool PreferenceManager::saveInstantly() const
{
  return m_saveInstantly;
}

void PreferenceManager::saveChanges()
{
  for (const auto& [preference, pendingState] : m_pendingValues)
  {
    const auto& [pendingValue, setValue] = pendingState;
    setValue(pendingValue);
  }
  m_pendingValues.clear();
}

void PreferenceManager::discardChanges()
{
  for (const auto& [preference, _] : m_pendingValues)
  {
    m_values.erase(preference);
  }

  m_pendingValues.clear();
}

void PreferenceManager::refreshPersistentValues()
{
  dropPersistentValues(m_values);
  dropPersistentValues(m_pendingValues);
}

void togglePref(Preference<bool>& preference)
{
  auto& prefs = PreferenceManager::instance();
  prefs.set(preference, !prefs.getPendingValue(preference));
  prefs.saveChanges();
}

} // namespace tb
