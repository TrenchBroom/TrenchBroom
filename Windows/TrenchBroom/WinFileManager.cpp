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

#include "Utilities/Utils.h"

#include <windows.h>
#include <sstream>

namespace TrenchBroom {
    namespace IO {
            bool WinFileManager::isDirectory(const std::string& path) {
                DWORD dwAttrib = GetFileAttributes(path.c_str());
                return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
            }

            bool WinFileManager::exists(const std::string& path) {
                return (GetFileAttributes(path.c_str()) != INVALID_FILE_ATTRIBUTES);
            }
            
            bool WinFileManager::makeDirectory(const std::string& path) {
                std::vector<std::string> components = pathComponents(path);
                if (components.empty())
                    return false;

                std::stringstream partialPath(components[0]);

                for (unsigned int i = 1; i < components.size(); i++) {
                    partialPath << pathSeparator() << components[i];
                    if (!exists(partialPath.str()))
                        if (!CreateDirectory(partialPath.str().c_str(), NULL))
                            return false;
                }
                
                return true;
            }
            
            bool WinFileManager::deleteFile(const std::string& path) {
                DWORD dwAttrib = GetFileAttributes(path.c_str());
                if (dwAttrib == INVALID_FILE_ATTRIBUTES || (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
                    return false;

                return DeleteFile(path.c_str()) == TRUE;
            }

            bool WinFileManager::moveFile(const std::string& sourcePath, const std::string& destPath, bool overwrite) {
                DWORD dwSourceAttrib = GetFileAttributes(sourcePath.c_str());
                if (dwSourceAttrib == INVALID_FILE_ATTRIBUTES || (dwSourceAttrib & FILE_ATTRIBUTE_DIRECTORY))
                    return false;

                DWORD dwDestAttrib = GetFileAttributes(destPath.c_str());
                if (dwDestAttrib != INVALID_FILE_ATTRIBUTES) {
                    if (!overwrite || (dwSourceAttrib & FILE_ATTRIBUTE_DIRECTORY))
                        return false;
                    else if (!DeleteFile(destPath.c_str()))
                        return false;
                }

                return MoveFile(sourcePath.c_str(), destPath.c_str()) == TRUE;
            }

            std::vector<std::string> WinFileManager::directoryContents(const std::string& path, std::string extension) {
                std::vector<std::string> result;
                WIN32_FIND_DATA findData;

                std::string extensionLower = toLower(extension);
                std::string wildcardPath = appendPathComponent(path, "*");
                HANDLE hfind = FindFirstFile(wildcardPath.c_str(), &findData);
                bool found = hfind != INVALID_HANDLE_VALUE;
                while (found) {
                    std::string entryName = findData.cFileName;
                    if (extension.empty() || toLower(pathExtension(entryName)) == extensionLower)
                        result.push_back(pathComponents(entryName).back());
                    found = FindNextFile(hfind, &findData) == TRUE;
                }

                FindClose(hfind);
                return result;
            }
    }
}
