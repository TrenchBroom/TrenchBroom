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

#ifndef __TrenchBroom__AbstractFileManager__
#define __TrenchBroom__AbstractFileManager__

#include "IO/mmapped_fstream.h"

#include "Utility/MessageException.h"
#include "Utility/String.h"

namespace TrenchBroom {
    namespace IO {
        class AbstractFileManager {
        public:
            bool isDirectory(const String& path);
            bool exists(const String& path);
            bool makeDirectory(const String& path);
            bool deleteFile(const String& path);
            bool moveFile(const String& sourcePath, const String& destPath, bool overwrite);
            char pathSeparator();
            StringList directoryContents(const String& path, String extension = "");
            
            StringList pathComponents(const String& path);
            String deleteLastPathComponent(const String& path);
            String appendPathComponent(const String& path, const String& component);
            String appendPath(const String& prefix, const String& suffix);
            
            String pathExtension(const String& path);
            String appendExtension(const String& path, const String& ext);
            String deleteExtension(const String& path);
            
            virtual String resourceDirectory() = 0;
        };
        
        class FileManagerException : public Utility::MessageException {
        public:
            FileManagerException(const StringStream& msg) : MessageException(msg) {}
        };
    }
}

#endif /* defined(__TrenchBroom__AbstractFileManager__) */
