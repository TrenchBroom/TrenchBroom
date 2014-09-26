/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef __TrenchBroom__MapDocument__
#define __TrenchBroom__MapDocument__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Assets/EntityDefinitionManager.h"
#include "Assets/EntityModelManager.h"
#include "Assets/TextureManager.h"
#include "IO/Path.h"
#include "Model/ModelTypes.h"
#include "View/CachingLogger.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace View {
        class MapDocument : public CachingLogger {
        public:
            static const BBox3 DefaultWorldBounds;
        private:
            BBox3 m_worldBounds;
            IO::Path m_path;
            Model::World* m_world;
            
            Assets::EntityDefinitionManager m_entityDefinitionManager;
            Assets::EntityModelManager m_entityModelManager;
            Assets::TextureManager m_textureManager;
        private:
            MapDocument();
        public:
            static MapDocumentSPtr newMapDocument();
            ~MapDocument();
        };
    }
}

#endif /* defined(__TrenchBroom__MapDocument__) */
