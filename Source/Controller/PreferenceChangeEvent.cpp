/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PreferenceChangeEvent.h"

namespace TrenchBroom {
    namespace Controller {
        PreferenceChangeEvent::PreferenceChangeEvent() :
        Command(PreferenceChange),
        m_menuChanged(false) {}
        
        PreferenceChangeEvent::PreferenceChangeEvent(const Preferences::PreferenceBase& preference) :
        Command(PreferenceChange),
        m_menuChanged(false) {
            addPreference(preference);
        }
        
        PreferenceChangeEvent::PreferenceChangeEvent(const Preferences::PreferenceBase::Set& preferences) :
        Command(PreferenceChange),
        m_menuChanged(false) {
            addPreferences(preferences);
        }

        void PreferenceChangeEvent::addPreference(const Preferences::PreferenceBase& preference) {
            m_preferences.insert(&preference);
        }
        
        void PreferenceChangeEvent::addPreferences(const Preferences::PreferenceBase::Set& preferences) {
            m_preferences.insert(preferences.begin(), preferences.end());
        }

        bool PreferenceChangeEvent::isPreferenceChanged(const Preferences::PreferenceBase& preference) const {
            return m_preferences.count(&preference) > 0;
        }
        
        void PreferenceChangeEvent::setMenuChanged(const bool menuChanged) {
            m_menuChanged = menuChanged;
        }
        
        bool PreferenceChangeEvent::menuHasChanged() const {
            return m_menuChanged;
        }
    }
}
