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

#include "SharedPointer.h"
#include "IO/Path.h"
#include "Model/Map.h"
#include "Model/TextureCollection.h"

namespace TrenchBroom {
    namespace Model {
        class Game {
        public:
            typedef std::tr1::shared_ptr<Game> Ptr;
        public:
            virtual ~Game();
            
            Map::Ptr loadMap(const IO::Path& path) const;
            IO::Path::List extractTexturePaths(Model::Map::Ptr map) const;
            TextureCollection::Ptr loadTextureCollection(const IO::Path& path) const;
            void uploadTextureCollection(TextureCollection::Ptr collection) const;
        protected:
            Game();
        private:
            virtual Map::Ptr doLoadMap(const IO::Path& path) const = 0;
            virtual IO::Path::List doExtractTexturePaths(Model::Map::Ptr map) const = 0;
            virtual TextureCollection::Ptr doLoadTextureCollection(const IO::Path& path) const = 0;
            virtual void doUploadTextureCollection(TextureCollection::Ptr collection) const = 0;
        };
    }
}

#endif
