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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__PreferenceManager__
#define __TrenchBroom__PreferenceManager__

#include "Color.h"
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
        
        PreferenceManager();
        
        void markAsUnsaved(PreferenceBase* preference, ValueHolderBase* valueHolder);
    public:
        inline static PreferenceManager& instance() {
            static PreferenceManager prefs;
            return prefs;
        }
        
        PreferenceBase::Set saveChanges();
        PreferenceBase::Set discardChanges();
        
        bool getBool(Preference<bool>& preference) const;
        void setBool(Preference<bool>& preference, bool value);
        
        int getInt(Preference<int>& preference) const;
        void setInt(Preference<int>& preference, int value);
        
        float getFloat(Preference<float>& preference) const;
        void setFloat(Preference<float>& preference, float value);
        
        const String& getString(Preference<String>& preference) const;
        void setString(Preference<String>& preference, const String& value);
        
        const Color& getColor(Preference<Color>& preference) const;
        void setColor(Preference<Color>& preference, const Color& value);
        
        const View::KeyboardShortcut& getKeyboardShortcut(Preference<View::KeyboardShortcut>& preference) const;
        void setKeyboardShortcut(Preference<View::KeyboardShortcut>& preference, const View::KeyboardShortcut& value);
    };
}

#endif /* defined(__TrenchBroom__PreferenceManager__) */
