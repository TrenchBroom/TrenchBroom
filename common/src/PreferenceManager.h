/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef TrenchBroom_PreferenceManager
#define TrenchBroom_PreferenceManager

#include "Color.h"
#include "Notifier.h"
#include "Preference.h"
#include "StringUtils.h"
#include "View/KeyboardShortcut.h"

#include <map>

namespace TrenchBroom {
    namespace IO {
        class Path;
    }
    
    class PreferenceManager {
    private:
        typedef std::map<PreferenceBase*, ValueHolderBase*> UnsavedPreferences;
        
        bool m_saveInstantly;
        UnsavedPreferences m_unsavedPreferences;
        
        void markAsUnsaved(PreferenceBase* preference, ValueHolderBase* valueHolder);
    public:
        ~PreferenceManager();
        static PreferenceManager& instance();
        
        Notifier1<const IO::Path&> preferenceDidChangeNotifier;

        bool saveInstantly() const;
        PreferenceBase::Set saveChanges();
        PreferenceBase::Set discardChanges();
        
        template <typename T>
        const T& get(const Preference<T>& preference) const {
            if (!preference.initialized())
                preference.load(wxConfig::Get());
            
            return preference.value();
        }
        
        template <typename T>
        const T& getDefault(const Preference<T>& preference) const {
            return preference.defaultValue();
        }
        
        template <typename T>
        void set(Preference<T>& preference, const T& value) {
            const T previousValue = preference.value();
            preference.setValue(value);
            if (saveInstantly()) {
                preference.save(wxConfig::Get());
                preferenceDidChangeNotifier(preference.path());
            } else {
                markAsUnsaved(&preference, new ValueHolder<T>(previousValue));
            }
        }
        
        template <typename T>
        void resetToDefault(Preference<T>& preference) {
            set(preference, preference.defaultValue());
        }
    private:
        PreferenceManager();
        PreferenceManager(const PreferenceManager&);
        PreferenceManager& operator=(const PreferenceManager&);
    };
    
    template <typename T>
    const T& pref(const Preference<T>& preference) {
        const PreferenceManager& prefs = PreferenceManager::instance();
        return prefs.get(preference);
    }
    
    template <typename T>
    class SetTemporaryPreference {
    private:
        Preference<T>& m_pref;
        T m_oldValue;
    public:
        SetTemporaryPreference(Preference<T>& pref, const T& newValue) :
        m_pref(pref),
        m_oldValue(pref.value()) {
            m_pref.setValue(newValue);
        }
        
        ~SetTemporaryPreference() {
            m_pref.setValue(m_oldValue);
        }
    };
}

#endif /* defined(TrenchBroom_PreferenceManager) */
