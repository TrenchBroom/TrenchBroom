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

#ifndef __TrenchBroom__MapDocument__
#define __TrenchBroom__MapDocument__

#include "SharedPointer.h"
#include "StringUtils.h"
#include "IO/Path.h"

namespace TrenchBroom {
    namespace View {
        class DocumentManager;
        class MapDocument;
        
        typedef std::tr1::shared_ptr<MapDocument> MapDocumentPtr;
        
        class MapFrame;

        class MapDocument {
        private:
            IO::Path m_path;
            MapFrame* m_frame;
            MapDocument();
        public:
            static MapDocumentPtr newMapDocument();
            
            ~MapDocument();
            
            const IO::Path& path() const;
            const String filename() const;
            
            void newDocument();
            void openDocument(const IO::Path& path);
            
            MapFrame* frame() const;
        private:
            void createOrRaiseFrame();
            void destroyFrame();
            
            bool closeDocument();
            
            friend class DocumentManager;
        };
    }
}

#endif /* defined(__TrenchBroom__MapDocument__) */
