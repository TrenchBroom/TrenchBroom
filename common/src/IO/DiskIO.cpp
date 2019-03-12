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

#include "DiskIO.h"

#include "IO/File.h"

#include <wx/dir.h>
#include <wx/filefn.h>
#include <wx/filename.h>

#include <cstdio>
#include <fstream>

namespace TrenchBroom {
    namespace IO {
        namespace Disk {
            bool doCheckCaseSensitive();
            Path findCaseSensitivePath(const Path::List& list, const Path& path);
            Path fixCase(const Path& path);
            
            bool doCheckCaseSensitive() {
                const wxString cwd = ::wxGetCwd();
                assert(::wxDirExists(cwd));
                return !::wxDirExists(cwd.Upper()) || !wxDirExists(cwd.Lower());
            }
            
            bool isCaseSensitive() {
                static const bool caseSensitive = doCheckCaseSensitive();
                return caseSensitive;
            }
            
            Path findCaseSensitivePath(const Path::List& list, const Path& path) {
                for (const Path& entry : list) {
                    if (StringUtils::caseInsensitiveEqual(entry.asString(), path.asString()))
                        return entry;
                }
                return Path("");
            }
            
            Path fixCase(const Path& path) {
                try {
                    if (!path.isAbsolute())
                        throw FileSystemException("Cannot fix case of relative path: '" + path.asString() + "'");
                    
                    if (path.isEmpty() || !isCaseSensitive())
                        return path;
                    const String str = path.asString();
                    if (::wxFileExists(str) || ::wxDirExists(str))
                        return path;
                    
                    Path result(path.firstComponent());
                    Path remainder(path.deleteFirstComponent());
                    if (remainder.isEmpty())
                        return result;
                    
                    while (!remainder.isEmpty()) {
                        const String nextPathStr = (result + remainder.firstComponent()).asString();
                        if (!::wxDirExists(nextPathStr) &&
                            !::wxFileExists(nextPathStr)) {
                            const Path::List content = getDirectoryContents(result);
                            const Path part = findCaseSensitivePath(content, remainder.firstComponent());
                            if (part.isEmpty())
                                return path;
                            result = result + part;
                        } else {
                            result = result + remainder.firstComponent();
                        }
                        remainder = remainder.deleteFirstComponent();
                    }
                    return result;
                } catch (const PathException& e) {
                    throw FileSystemException("Cannot fix case of path: '" + path.asString() + "'", e);
                }
            }
            
            Path fixPath(const Path& path) {
                try {
                    if (!path.isAbsolute())
                        throw FileSystemException("Cannot fix relative path: '" + path.asString() + "'");
                    return fixCase(path.makeCanonical());
                } catch (const PathException& e) {
                    throw FileSystemException("Cannot fix path: '" + path.asString() + "'", e);
                }
            }
            
            bool directoryExists(const Path& path) {
                const Path fixedPath = fixPath(path);
                return ::wxDirExists(fixedPath.asString());
            }
            
            bool fileExists(const Path& path) {
                const Path fixedPath = fixPath(path);
                return ::wxFileExists(fixedPath.asString());
            }
            
            String replaceForbiddenChars(const String& name) {
                static const String forbidden = wxFileName::GetForbiddenChars().ToStdString();
                return StringUtils::replaceChars(name, forbidden, "_");
            }

            Path::List getDirectoryContents(const Path& path) {
                const Path fixedPath = fixPath(path);
                wxDir dir(fixedPath.asString());
                if (!dir.IsOpened()) {
                    throw FileSystemException("Cannot open directory: '" + fixedPath.asString() + "'");
                }

                Path::List result;
                wxString filename;
                if (dir.GetFirst(&filename)) {
                    result.push_back(Path(filename.ToStdString()));
                    while (dir.GetNext(&filename)) {
                        result.push_back(Path(filename.ToStdString()));
                    }
                }
                
                return result;
            }
            
            std::shared_ptr<File> openFile(const Path& path) {
                const Path fixedPath = fixPath(path);
                if (!fileExists(fixedPath)) {
                    throw FileNotFoundException("File not found: '" + fixedPath.asString() + "'");
                }

                return std::make_shared<CFile>(fixedPath);
            }
            
            Path getCurrentWorkingDir() {
                return Path(::wxGetCwd().ToStdString());
            }
            
            Path::List findItems(const Path& path) {
                return findItems(path, FileTypeMatcher());
            }
            
            Path::List findItemsRecursively(const Path& path) {
                return findItemsRecursively(path, FileTypeMatcher());
            }
            
            void createFile(const Path& path, const String& contents) {
                const Path fixedPath = fixPath(path);
                if (fileExists(fixedPath)) {
                    deleteFile(fixedPath);
                } else {
                    const Path directory = fixedPath.deleteLastComponent();
                    if (!directoryExists(directory))
                        createDirectory(directory);
                }
                
                const String fixedPathStr = fixedPath.asString();
                std::ofstream stream(fixedPathStr.c_str());
                stream  << contents;
            }

            bool createDirectoryHelper(const Path& path);
            
            void createDirectory(const Path& path) {
                const Path fixedPath = fixPath(path);
                if (fileExists(fixedPath))
                    throw FileSystemException("Could not create directory '" + fixedPath.asString() + "': A file already exists at that path.");
                if (directoryExists(fixedPath))
                    throw FileSystemException("Could not create directory '" + fixedPath.asString() + "': A directory already exists at that path.");
                if (!createDirectoryHelper(fixedPath))
                    throw FileSystemException("Could not create directory '" + fixedPath.asString() + "'");
            }
            
            bool createDirectoryHelper(const Path& path) {
                if (path.isEmpty())
                    return false;
                const IO::Path parent = path.deleteLastComponent();
                if (!::wxDirExists(parent.asString()) && !createDirectoryHelper(parent))
                    return false;
                return ::wxMkdir(path.asString());
            }

            void ensureDirectoryExists(const Path& path) {
                const Path fixedPath = fixPath(path);
                if (fileExists(fixedPath))
                    throw FileSystemException("Could not create directory '" + fixedPath.asString() + "': A file already exists at that path.");
                if (!directoryExists(fixedPath))
                    createDirectoryHelper(fixedPath);
            }

            void deleteFile(const Path& path) {
                const Path fixedPath = fixPath(path);
                if (!fileExists(fixedPath))
                    throw FileSystemException("Could not delete file '" + fixedPath.asString() + "': File does not exist.");
                if (!::wxRemoveFile(fixedPath.asString()))
                    throw FileSystemException("Could not delete file '" + path.asString() + "'");
            }
            
            void copyFile(const Path& sourcePath, const Path& destPath, const bool overwrite) {
                const Path fixedSourcePath = fixPath(sourcePath);
                Path fixedDestPath = fixPath(destPath);
                if (!overwrite && fileExists(fixedDestPath))
                    throw FileSystemException("Could not copy file '" + fixedSourcePath.asString() + "' to '" + fixedDestPath.asString() + "': file already exists");
                if (directoryExists(fixedDestPath))
                    fixedDestPath = fixedDestPath + sourcePath.lastComponent();
                if (!::wxCopyFile(fixedSourcePath.asString(), fixedDestPath.asString(), overwrite))
                    throw FileSystemException("Could not copy file '" + fixedSourcePath.asString() + "' to '" + fixedDestPath.asString() + "'");
            }
            
            void moveFile(const Path& sourcePath, const Path& destPath, const bool overwrite) {
                const Path fixedSourcePath = fixPath(sourcePath);
                Path fixedDestPath = fixPath(destPath);
                if (!overwrite && fileExists(fixedDestPath))
                    throw FileSystemException("Could not move file '" + fixedSourcePath.asString() + "' to '" + fixedDestPath.asString() + "': file already exists");
                if (directoryExists(fixedDestPath))
                    fixedDestPath = fixedDestPath + sourcePath.lastComponent();
                if (!::wxRenameFile(fixedSourcePath.asString(), fixedDestPath.asString(), overwrite))
                    throw FileSystemException("Could not move file '" + fixedSourcePath.asString() + "' to '" + fixedDestPath.asString() + "'");
            }
            
            IO::Path resolvePath(const Path::List& searchPaths, const Path& path) {
                if (path.isAbsolute()) {
                    if (fileExists(path) || directoryExists(path))
                        return path;
                } else {
                    for (const Path& searchPath : searchPaths) {
                        if (searchPath.isAbsolute()) {
                            try {
                                const Path fullPath = searchPath + path;
                                if (fileExists(fullPath) || directoryExists(fullPath))
                                    return fullPath;
                            } catch (const Exception&) {}
                        }
                    }
                }
                return Path("");
            }
        }
    }
}
