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

#pragma once
#include "IO/AbstractFileManager.h"

// can't include Windows.h here
typedef void *HANDLE;

namespace TrenchBroom {
    namespace IO {
        class WinMappedFile : public MappedFile {
        private:
            HANDLE m_fileHandle;
	        HANDLE m_mappingHandle;
        public:
            WinMappedFile(HANDLE fileHandle, HANDLE mappingHandle, char* address, size_t size);
            ~WinMappedFile();
        };

        class WinFileManager : public AbstractFileManager {
        protected:
            String appDirectory();
        public:
            String logDirectory();
            String resourceDirectory();
            String resolveFontPath(const String& fontName);

            
            MappedFile::Ptr mapFile(const String& path, std::ios_base::openmode mode = std::ios_base::in);
        };
    }
}

