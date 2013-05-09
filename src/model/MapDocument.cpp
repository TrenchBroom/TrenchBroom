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

#include "view/MapFrame.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        MapDocument::MapDocument() :
        m_frame(NULL) {}
        
        MapDocument::~MapDocument() {
            destroyFrame();
        }
        
        void MapDocument::newDocument() {
            createOrRaiseFrame();
        }
        
        void MapDocument::openDocument(const String& path) {
            createOrRaiseFrame();
        }

        View::MapFrame* MapDocument::getFrame() const {
            return m_frame;
        }

        void MapDocument::createOrRaiseFrame() {
            if (m_frame == NULL)
                m_frame = new View::MapFrame();
            m_frame->Show();
            m_frame->Raise();
        }
        
        void MapDocument::destroyFrame() {
            if (m_frame != NULL) {
                m_frame->Destroy();
                m_frame = NULL;
            }
        }
    }
}
