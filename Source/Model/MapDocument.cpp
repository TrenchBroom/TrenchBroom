/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#include "IO/MapParser.h"
#include "Model/Map.h"
#include "Utility/Console.h"
#include "Utility/VecMath.h"
#include "View/EditorView.h"

#include <cassert>
#include <ctime>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        IMPLEMENT_DYNAMIC_CLASS(MapDocument, wxDocument)
        
        MapDocument::MapDocument() {}
        
        bool MapDocument::DoOpenDocument(const wxString& file) {
            console().info("Loading file %s", file.mbc_str().data());
            return wxDocument::DoOpenDocument(file);
        }
        
        bool MapDocument::DoSaveDocument(const wxString& file) {
            return wxDocument::DoSaveDocument(file);
        }

        std::istream& MapDocument::LoadObject(std::istream& stream) {
            wxDocument::LoadObject(stream);
            
            clock_t start = clock();
            IO::MapParser parser(stream, console());
            parser.parseMap(*m_map, NULL);
            stream.clear(); // everything went well, prevent wx from displaying an error dialog
            console().info("Loaded map file in %f seconds", (clock() - start) / CLOCKS_PER_SEC / 10000.0f);

            return stream;
        }
        
        std::ostream& MapDocument::SaveObject(std::ostream& stream) {
            return wxDocument::SaveObject(stream);
        }
        
        Model::Map& MapDocument::map() const {
            return *m_map;
        }
        
        Utility::Console& MapDocument::console() const {
            View::EditorView* editorView = dynamic_cast<View::EditorView*>(GetFirstView());
            assert(editorView != NULL);
            return editorView->console();
        }

        bool MapDocument::OnCreate(const wxString& path, long flags) {
            BBox worldBounds(Vec3f(-4096, -4096, -4096), Vec3f(4096, 4096, 4096));
            m_map = new Map(worldBounds);
            
            // initialize here
            
            return wxDocument::OnCreate(path, flags);
        }
        
    }
}