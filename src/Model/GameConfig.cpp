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

#include "GameConfig.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        GameConfig::TextureFormat::TextureFormat(const Type i_type, const IO::Path& i_palette) :
        type(i_type),
        palette(i_palette) {
            assert(type != TUnknown);
        }

        GameConfig::GameConfig(const String& name, const TextureFormat& textureFormat, const StringSet& modelFormats) :
        m_name(name),
        m_textureFormat(textureFormat),
        m_modelFormats(modelFormats) {
            assert(!StringUtils::trim(m_name).empty());
            assert(!m_modelFormats.empty());
        }
        
        const String& GameConfig::name() const {
            return m_name;
        }
        
        const GameConfig::TextureFormat& GameConfig::textureFormat() const {
            return m_textureFormat;
        }
        
        const StringSet& GameConfig::modelFormats() const {
            return m_modelFormats;
        }

        GameConfig::TextureFormat::Type GameConfig::parseType(const String str) {
            if (StringUtils::caseSensitiveEqual(str, "wad"))
                return TextureFormat::TWad;
            if (StringUtils::caseSensitiveEqual(str, "wal"))
                return TextureFormat::TWal;
            return TextureFormat::TUnknown;
        }
    }
}
