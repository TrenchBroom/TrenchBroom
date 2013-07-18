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
#include "IO/Path.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {
        class Game {
        public:
            virtual ~Game();
            
            MapPtr loadMap(const BBox3& worldBounds, const IO::Path& path) const;
            IO::Path::List extractTexturePaths(MapPtr map) const;
            TextureCollectionPtr loadTextureCollection(const IO::Path& path) const;
            void uploadTextureCollection(TextureCollectionPtr collection) const;
        protected:
            Game();
        private:
            virtual MapPtr doLoadMap(const BBox3& worldBounds, const IO::Path& path) const = 0;
            virtual IO::Path::List doExtractTexturePaths(MapPtr map) const = 0;
            virtual TextureCollectionPtr doLoadTextureCollection(const IO::Path& path) const = 0;
            virtual void doUploadTextureCollection(TextureCollectionPtr collection) const = 0;
        };
    }
}

#endif
