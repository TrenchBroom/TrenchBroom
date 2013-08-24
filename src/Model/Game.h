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

#ifndef TrenchBroom_Game_h
#define TrenchBroom_Game_h

#include "TrenchBroom.h"
#include "VecMath.h"
#include "SharedPointer.h"
#include "Assets/AssetTypes.h"
#include "IO/Path.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    class Logger;
    
    namespace Model {
        class Map;
        
        class Game {
        private:
            Logger* m_logger;
        public:
            static const String GameNames[];
            static const size_t GameCount;

            static GamePtr game(const String& gameName, Logger* logger);
            static GamePtr game(const size_t gameIndex, Logger* logger);
            
            static GamePtr detectGame(const IO::Path& path, Logger* logger);
            static StringList gameList();
            
            virtual ~Game();
            Map* newMap() const;
            Map* loadMap(const BBox3& worldBounds, const IO::Path& path) const;
            
            IO::Path::List extractTexturePaths(const Map* map) const;
            Assets::FaceTextureCollection* loadTextureCollection(const IO::Path& path) const;
            void uploadTextureCollection(Assets::FaceTextureCollection* collection) const;
            
            Assets::EntityDefinitionList loadEntityDefinitions(const IO::Path& path) const;
            IO::Path defaultEntityDefinitionFile() const;
            IO::Path extractEntityDefinitionFile(const Map* map) const;
            
            Assets::EntityModel* loadModel(const IO::Path& path) const;
        protected:
            Game(Logger* logger);
        private:
            virtual Map* doNewMap() const = 0;
            virtual Map* doLoadMap(const BBox3& worldBounds, const IO::Path& path) const = 0;
            
            virtual IO::Path::List doExtractTexturePaths(const Map* map) const = 0;
            virtual Assets::FaceTextureCollection* doLoadTextureCollection(const IO::Path& path) const = 0;
            virtual void doUploadTextureCollection(Assets::FaceTextureCollection* collection) const = 0;
            
            virtual Assets::EntityDefinitionList doLoadEntityDefinitions(const IO::Path& path) const = 0;
            virtual IO::Path doDefaultEntityDefinitionFile() const = 0;
            virtual IO::Path doExtractEntityDefinitionFile(const Map* map) const = 0;

            virtual Assets::EntityModel* doLoadModel(const IO::Path& path) const = 0;
        };
    }
}

#endif
