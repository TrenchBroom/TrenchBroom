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

#include "Utility/MessageException.h"
#include "Utility/String.h"

namespace TrenchBroom {
    namespace IO {
        class AbstractFileManager {
        public:
            virtual ~AbstractFileManager() {}
            
            bool isAbsolutePath(const String& path);
            bool isDirectory(const String& path);
            bool exists(const String& path);
            bool makeDirectory(const String& path);
            bool deleteFile(const String& path);
            bool moveFile(const String& sourcePath, const String& destPath, bool overwrite);
            char pathSeparator();
            StringList directoryContents(const String& path, String extension = "");
            bool resolveRelativePath(const String& relativePath, const StringList& rootPaths, String& absolutePath);
            
            StringList pathComponents(const String& path);
            String joinComponents(const StringList& pathComponents);
            String deleteLastPathComponent(const String& path);
            String appendPathComponent(const String& path, const String& component);
            String appendPath(const String& prefix, const String& suffix);
            String resolvePath(const String& path);
            StringList resolvePath(const StringList& pathComponents);
            String makeRelative(const String& absolutePath, const String& referencePath);
            String makeAbsolute(const String& relativePath, const String& referencePath);
            
            String pathExtension(const String& path);
            String appendExtension(const String& path, const String& ext);
            String deleteExtension(const String& path);
            
            virtual String resourceDirectory() = 0;
            virtual String resolveFontPath(const String& fontName) = 0;
        };
        
        class FileManagerException : public Utility::MessageException {
        public:
            FileManagerException(const StringStream& msg) : MessageException(msg) {}
        };
    }
}


#endif /* defined(__TrenchBroom__AbstractFileManager__) */
