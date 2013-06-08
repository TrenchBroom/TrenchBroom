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
#include "View/MapFrame.h"

#include <cassert>

#include <wx/filedlg.h>
#include <wx/msgdlg.h>

namespace TrenchBroom {
    namespace View {
        MapDocument::MapDocument() :
        m_path(""),
        m_frame(NULL),
        m_modificationCount(0) {}
        
        MapDocument::Ptr MapDocument::newMapDocument() {
            MapDocument::Ptr document = MapDocument::Ptr(new MapDocument());
            document->setPtr(document);
            return document;
        }

        MapDocument::~MapDocument() {
            destroyFrame();
        }
        
        const IO::Path& MapDocument::path() const {
            return m_path;
        }

        String MapDocument::filename() const {
            if (m_path.isEmpty())
                return "";
            return  m_path.lastComponent();
        }

        bool MapDocument::isModified() const {
            return m_modificationCount > 0;
        }

        void MapDocument::incModificationCount() {
            ++m_modificationCount;
            m_frame->OSXSetModified(true);
        }
        
        void MapDocument::decModificationCount() {
            assert(m_modificationCount > 0);
            --m_modificationCount;
            m_frame->OSXSetModified(m_modificationCount > 0);
        }

        void MapDocument::clearModificationCount() {
            m_modificationCount = 0;
            if (m_frame != NULL)
                m_frame->OSXSetModified(false);
        }

        bool MapDocument::newDocument() {
            if (!confirmDiscardChanges())
                return false;
            
            m_path = IO::Path("");
            clearModificationCount();
            return true;
        }
        
        bool MapDocument::openDocument(const IO::Path& path) {
            if (!confirmDiscardChanges())
                return false;

            m_path = path;
            clearModificationCount();
            return true;
        }

        bool MapDocument::saveDocument() {
            IO::FileSystem fs;
            if (!fs.exists(m_path)) {
                const wxString newPathStr = ::wxSaveFileSelector("", "map", filename(), m_frame);
                if (newPathStr.empty())
                    return false;
                const IO::Path newPath(newPathStr.ToStdString());
                return saveDocumentAs(newPath);
            }
            return doSaveDocument(m_path);
        }
        
        bool MapDocument::saveDocumentAs(const IO::Path& path) {
            return doSaveDocument(path);
        }

        MapFrame* MapDocument::frame() const {
            return m_frame;
        }

        void MapDocument::setPtr(MapDocument::Ptr ptr) {
            m_ptr = ptr;
        }

        bool MapDocument::confirmDiscardChanges() {
            if (!isModified())
                return true;
            const int result = ::wxMessageBox(filename() + " has been modified. Do you want to save the changes?", "", wxYES_NO | wxCANCEL, m_frame);
            switch (result) {
                case wxYES:
                    return saveDocument();
                case wxNO:
                    return true;
                default:
                    return false;
            };
        }

        void MapDocument::createOrRaiseFrame() {
            if (m_frame == NULL)
                m_frame = new MapFrame(Ptr(m_ptr));
            m_frame->Show();
            m_frame->Raise();
        }
        
        void MapDocument::destroyFrame() {
            if (m_frame != NULL) {
                m_frame->Destroy();
                m_frame = NULL;
            }
        }

        bool MapDocument::doSaveDocument(const IO::Path& path) {
            m_path = path;
            return true;
        }

        bool MapDocument::closeDocument() {
            if (!confirmDiscardChanges())
                return false;

            destroyFrame();
            return true;
        }
    }
}
