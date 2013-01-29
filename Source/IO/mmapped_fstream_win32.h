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

#ifndef TrenchBroom_mmapped_fstream_win32_h
#define TrenchBroom_mmapped_fstream_win32_h

#include <cassert>
#include <iostream>
#include <functional>
#include <string>

#include <Windows.h>

class mmapped_fstream_win32 : public std::istream {
private:
    HANDLE m_fileHandle;
	HANDLE m_mappingHandle;
    void* m_address;
    size_t m_length;
    mmapped_streambuf* m_buf;

    mmapped_streambuf* makebuf(const char* filename, ios_base::openmode mode = ios_base::in | ios_base::out) {
		m_fileHandle = INVALID_HANDLE_VALUE;
		m_mappingHandle = NULL;
        m_address = NULL;
        m_length = 0;
        m_buf = NULL;
        
        DWORD accessMode = 0;
		DWORD protect = 0;
		DWORD mapAccess = 0;
		if (mode & (ios_base::in | ios_base::out)) {
			accessMode = GENERIC_READ | GENERIC_WRITE;
			protect = PAGE_READWRITE;
			mapAccess = FILE_MAP_ALL_ACCESS;
		} else if (mode & (ios_base::out)) {
			accessMode = GENERIC_WRITE;
			protect = PAGE_READWRITE;
			mapAccess = FILE_MAP_WRITE;
		} else {
			accessMode = GENERIC_READ;
			protect = PAGE_READONLY;
			mapAccess = FILE_MAP_READ;
		}
        
		unsigned int numChars = strlen(filename);
		LPWSTR uFilename = new TCHAR[numChars + 1];
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, filename, numChars, uFilename, numChars + 1);
		uFilename[numChars] = 0;

		char* mappingName = new char[numChars + 1];
		for (size_t i = 0; i < numChars; i++) {
			if (filename[i] == '\\')
				mappingName[i] = '_';
			else
				mappingName[i] = filename[i];
		}
		mappingName[numChars] = 0;

		LPWSTR uMappingName = new TCHAR[numChars + 1];
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, mappingName, numChars, uMappingName, numChars + 1);
		uMappingName[numChars] = 0;
		delete [] mappingName;

		m_mappingHandle = OpenFileMapping(mapAccess, true, uMappingName);
		if (m_mappingHandle == NULL) {
			m_fileHandle = CreateFile(uFilename, accessMode, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (m_fileHandle != INVALID_HANDLE_VALUE) {
				m_length = static_cast<size_t>(GetFileSize(m_fileHandle, NULL));
				m_mappingHandle = CreateFileMapping(m_fileHandle, NULL, protect, 0, 0, uMappingName);
			}
		} else {
			m_fileHandle = CreateFile(uFilename, 0, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (m_fileHandle == INVALID_HANDLE_VALUE) {
				CloseHandle(m_mappingHandle);
				m_mappingHandle = NULL;
			} else {
				m_length = static_cast<size_t>(GetFileSize(m_fileHandle, NULL));
				CloseHandle(m_fileHandle);
				m_fileHandle = INVALID_HANDLE_VALUE;
			}
		}

		if (m_mappingHandle != NULL) {
			m_address = MapViewOfFile(m_mappingHandle, mapAccess, 0, 0, 0);
			if (m_address != NULL) {
				char* begin = static_cast<char*>(m_address);
				char* end = begin + m_length;
				m_buf = new mmapped_streambuf(begin, end);
			} else {
				CloseHandle(m_mappingHandle);
				m_mappingHandle = NULL;
				CloseHandle(m_fileHandle);
				m_fileHandle = INVALID_HANDLE_VALUE;
				clear(std::ios::failbit);
			}
		} else {
			if (m_fileHandle != INVALID_HANDLE_VALUE) {
				CloseHandle(m_fileHandle);
				m_fileHandle = INVALID_HANDLE_VALUE;
			}
			clear(std::ios::failbit);
		}
        
		delete [] uFilename;
		delete [] uMappingName;
        return m_buf;
    }

    
    // copy ctor and assignment not implemented;
    // copying not allowed
    mmapped_fstream_win32(const mmapped_fstream_win32 &);
    mmapped_fstream_win32 &operator= (const mmapped_fstream_win32 &);
public:
    mmapped_fstream_win32(const char* filename, ios_base::openmode mode = ios_base::in | ios_base::out) : std::istream(makebuf(filename, mode)) {
        init(m_buf);
    }
    
    ~mmapped_fstream_win32() {
		if (m_buf != NULL) {
            delete m_buf;
            m_buf = NULL;
        }

        if (m_address != NULL) {
        	UnmapViewOfFile(m_address);
        	m_address = NULL;
        }

		if (m_mappingHandle != NULL) {
			CloseHandle(m_mappingHandle);
			m_mappingHandle = NULL;
		}

		if (m_fileHandle != INVALID_HANDLE_VALUE) {
			CloseHandle(m_fileHandle);
			m_fileHandle = INVALID_HANDLE_VALUE;
		}
    }
    
    inline bool is_open() const {
        return m_address != NULL;
    }
};

#endif
