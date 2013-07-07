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

#ifndef TrenchBroom_TextureLoader_h
#define TrenchBroom_TextureLoader_h

#include "SharedPointer.h"
#include "Model/TextureCollection.h"

namespace TrenchBroom {
    namespace IO {
        class Path;
        
        class TextureLoader {
        public:
            virtual ~TextureLoader();
            
            Model::TextureCollection::Ptr loadTextureCollection(const Path& path);
            void uploadTextureCollection(Model::TextureCollection::Ptr collection);
        private:
            virtual Model::TextureCollection::Ptr doLoadTextureCollection(const Path& path) = 0;
            virtual void doUploadTextureCollection(Model::TextureCollection::Ptr collection) = 0;
        };
    }
}

#endif
