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

#include "WinFileManager.h"

#include <Windows.h>
#include <fstream>

namespace TrenchBroom {
	namespace IO {
        WinMappedFile::WinMappedFile(HANDLE fileHandle, HANDLE mappingHandle, char* address, size_t size) :
        MappedFile(address, address + size),
        m_fileHandle(fileHandle),
        m_mappingHandle(mappingHandle) {}

        WinMappedFile::~WinMappedFile() {
            if (m_begin != NULL) {
        	    UnmapViewOfFile(m_begin);
        	    m_begin = NULL;
                m_end = NULL;
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

        String WinFileManager::appDirectory() {
			TCHAR uAppPathC[MAX_PATH] = L"";
			DWORD numChars = GetModuleFileName(0, uAppPathC, MAX_PATH - 1);

			char appPathC[MAX_PATH];
			WideCharToMultiByte(CP_ACP, 0, uAppPathC, numChars, appPathC, numChars, NULL, NULL);
			appPathC[numChars] = 0;

			String appPath(appPathC);
			return deleteLastPathComponent(appPath);
        }

        String WinFileManager::logDirectory() {
            return appDirectory();
        }

        String WinFileManager::resourceDirectory() {
			return appendPath(appDirectory(), "Resources");
		}

		String WinFileManager::resolveFontPath(const String& fontName) {
			TCHAR uWindowsPathC[MAX_PATH] = L"";
			DWORD numChars = GetWindowsDirectory(uWindowsPathC, MAX_PATH - 1);

			char windowsPathC[MAX_PATH];
			WideCharToMultiByte(CP_ACP, 0, uWindowsPathC, numChars, windowsPathC, numChars, NULL, NULL);
			windowsPathC[numChars] = 0;

			String windowsPath(windowsPathC);
			if (windowsPath.back() != '\\')
				windowsPath.push_back('\\');

			String extensions[2] = {".ttf", ".ttc"};
			String fontDirectoryPath = windowsPath + "Fonts\\";
			String fontBasePath = fontDirectoryPath + fontName;

			for (int i = 0; i < 2; i++) {
				String fontPath = fontBasePath + extensions[i];
				std::fstream fs(fontPath.c_str(), std::ios::binary | std::ios::in);
				if (fs.is_open())
					return fontPath;
			}

			return fontDirectoryPath + "Arial.ttf";            
		}

        MappedFile::Ptr WinFileManager::mapFile(const String& path, std::ios_base::openmode mode) {
            HANDLE fileHandle = INVALID_HANDLE_VALUE;
		    HANDLE mappingHandle = NULL;
            char* address = NULL;
            size_t size = 0;
        
            DWORD accessMode = 0;
		    DWORD protect = 0;
		    DWORD mapAccess = 0;
		    if (mode & (std::ios_base::in | std::ios_base::out)) {
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
        
		    const size_t numChars = path.size();
		    LPWSTR uFilename = new TCHAR[numChars + 1];
		    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, path.c_str(), numChars, uFilename, numChars + 1);
		    uFilename[numChars] = 0;

		    char* mappingName = new char[numChars + 1];
		    for (size_t i = 0; i < numChars; i++) {
			    if (path[i] == '\\')
				    mappingName[i] = '_';
			    else
				    mappingName[i] = path[i];
		    }
		    mappingName[numChars] = 0;

		    LPWSTR uMappingName = new TCHAR[numChars + 1];
		    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, mappingName, numChars, uMappingName, numChars + 1);
		    uMappingName[numChars] = 0;
		    delete [] mappingName;

		    mappingHandle = OpenFileMapping(mapAccess, true, uMappingName);
		    if (mappingHandle == NULL) {
			    fileHandle = CreateFile(uFilename, accessMode, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			    if (fileHandle != INVALID_HANDLE_VALUE) {
				    size = static_cast<size_t>(GetFileSize(fileHandle, NULL));
				    mappingHandle = CreateFileMapping(fileHandle, NULL, protect, 0, 0, uMappingName);
			    }
		    } else {
                WIN32_FILE_ATTRIBUTE_DATA attrs;
                if (GetFileAttributesEx(uFilename, GetFileExInfoStandard, &attrs) != 0) {
                    size = (attrs.nFileSizeHigh << 16) + attrs.nFileSizeLow;
                } else {
                    DWORD error = GetLastError();
				    CloseHandle(mappingHandle);
				    mappingHandle = NULL;
                }
		    }

            MappedFile::Ptr mappedFile;
		    if (mappingHandle != NULL) {
			    address = static_cast<char*>(MapViewOfFile(mappingHandle, mapAccess, 0, 0, 0));
			    if (address != NULL) {
                    mappedFile = MappedFile::Ptr(new WinMappedFile(fileHandle, mappingHandle, address, size));
			    } else {
				    CloseHandle(mappingHandle);
				    mappingHandle = NULL;
				    CloseHandle(fileHandle);
				    fileHandle = INVALID_HANDLE_VALUE;
			    }
		    } else {
			    if (fileHandle != INVALID_HANDLE_VALUE) {
				    CloseHandle(fileHandle);
				    fileHandle = INVALID_HANDLE_VALUE;
			    }
		    }
        
		    delete [] uFilename;
		    delete [] uMappingName;
            return mappedFile;
        }
    }
}