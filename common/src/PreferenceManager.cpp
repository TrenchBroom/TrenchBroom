/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

namespace TrenchBroom {
    void PreferenceManager::markAsUnsaved(PreferenceBase* preference, ValueHolderBase* valueHolder) {
        UnsavedPreferences::iterator it = m_unsavedPreferences.find(preference);
        if (it == std::end(m_unsavedPreferences))
            m_unsavedPreferences[preference] = valueHolder;
        else
            delete valueHolder;
    }
    
    PreferenceManager& PreferenceManager::instance() {
        static PreferenceManager prefs;
        return prefs;
    }

    bool PreferenceManager::saveInstantly() const {
        return m_saveInstantly;
    }

    PreferenceBase::Set PreferenceManager::saveChanges() {
        PreferenceBase::Set changedPreferences;
        for (const auto& entry : m_unsavedPreferences) {
            PreferenceBase* pref = entry.first;
            ValueHolderBase* value = entry.second;
            
            pref->save(wxConfig::Get());
            preferenceDidChangeNotifier(pref->path());
            
            changedPreferences.insert(pref);
            delete value;
        }
        
        m_unsavedPreferences.clear();
        return changedPreferences;
    }
    
    PreferenceBase::Set PreferenceManager::discardChanges() {
        PreferenceBase::Set changedPreferences;
        for (const auto& entry : m_unsavedPreferences) {
            PreferenceBase* pref = entry.first;
            ValueHolderBase* value = entry.second;

            pref->setValue(value);
            changedPreferences.insert(pref);
            delete value;
        }
        
        m_unsavedPreferences.clear();
        return changedPreferences;
    }
    
    PreferenceManager::PreferenceManager() {
#if defined __APPLE__
        m_saveInstantly = true;
#else
        m_saveInstantly = false;
#endif
    }

    PreferenceManager::~PreferenceManager() {
        MapUtils::clearAndDelete(m_unsavedPreferences);
    }
}
