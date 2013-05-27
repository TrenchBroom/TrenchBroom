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

#ifndef __TrenchBroom__MappedFile__
#define __TrenchBroom__MappedFile__

#include "SharedPointer.h"

#include "Exceptions.h"

#ifdef WIN32
// can't include Windows.h here
typedef void *HANDLE;
#endif

namespace TrenchBroom {
    namespace IO {
        class Path;
        
        class MappedFile {
        public:
            typedef std::tr1::shared_ptr<MappedFile> Ptr;
        private:
            char* m_begin;
            char* m_end;
            size_t m_size;
        public:
            MappedFile() :
            m_begin(NULL),
            m_end(NULL),
            m_size(0) {
            }
            
            virtual ~MappedFile() {
                m_begin = NULL;
                m_end = NULL;
                m_size = 0;
            };
            
            inline size_t size() const {
                return m_size;
            }
            
            inline const char* begin() const {
                return m_begin;
            }
            
            inline const char* end() const {
                return m_end;
            }
        protected:
            void init(char* begin, char* end) {
                assert(m_begin == NULL && m_end == NULL);
                if (end < begin)
                    throw new FileSystemException("End of mapped file is before begin");
                m_begin = begin;
                m_end = end;
                m_size = static_cast<size_t>(m_end - m_begin);
            }
        };

#ifdef WIN32
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

#endif /* defined(__TrenchBroom__MappedFile__) */
