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

#include "View/MapFrame.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        MapDocument::MapDocument() :
        m_path(""),
        m_frame(NULL) {}
        
        MapDocumentPtr MapDocument::newMapDocument() {
            return MapDocumentPtr(new MapDocument());
        }

        MapDocument::~MapDocument() {
            destroyFrame();
        }
        
        const IO::Path& MapDocument::path() const {
            return m_path;
        }

        const String MapDocument::filename() const {
            if (m_path.isEmpty())
                return "";
            return  m_path.lastComponent();
        }

        void MapDocument::newDocument() {
            m_path = IO::Path("");
            createOrRaiseFrame();
        }
        
        void MapDocument::openDocument(const IO::Path& path) {
            m_path = path;
            createOrRaiseFrame();
        }

        MapFrame* MapDocument::frame() const {
            return m_frame;
        }

        void MapDocument::createOrRaiseFrame() {
            if (m_frame == NULL)
                m_frame = new MapFrame();
            m_frame->Show();
            m_frame->Raise();
        }
        
        void MapDocument::destroyFrame() {
            if (m_frame != NULL) {
                m_frame->Destroy();
                m_frame = NULL;
            }
        }

        bool MapDocument::closeDocument() {
            return true;
        }
    }
}
