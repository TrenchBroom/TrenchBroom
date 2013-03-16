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
#include "Utility/SharedPointer.h"
#include "Utility/String.h"

#include <cassert>

namespace TrenchBroom {
    namespace IO {
        class MappedFile {
        public:
            typedef std::tr1::shared_ptr<MappedFile> Ptr;
        protected:
            char* m_begin;
            char* m_end;
            size_t m_size;
        public:
            MappedFile(char* begin, char* end) :
            m_begin(begin),
            m_end(end) {
                assert(m_end >= m_begin);
                m_size = static_cast<size_t>(m_end - m_begin);
            }
            virtual ~MappedFile() {};
            
            inline size_t size() const {
                return m_size;
            }
            
            inline char* begin() const {
                return m_begin;
            }
            
            inline char* end() const {
                return m_end;
            }
        };
        
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
            StringList directoryContents(const String& path, String extension = "", bool directories = true, bool files = true);
            bool resolveRelativePath(const String& relativePath, const StringList& rootPaths, String& absolutePath);
            StringList resolveSearchpaths(const String& rootPath, const StringList& searchPaths);
            
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
            
            virtual String logDirectory() = 0;
            virtual String resourceDirectory() = 0;
            virtual String resolveFontPath(const String& fontName) = 0;
            
            virtual MappedFile::Ptr mapFile(const String& path, std::ios_base::openmode mode = std::ios_base::in) = 0;
        };
        
        class FileManagerException : public Utility::MessageException {
        public:
            FileManagerException(const StringStream& msg) : MessageException(msg) {}
        };
    }
}


#endif /* defined(__TrenchBroom__AbstractFileManager__) */
