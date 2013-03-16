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

#ifndef __TrenchBroom__FileManager__
#define __TrenchBroom__FileManager__

#include "IO/AbstractFileManager.h"

namespace TrenchBroom {
    namespace IO {
        class MacMappedFile : public MappedFile {
        private:
            int m_filedesc;
        public:
            MacMappedFile(int filedesc, char* address, size_t size);
            ~MacMappedFile();
        };
        
        class MacFileManager : public AbstractFileManager {
        public:
            ~MacFileManager() {}
            
            String logDirectory();
            String resourceDirectory();
            String resolveFontPath(const String& fontName);

            MappedFile::Ptr mapFile(const String& path, std::ios_base::openmode mode = std::ios_base::in);
        };
    }
}

#endif /* defined(__TrenchBroom__FileManager__) */
