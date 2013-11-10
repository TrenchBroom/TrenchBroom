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

#ifndef __TrenchBroom__GameConfigParser__
#define __TrenchBroom__GameConfigParser__

#include "StringUtils.h"
#include "IO/ConfigParser.h"
#include "Model/GameConfig.h"

#include <iostream>

namespace TrenchBroom {
    namespace IO {
        class GameConfigParser {
        private:
            ConfigParser m_parser;
        public:
            GameConfigParser(const char* begin, const char* end);
            GameConfigParser(const String& str);
            
            Model::GameConfig parse();
        private:
            Model::GameConfig::TextureFormat parseTextureFormat(const ConfigTable& table) const;
            StringSet parseModelFormats(const ConfigList& list) const;
            
            void expectEntry(const ConfigEntry::Type typeMask, const ConfigEntry& entry) const;
            void expectTableEntry(const String& key, const ConfigEntry::Type typeMask, const ConfigTable& table) const;
            String typeNames(const ConfigEntry::Type typeMask) const;
        };
    }
}

#endif /* defined(__TrenchBroom__GameConfigParser__) */
