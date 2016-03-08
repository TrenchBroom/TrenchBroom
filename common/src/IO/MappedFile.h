/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_MappedFile
#define TrenchBroom_MappedFile

#include "SharedPointer.h"

#include "Exceptions.h"

#include <vector>

#ifdef _WIN32
// can't include Windows.h here
typedef void *HANDLE;
#endif

namespace TrenchBroom {
    namespace IO {
        class Path;
        
        class MappedFile {
        public:
            typedef std::tr1::shared_ptr<MappedFile> Ptr;
            typedef std::vector<Ptr> List;
        private:
            const char* m_begin;
            const char* m_end;
            size_t m_size;
        public:
            MappedFile();
            virtual ~MappedFile();
            
            size_t size() const;
            const char* begin() const;
            const char* end() const;
        protected:
            void init(const char* begin, const char* end);
        };
        
        class MappedFileView : public MappedFile {
        public:
            MappedFileView(const char* begin, const char* end);
        };

#ifdef _WIN32
        class WinMappedFile : public MappedFile {
        private:
            HANDLE m_fileHandle;
	        HANDLE m_mappingHandle;
            char* m_address;
        public:
            WinMappedFile(const Path& path, std::ios_base::openmode mode);
            ~WinMappedFile();
        };
#else
        class PosixMappedFile : public MappedFile {
        private:
            char* m_address;
            size_t m_size;
            int m_filedesc;
        public:
            PosixMappedFile(const Path& path, std::ios_base::openmode mode);
            ~PosixMappedFile();
        };
#endif
    }
}

#endif /* defined(TrenchBroom_MappedFile) */
