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

#ifndef __TrenchBroom__Hexen2Game__
#define __TrenchBroom__Hexen2Game__


#include "Color.h"
#include "Model/Game.h"
#include "Assets/AssetTypes.h"
#include "Assets/Palette.h"
#include "IO/QuakeFS.h"
#include "VecMath.h"

namespace TrenchBroom {
    class Logger;
    
    namespace Model {
        class Hexen2Game : public Game {
        private:
            IO::QuakeFS m_fs;
            Color m_defaultEntityColor;
            Assets::Palette m_palette;
        public:
            static GamePtr newGame(const IO::Path& gamePath, const Color& defaultEntityColor, Logger* logger);
        private:
            Hexen2Game(const IO::Path& gamePath, const Color& defaultEntityColor, Logger* logger = NULL);
            
            static const BBox3 WorldBounds;
            static IO::Path palettePath();
            
            Map* doNewMap() const;
            Map* doLoadMap(const BBox3& worldBounds, const IO::Path& path) const;
            IO::Path::List doExtractTexturePaths(const Map* map) const;
            Assets::FaceTextureCollection* doLoadTextureCollection(const IO::Path& path) const;
            void doUploadTextureCollection(Assets::FaceTextureCollection* collection) const;
            
            Assets::EntityDefinitionList doLoadEntityDefinitions(const IO::Path& path) const;
            IO::Path doDefaultEntityDefinitionFile() const;
            IO::Path doExtractEntityDefinitionFile(const Map* map) const;
            
            Assets::EntityModel* doLoadModel(const IO::Path& path) const;
        };
    }
}

#endif /* defined(__TrenchBroom__Hexen2Game__) */
