/*
 Copyright (C) 2020 MaxED

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

#pragma once

#include "Logger.h"

#include "Assets/Texture.h"
#include "Assets/TextureCollection.h"

#include <kdl/string_format.h>

namespace TrenchBroom::Assets {
    class EntitySpriteManager {
    private:
        Logger& m_logger;

        int m_minFilter;
        int m_magFilter;
        bool m_resetTextureMode;

        TextureCollection m_spriteCollection;
    public:
        EntitySpriteManager(int magFilter, int minFilter, Logger& logger);
        ~EntitySpriteManager();

        void load();

        const Texture* sprite(const std::string& name) const;

        void setTextureMode(int minFilter, int magFilter);
        void commitChanges();
    };
}

