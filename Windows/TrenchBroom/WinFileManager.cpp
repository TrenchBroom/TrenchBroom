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

#include <windows.h>

namespace TrenchBroom {
	namespace IO {
			bool WinFileManager::isDirectory(const std::string& path) {
				DWORD dwAttrib = GetFileAttributes(path.c_str());
				return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
			}

			bool WinFileManager::exists(const std::string& path) {
				return (GetFileAttributes(path.c_str()) != INVALID_FILE_ATTRIBUTES);
			}
			
			std::vector<std::string> WinFileManager::directoryContents(const std::string& path, std::string extension) {
				std::vector<std::string> result;
				WIN32_FIND_DATA findData;

				HANDLE hfind = FindFirstFile(path.c_str(), &findData);
				bool found = hfind != INVALID_HANDLE_VALUE;
				while (found) {
					std::string entryName = findData.cFileName;
					if (extension.empty() || pathExtension(entryName) == extension)
						result.push_back(pathComponents(entryName).back());
					found = FindNextFile(hfind, &findData) == TRUE;
				}

				FindClose(hfind);
				return result;
			}
	}
}
