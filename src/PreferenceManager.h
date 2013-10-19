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

#ifndef __TrenchBroom__PreferenceManager__
#define __TrenchBroom__PreferenceManager__

#include "Color.h"
#include "Notifier.h"
#include "Preference.h"
#include "StringUtils.h"
#include "View/KeyboardShortcut.h"

#include <map>

namespace TrenchBroom {
    class PreferenceManager {
    private:
        typedef std::map<PreferenceBase*, ValueHolderBase*> UnsavedPreferences;
        
        bool m_saveInstantly;
        UnsavedPreferences m_unsavedPreferences;
        
        void markAsUnsaved(PreferenceBase* preference, ValueHolderBase* valueHolder);
    public:
        static PreferenceManager& instance();
        
        Notifier1<const String&> preferenceDidChangeNotifier;

        PreferenceBase::Set saveChanges();
        PreferenceBase::Set discardChanges();
        
        template <typename T>
        const T& get(Preference<T>& preference) const {
            if (!preference.initialized())
                preference.load(wxConfig::Get());
            
            return preference.value();
        }
        
        template <typename T>
        void set(Preference<T>& preference, const T& value) {
            const T previousValue = preference.value();
            preference.setValue(value);
            if (m_saveInstantly) {
                preference.save(wxConfig::Get());
                preferenceDidChangeNotifier(preference.name());
            } else {
                markAsUnsaved(&preference, new ValueHolder<T>(previousValue));
            }
        }
    private:
        PreferenceManager();
        ~PreferenceManager();
        
        PreferenceManager(const PreferenceManager& other);
        PreferenceManager& operator= (const PreferenceManager& other);
    };
    
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

#endif /* defined(__TrenchBroom__PreferenceManager__) */
