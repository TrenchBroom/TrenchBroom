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

#include "MapDocument.h"

#include "IO/FileSystem.h"
#include "View/Console.h"
#include "View/MapFrame.h"

#include <cassert>

#include <wx/filedlg.h>
#include <wx/msgdlg.h>

namespace TrenchBroom {
    namespace View {
        MapDocument::MapDocument() :
        m_path(""),
        m_modificationCount(0) {}
        
        MapDocument::Ptr MapDocument::newMapDocument() {
            return MapDocument::Ptr(new MapDocument());
        }

        MapDocument::~MapDocument() {
        }
        
        const IO::Path& MapDocument::path() const {
            return m_path;
        }
        
        String MapDocument::filename() const {
            if (m_path.isEmpty())
                return "";
            return  m_path.lastComponent();
        }
        
        Model::Game::Ptr MapDocument::game() const {
            return m_game;
        }
        
        Model::Map::Ptr MapDocument::map() const {
            return m_map;
        }

        bool MapDocument::modified() const {
            return m_modificationCount > 0;
        }

        void MapDocument::incModificationCount() {
            ++m_modificationCount;
        }
        
        void MapDocument::decModificationCount() {
            assert(m_modificationCount > 0);
            --m_modificationCount;
        }

        void MapDocument::clearModificationCount() {
            m_modificationCount = 0;
        }

        void MapDocument::newDocument(Model::Game::Ptr game) {
            assert(game != NULL);
            m_game = game;
            m_map = Model::Map::newMap();
            
            setDocumentPath(IO::Path(""));
            clearModificationCount();
        }
        
        void MapDocument::openDocument(Model::Game::Ptr game, const IO::Path& path) {
            assert(game != NULL);
            m_map = game->loadMap(path);
            m_game = game;
            
            setDocumentPath(path);
            clearModificationCount();
        }

        void MapDocument::saveDocument() {
            assert(!m_path.isEmpty());
            doSaveDocument(m_path);
        }
        
        void MapDocument::saveDocumentAs(const IO::Path& path) {
            doSaveDocument(path);
        }

        void MapDocument::doSaveDocument(const IO::Path& path) {
            setDocumentPath(path);
        }

        void MapDocument::setDocumentPath(const IO::Path& path) {
            m_path = path;
        }
    }
}
