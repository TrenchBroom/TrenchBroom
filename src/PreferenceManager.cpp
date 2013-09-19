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
            it->first->save(wxConfig::Get());
            changedPreferences.insert(it->first);
            delete it->second;
        }
        
        m_unsavedPreferences.clear();
        return changedPreferences;
    }
    
    PreferenceBase::Set PreferenceManager::discardChanges() {
        PreferenceBase::Set changedPreferences;
        UnsavedPreferences::iterator it, end;
        for (it = m_unsavedPreferences.begin(), end = m_unsavedPreferences.end(); it != end; ++it) {
            it->first->setValue(it->second);
            changedPreferences.insert(it->first);
            delete it->second;
        }
        
        m_unsavedPreferences.clear();
        return changedPreferences;
    }
    
    bool PreferenceManager::getBool(Preference<bool>& preference) const {
        if (!preference.initialized())
            preference.load(wxConfig::Get());
        
        return preference.value();
    }
    
    void PreferenceManager::setBool(Preference<bool>& preference, bool value) {
        bool previousValue = preference.value();
        preference.setValue(value);
        if (m_saveInstantly)
            preference.save(wxConfig::Get());
        else
            markAsUnsaved(&preference, new ValueHolder<bool>(previousValue));
    }
    
    int PreferenceManager::getInt(Preference<int>& preference) const {
        if (!preference.initialized())
            preference.load(wxConfig::Get());
        
        return preference.value();
    }
    
    void PreferenceManager::setInt(Preference<int>& preference, int value) {
        int previousValue = preference.value();
        preference.setValue(value);
        if (m_saveInstantly)
            preference.save(wxConfig::Get());
        else
            markAsUnsaved(&preference, new ValueHolder<int>(previousValue));
    }
    
    float PreferenceManager::getFloat(Preference<float>& preference) const {
        if (!preference.initialized())
            preference.load(wxConfig::Get());
        
        return preference.value();
    }
    
    void PreferenceManager::setFloat(Preference<float>& preference, float value) {
        float previousValue = preference.value();
        preference.setValue(value);
        if (m_saveInstantly)
            preference.save(wxConfig::Get());
        else
            markAsUnsaved(&preference, new ValueHolder<float>(previousValue));
    }
    
    const String& PreferenceManager::getString(Preference<String>& preference) const {
        if (!preference.initialized())
            preference.load(wxConfig::Get());
        
        return preference.value();
    }
    
    void PreferenceManager::setString(Preference<String>& preference, const String& value) {
        String previousValue = preference.value();
        preference.setValue(value);
        if (m_saveInstantly)
            preference.save(wxConfig::Get());
        else
            markAsUnsaved(&preference, new ValueHolder<String>(previousValue));
    }
    
    const Color& PreferenceManager::getColor(Preference<Color>& preference) const {
        if (!preference.initialized())
            preference.load(wxConfig::Get());
        
        return preference.value();
    }
    
    void PreferenceManager::setColor(Preference<Color>& preference, const Color& value) {
        Color previousValue = preference.value();
        preference.setValue(value);
        if (m_saveInstantly)
            preference.save(wxConfig::Get());
        else
            markAsUnsaved(&preference, new ValueHolder<Color>(previousValue));
    }
    
    const View::KeyboardShortcut& PreferenceManager::getKeyboardShortcut(Preference<View::KeyboardShortcut>& preference) const {
        if (!preference.initialized())
            preference.load(wxConfig::Get());
        
        return preference.value();
    }
    
    void PreferenceManager::setKeyboardShortcut(Preference<View::KeyboardShortcut>& preference, const View::KeyboardShortcut& value) {
        View::KeyboardShortcut previousValue = preference.value();
        preference.setValue(value);
        if (m_saveInstantly)
            preference.save(wxConfig::Get());
        else
            markAsUnsaved(&preference, new ValueHolder<View::KeyboardShortcut>(previousValue));
    }

    PreferenceManager::PreferenceManager() {
#if defined __APPLE__
        m_saveInstantly = true;
#else
        m_saveInstantly = false;
#endif
    }
}
