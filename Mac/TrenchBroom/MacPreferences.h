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

#import <Foundation/Foundation.h>
#import "Model/Preferences.h"

namespace TrenchBroom {
    namespace Model {
        class MacPreferences : public Preferences {
        private:
            static void setDictionaryValue(NSMutableDictionary* dict, const std::string& key, int value);
            static void setDictionaryValue(NSMutableDictionary* dict, const std::string& key, float value);
            static void setDictionaryValue(NSMutableDictionary* dict, const std::string& key, bool value);
            static void setDictionaryValue(NSMutableDictionary* dict, const std::string& key, const std::string& value);
            static void setDictionaryValue(NSMutableDictionary* dict, const std::string& key, const Vec4f& value);
        protected:
            void loadDefaults();
            void loadPlatformDefaults();
            void loadPreferences();
            
            bool loadInt(const std::string& key, int& value);
            bool loadFloat(const std::string& key, float& value);
            bool loadBool(const std::string& key, bool& value);
            bool loadString(const std::string& key, std::string& value);
            bool loadVec4f(const std::string& key, Vec4f& value);

            void saveInt(const std::string& key, int value);
            void saveFloat(const std::string& key, float value);
            void saveBool(const std::string& key, bool value);
            void saveString(const std::string& key, const std::string& value);
            void saveVec4f(const std::string& key, const Vec4f& value);
            bool saveInstantly();
        };
    }
}