/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#include "MappedFile.h"

#include "Exceptions.h"
#include "IO/Path.h"

#ifdef _WIN32
#include <Windows.h>
#include <fstream>
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#endif

namespace TrenchBroom {
    namespace IO {
        const TypedAttributeMap::Attribute<float> MappedFile::Transparency("Transparency", 1.0f);

        MappedFile::MappedFile(const Path& path) :
        m_path(path),
        m_begin(nullptr),
        m_end(nullptr) {
        }
        
        MappedFile::~MappedFile() {
            m_begin = nullptr;
            m_end = nullptr;
        }

        const Path& MappedFile::path() const {
            return m_path;
        }

        size_t MappedFile::size() const {
            return static_cast<size_t>(m_end - m_begin);
        }
        
        const char* MappedFile::begin() const {
            return m_begin;
        }
        
        const char* MappedFile::end() const {
            return m_end;
        }

        void MappedFile::init(const char* begin, const char* end) {
            assert(m_begin == nullptr && m_end == nullptr);
            if (end < begin)
                throw FileSystemException("End of mapped file is before begin");
            m_begin = begin;
            m_end = end;
        }

        MappedFileView::MappedFileView(MappedFile::Ptr container, const Path& path, const char* begin, const char* end) :
        MappedFile(path),
        m_container(container) {
            init(begin, end);
        }

        MappedFileView::MappedFileView(MappedFile::Ptr container, const Path& path, const char* begin, const size_t size) :
        MappedFile(path),
        m_container(container) {
            init(begin, begin + size);
        }

        MappedFileBuffer::MappedFileBuffer(const Path& path, std::unique_ptr<char[]> buffer, const size_t size) :
        MappedFile(path),
        m_buffer(std::move(buffer)) {
            const auto* begin = m_buffer.get();
            init(begin, begin + size);
        }

#ifdef _WIN32
        WinMappedFile::WinMappedFile(const Path& path, std::ios_base::openmode mode) :
        MappedFile(path),
        m_fileHandle(INVALID_HANDLE_VALUE),
        m_mappingHandle(nullptr),
        m_address(nullptr) {
            size_t size = 0;
            
            DWORD accessMode = 0;
		    DWORD protect = 0;
		    DWORD mapAccess = 0;
		    if ((mode & (std::ios_base::in | std::ios_base::out)) == (std::ios_base::in | std::ios_base::out)) {
			    accessMode = GENERIC_READ | GENERIC_WRITE;
			    protect = PAGE_READWRITE;
			    mapAccess = FILE_MAP_ALL_ACCESS;
		    } else if (mode & (std::ios_base::out)) {
			    accessMode = GENERIC_WRITE;
			    protect = PAGE_READWRITE;
			    mapAccess = FILE_MAP_WRITE;
		    } else {
			    accessMode = GENERIC_READ;
			    protect = PAGE_READONLY;
			    mapAccess = FILE_MAP_READ;
		    }
            
            const String pathStr = path.asString();
		    const size_t numChars = pathStr.size();
		    LPWSTR uFilename = new wchar_t[numChars + 1];
		    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pathStr.c_str(), numChars, uFilename, numChars + 1);
		    uFilename[numChars] = 0;
            
		    char* mappingName = new char[numChars + 1];
		    for (size_t i = 0; i < numChars; i++) {
			    if (pathStr[i] == '\\')
				    mappingName[i] = '_';
			    else
				    mappingName[i] = pathStr[i];
		    }
		    mappingName[numChars] = 0;
            
		    LPWSTR uMappingName = new TCHAR[numChars + 1];
		    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, mappingName, numChars, uMappingName, numChars + 1);
		    uMappingName[numChars] = 0;
		    delete [] mappingName;
            
		    m_mappingHandle = OpenFileMapping(mapAccess, true, uMappingName);
		    if (m_mappingHandle == nullptr) {
			    m_fileHandle = CreateFile(uFilename, accessMode, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
                delete [] uFilename;

			    if (m_fileHandle != INVALID_HANDLE_VALUE) {
				    size = static_cast<size_t>(GetFileSize(m_fileHandle, nullptr));
				    m_mappingHandle = CreateFileMapping(m_fileHandle, nullptr, protect, 0, 0, uMappingName);
                    delete [] uMappingName;
			    } else {
                    delete [] uMappingName;
                    throw FileSystemException("Cannot open file " + path.asString());
                }
		    } else {
                WIN32_FILE_ATTRIBUTE_DATA attrs;
                const BOOL result = GetFileAttributesEx(uFilename, GetFileExInfoStandard, &attrs);
                
                delete [] uFilename;
                delete [] uMappingName;
                
                if (result != 0) {
                    size = (attrs.nFileSizeHigh << 16) + attrs.nFileSizeLow;
                } else {
				    CloseHandle(m_mappingHandle);
				    m_mappingHandle = nullptr;
                    throw FileSystemException("Cannot open file " + path.asString());
                }
		    }
            
		    if (m_mappingHandle != nullptr) {
			    m_address = static_cast<char*>(MapViewOfFile(m_mappingHandle, mapAccess, 0, 0, 0));
			    if (m_address != nullptr) {
                    init(m_address, m_address + size);
			    } else {
				    CloseHandle(m_mappingHandle);
				    m_mappingHandle = nullptr;
				    CloseHandle(m_fileHandle);
				    m_fileHandle = INVALID_HANDLE_VALUE;
                    throw FileSystemException("Cannot open file " + path.asString());
			    }
		    } else {
			    if (m_fileHandle != INVALID_HANDLE_VALUE) {
				    CloseHandle(m_fileHandle);
				    m_fileHandle = INVALID_HANDLE_VALUE;
                    throw FileSystemException("Cannot open file " + path.asString());
			    }
		    }
        }
        
        WinMappedFile::~WinMappedFile() {
            if (m_address != nullptr) {
        	    UnmapViewOfFile(m_address);
                m_address = nullptr;
            }
            
		    if (m_mappingHandle != nullptr) {
			    CloseHandle(m_mappingHandle);
			    m_mappingHandle = nullptr;
		    }
            
		    if (m_fileHandle != INVALID_HANDLE_VALUE) {
			    CloseHandle(m_fileHandle);
			    m_fileHandle = INVALID_HANDLE_VALUE;
		    }
        }
#else
        PosixMappedFile::PosixMappedFile(const Path& path, std::ios_base::openmode mode) :
        MappedFile(path),
        m_address(nullptr),
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
                m_address = static_cast<char*>(mmap(nullptr, m_size, prot, MAP_FILE | MAP_PRIVATE, m_filedesc, 0));
                if (m_address != nullptr) {
                    init(m_address, m_address + m_size);
                } else {
                    close(m_filedesc);
                    m_filedesc = -1;
                    throw FileSystemException("Cannot open file " + path.asString());
                }
            } else {
                throw FileSystemException("Cannot open file " + path.asString());
            }
        }
        
        PosixMappedFile::~PosixMappedFile() {
            if (m_address != nullptr) {
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
