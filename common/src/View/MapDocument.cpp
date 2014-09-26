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

#include "View/MapDocument.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace View {
        const BBox3 MapDocument::DefaultWorldBounds(-16384.0, 16384.0);

        MapDocument::MapDocument() :
        m_worldBounds(DefaultWorldBounds),
        m_path(""),
        m_world(NULL),
        m_entityDefinitionManager(),
        m_entityModelManager(this),
        m_textureManager(this, pref(Preferences::TextureMinFilter), pref(Preferences::TextureMagFilter)) {}

        MapDocumentSPtr MapDocument::newMapDocument() {
            return MapDocumentSPtr(new MapDocument());
        }
        
        MapDocument::~MapDocument() {
            delete m_world;
            m_world = NULL;
        }
    }
}
