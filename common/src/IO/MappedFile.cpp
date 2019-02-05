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

#include <cassert>
#include <map>

namespace TrenchBroom {
    namespace IO {
        MappedFile::MappedFile(const Path& path) :
        m_path(path),
        m_begin(nullptr),
        m_end(nullptr) {}
        
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

        MappedFileBufferView::MappedFileBufferView(const Path& path, const char* begin, const char* end) :
        MappedFile(path) {
            init(begin, end);
        }

        MappedFileBufferView::MappedFileBufferView(const Path& path, const char* begin, const size_t size) :
        MappedFileBufferView(path, begin, begin + size) {}

        MappedFileView::MappedFileView(MappedFile::Ptr container, const Path& path, const char* begin, const char* end) :
        MappedFileBufferView(path, begin, end),
        m_container(std::move(container)) {}

        MappedFileView::MappedFileView(MappedFile::Ptr container, const Path& path, const char* begin, const size_t size) :
        MappedFileBufferView(path, begin, size),
        m_container(std::move(container)) {}

        MappedFileBuffer::MappedFileBuffer(const Path& path, std::unique_ptr<char[]> buffer, const size_t size) :
        MappedFileBufferView(path, buffer.get(), buffer.get() + size),
        m_buffer(std::move(buffer)) {}

        MappedFile::Ptr openMappedFile(const Path& path, const std::ios_base::openmode mode) {
            using FileCache = std::map<Path, std::weak_ptr<MappedFile>>;
            static FileCache fileCache;

            if (mode == std::ios_base::in) {
                const auto it = fileCache.find(path);
                if (it != std::end(fileCache)) {
                    auto wptr = it->second;
                    if (wptr.expired()) {
                        fileCache.erase(it);
                    } else {
                        return wptr.lock();
                    }
                }
            }

            auto file =
#ifdef _WIN32
            std::make_shared<WinMappedFile>(path, mode);
#else
            std::make_shared<PosixMappedFile>(path, mode);
#endif

            if (mode == std::ios_base::in) {
                fileCache.insert(std::make_pair(path, file));
            }

            return file;
        }

#ifdef _WIN32
        using nstring = std::basic_string<char>;
        using wstring = std::basic_string<WCHAR>;

        nstring toMappingName(nstring str) {
            size_t pos = str.find_first_of('\\');
            while (pos != nstring::npos) {
                str[pos] = '_';
                pos = str.find_first_of('\\', pos + 1);
            }
            return str;
        }

        wstring toWString(const nstring& str) {
            const size_t length = str.size();
            wstring result(length, 0);
            MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str.c_str(), length, result.data(), length);
            return result;
        }

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
            
            const wstring pathName = toWString(path.asString());
            const wstring mappingName = toWString(toMappingName(path.asString()));

            m_mappingHandle = OpenFileMappingW(mapAccess, true, mappingName.c_str());
            if (m_mappingHandle == nullptr) {
                m_fileHandle = CreateFileW(pathName.c_str(), accessMode, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
                if (m_fileHandle == INVALID_HANDLE_VALUE) {
                    throwError(path, "CreateFile");
                } else {
                    size = static_cast<size_t>(GetFileSize(m_fileHandle, nullptr));
                    m_mappingHandle = CreateFileMappingW(m_fileHandle, nullptr, protect, 0, 0, mappingName.c_str());
                    if (m_mappingHandle == nullptr) {
                        throwError(path, "CreateFileMapping");
                    }
                }
            } else {
                WIN32_FILE_ATTRIBUTE_DATA attrs;
                const BOOL result = GetFileAttributesExW(pathName.c_str(), GetFileExInfoStandard, &attrs);
                if (result == 0) {
                    throwError(path, "GetFileAttributesEx");
                } else {
                    size = (attrs.nFileSizeHigh << 16) + attrs.nFileSizeLow;
                }
            }
            
            assert(m_mappingHandle != nullptr);
            m_address = static_cast<char*>(MapViewOfFile(m_mappingHandle, mapAccess, 0, 0, 0));
            if (m_address == nullptr) {
                throwError(path, "MapViewOfFile");
            } else {
                init(m_address, m_address + size);
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

        void WinMappedFile::throwError(const Path& path, const String& functionName) {
            char buf[512];
            const auto error = GetLastError();
            FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                buf, (sizeof(buf) / sizeof(char)), NULL);

            StringStream msg;
            msg << "Cannot open file " << path << ": Function " << functionName << " threw error " << error << " - "  << buf;
            throw FileSystemException(msg.str());
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
                if ((mode & std::ios_base::out)) {
                    flags = O_RDWR;
                } else {
                    flags = O_RDONLY;
                }
                prot |= PROT_READ;
            }
            if ((mode & std::ios_base::out)) {
                if (!(mode & std::ios_base::in)) {
                    flags = O_WRONLY;
                }
                prot |= PROT_WRITE;
            }

            const auto pathName = path.asString();
            m_filedesc = open(pathName.c_str(), flags);
            if (m_filedesc == 0) {
                throw FileSystemException() << "Cannot open file " << path << ": open() failed";
            }

            m_size = static_cast<size_t>(lseek(m_filedesc, 0, SEEK_END));
            lseek(m_filedesc, 0, SEEK_SET);

            m_address = static_cast<char*>(mmap(nullptr, m_size, prot, MAP_FILE | MAP_PRIVATE, m_filedesc, 0));
            if (m_address == nullptr) {
                throw FileSystemException() << "Cannot open file " << path << ": mmap() failed";
            }

            init(m_address, m_address + m_size);
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
