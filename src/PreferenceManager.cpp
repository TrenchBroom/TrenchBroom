/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
        if (it == m_unsavedPreferences.end())
            m_unsavedPreferences[preference] = valueHolder;
        else
            delete valueHolder;
    }
    
    PreferenceManager& PreferenceManager::instance() {
        static PreferenceManager prefs;
        return prefs;
    }

    PreferenceBase::Set PreferenceManager::saveChanges() {
        PreferenceBase::Set changedPreferences;
        UnsavedPreferences::iterator it, end;
        for (it = m_unsavedPreferences.begin(), end = m_unsavedPreferences.end(); it != end; ++it) {
            PreferenceBase* pref = it->first;
            ValueHolderBase* value = it->second;
            
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
        UnsavedPreferences::iterator it, end;
        for (it = m_unsavedPreferences.begin(), end = m_unsavedPreferences.end(); it != end; ++it) {
            PreferenceBase* pref = it->first;
            ValueHolderBase* value = it->second;

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
        UnsavedPreferences::iterator it, end;
        for (it = m_unsavedPreferences.begin(), end = m_unsavedPreferences.end(); it != end; ++it)
            delete it->second;
    }
}
