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

namespace TrenchBroom {
    namespace Model {
        IMPLEMENT_DYNAMIC_CLASS(MapDocument, wxDocument)
        
        MapDocument::MapDocument() {}
        
        std::istream& MapDocument::LoadObject(std::istream& stream) {
            wxDocument::LoadObject(stream);
            
            IO::MapParser parser(stream);
            parser.parseMap(*m_map, NULL);
            return stream;
        }
        
        std::ostream& MapDocument::SaveObject(std::ostream& stream) {
            return wxDocument::SaveObject(stream);
        }
        
        bool MapDocument::OnCreate(const wxString& path, long flags) {
            m_map = new Map();
            
            // initialize here
            
            return wxDocument::OnCreate(path, flags);
        }
        
    }
}