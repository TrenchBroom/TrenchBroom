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

#ifndef __TrenchBroom__QuakeGame__
#define __TrenchBroom__QuakeGame__

#include "Color.h"
#include "Model/Game.h"
#include "Assets/AssetTypes.h"
#include "Assets/Palette.h"
#include "Model/ModelTypes.h"
#include "VecMath.h"

namespace TrenchBroom {
    namespace View {
        class Logger;
    }
    
    namespace Model {
        class QuakeGame : public Game {
        private:
            Color m_defaultEntityColor;
            View::Logger* m_logger;
            Assets::Palette m_palette;
        public:
            static GamePtr newGame(const Color& defaultEntityColor, View::Logger* logger = NULL);
        private:
            QuakeGame(const Color& defaultEntityColor, View::Logger* logger);
            static const BBox3 WorldBounds;
            static IO::Path palettePath();
            
            Map* doLoadMap(const BBox3& worldBounds, const IO::Path& path) const;
            IO::Path::List doExtractTexturePaths(const Map* map) const;
            Assets::TextureCollection* doLoadTextureCollection(const IO::Path& path) const;
            void doUploadTextureCollection(Assets::TextureCollection* collection) const;
            Assets::EntityDefinitionList doLoadEntityDefinitions(const IO::Path& path) const;
            IO::Path doDefaultEntityDefinitionFile() const;
            IO::Path doExtractEntityDefinitionFile(const Map* map) const;
        };
    }
}

#endif /* defined(__TrenchBroom__QuakeGame__) */
