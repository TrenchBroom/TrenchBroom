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

#include "MappedFile.h"

#include "IO/Path.h"

#ifdef _Win32
#include <Windows.h>
#include <fstream>
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#endif

namespace TrenchBroom {
    namespace IO {
#ifdef _Win32
#else
        PosixMappedFile::PosixMappedFile(const Path& path, std::ios_base::openmode mode) :
        MappedFile(),
        m_address(NULL),
        m_size(0),
        m_filedesc(-1) {
            int flags = 0;
            int prot = 0;
            if ((mode & std::ios_base::in)) {
                if ((mode & std::ios_base::out))
                    flags = O_RDWR;
                else
                    flags = O_RDONLY;
                prot |= PROT_READ;
            }
            if ((mode & std::ios_base::out)) {
                if (!(mode & std::ios_base::in))
                    flags = O_WRONLY;
                prot |= PROT_WRITE;
            }
            
            m_filedesc = open(path.asString().c_str(), flags);
            if (m_filedesc >= 0) {
                m_size = static_cast<size_t>(lseek(m_filedesc, 0, SEEK_END));
                lseek(m_filedesc, 0, SEEK_SET);
                m_address = static_cast<char*>(mmap(NULL, m_size, prot, MAP_FILE | MAP_PRIVATE, m_filedesc, 0));
                if (m_address != NULL) {
                    init(m_address, m_address + m_size);
                } else {
                    close(m_filedesc);
                    m_filedesc = -1;
                }
            }
        }
        
        PosixMappedFile::~PosixMappedFile() {
            if (m_address != NULL) {
                munmap(m_address, m_size);
            }
            
            if (m_filedesc >= 0) {
                close(m_filedesc);
                m_filedesc = -1;
            }
        }
#endif
    }
}
