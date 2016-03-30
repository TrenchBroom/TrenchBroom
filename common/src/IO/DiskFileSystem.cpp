/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "DiskFileSystem.h"

#include "Exceptions.h"
#include "StringUtils.h"

#include <wx/dir.h>
#include <wx/filefn.h>
#include <wx/filename.h>

#include <cassert>
#include <iostream>

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
                Path::List::const_iterator it, end;
                for (it = list.begin(), end = list.end(); it != end; ++it) {
                    const Path& entry = *it;
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
            
            Path::List getDirectoryContents(const Path& path) {
                const Path fixedPath = fixPath(path);
                wxDir dir(fixedPath.asString());
                if (!dir.IsOpened())
                    throw FileSystemException("Cannot open directory: '" + fixedPath.asString() + "'");
                
                Path::List result;
                wxString filename;
                if (dir.GetFirst(&filename)) {
                    result.push_back(Path(filename.ToStdString()));
                    while (dir.GetNext(&filename))
                        result.push_back(Path(filename.ToStdString()));
                }
                
                return result;
            }
            
            MappedFile::Ptr openFile(const Path& path) {
                const Path fixedPath = fixPath(path);
                if (!fileExists(fixedPath))
                    throw FileNotFoundException("File not found: '" + fixedPath.asString() + "'");
#ifdef _WIN32
                return MappedFile::Ptr(new WinMappedFile(fixedPath, std::ios::in));
#else
                return MappedFile::Ptr(new PosixMappedFile(fixedPath, std::ios::in));
#endif
            }
            
            Path getCurrentWorkingDir() {
                return Path(::wxGetCwd().ToStdString());
            }
            
            void createDirectory(const Path& path) {
                const Path fixedPath = fixPath(path);
                if (fileExists(fixedPath))
                    throw FileSystemException("Could not create directory '" + fixedPath.asString() + "': A file already exists at that path.");
                if (directoryExists(fixedPath))
                    throw FileSystemException("Could not create directory '" + fixedPath.asString() + "': A directory already exists at that path.");
                if (!::wxMkdir(fixedPath.asString()))
                    throw FileSystemException("Could not create directory '" + fixedPath.asString() + "'");
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
                const Path fixedDestPath = fixPath(destPath);
                if (!overwrite && fileExists(fixedDestPath))
                    throw FileSystemException("Could not copy file '" + fixedSourcePath.asString() + "' to '" + fixedDestPath.asString() + "': file already exists");
                if (!::wxCopyFile(fixedSourcePath.asString(), fixedDestPath.asString(), overwrite))
                    throw FileSystemException("Could not copy file '" + fixedSourcePath.asString() + "' to '" + fixedDestPath.asString() + "'");
            }
            
            void moveFile(const Path& sourcePath, const Path& destPath, const bool overwrite) {
                const Path fixedSourcePath = fixPath(sourcePath);
                const Path fixedDestPath = fixPath(destPath);
                if (!overwrite && fileExists(fixedDestPath))
                    throw FileSystemException("Could not move file '" + fixedSourcePath.asString() + "' to '" + fixedDestPath.asString() + "': file already exists");
                if (!::wxRenameFile(fixedSourcePath.asString(), fixedDestPath.asString(), overwrite))
                    throw FileSystemException("Could not move file '" + fixedSourcePath.asString() + "' to '" + fixedDestPath.asString() + "'");
            }

            IO::Path resolvePath(const Path::List& searchPaths, const Path& path) {
                if (path.isAbsolute()) {
                    if (fileExists(path) || directoryExists(path))
                        return path;
                } else {
                    for (size_t j = 0; j < searchPaths.size(); ++j) {
                        const Path& searchPath = searchPaths[j];
                        if (fileExists(searchPath + path) || directoryExists(searchPath + path))
                            return searchPath + path;
                    }
                }
                return Path("");
            }
        }
        
        DiskFileSystem::DiskFileSystem(const Path& root, const bool ensureExists) :
        m_root(Disk::fixPath(root)) {
            if (ensureExists && !Disk::directoryExists(m_root))
                throw FileSystemException("Root directory not found: '" + m_root.asString() + "'");
        }
        
        const Path& DiskFileSystem::getPath() const {
            return m_root;
        }
        
        const Path DiskFileSystem::makeAbsolute(const Path& relPath) const {
            return getPath() + relPath.makeCanonical();
        }
        
        bool DiskFileSystem::doDirectoryExists(const Path& path) const {
            return Disk::directoryExists(m_root + path.makeCanonical());
        }
        
        bool DiskFileSystem::doFileExists(const Path& path) const {
            return Disk::fileExists(m_root + path.makeCanonical());
        }
        
        Path::List DiskFileSystem::doGetDirectoryContents(const Path& path) const {
            return Disk::getDirectoryContents(m_root + path.makeCanonical());
        }
        
        const MappedFile::Ptr DiskFileSystem::doOpenFile(const Path& path) const {
            return Disk::openFile(m_root + path.makeCanonical());
        }
        
        WritableDiskFileSystem::WritableDiskFileSystem(const Path& root, const bool create) :
        DiskFileSystem(root, !create) {
            if (create && !Disk::directoryExists(m_root) && !::wxMkdir(m_root.asString()))
                throw FileSystemException("Could not create directory '" + m_root.asString() + "'");
        }
        
        void WritableDiskFileSystem::doCreateDirectory(const Path& path) {
            Disk::createDirectory(m_root + path.makeCanonical());
        }
        
        void WritableDiskFileSystem::doDeleteFile(const Path& path) {
            Disk::deleteFile(m_root + path.makeCanonical());
        }
        
        void WritableDiskFileSystem::doCopyFile(const Path& sourcePath, const Path& destPath, const bool overwrite) {
            Disk::copyFile(m_root + sourcePath.makeCanonical(), m_root + destPath.makeCanonical(), overwrite);
        }

        void WritableDiskFileSystem::doMoveFile(const Path& sourcePath, const Path& destPath, const bool overwrite) {
            Disk::moveFile(m_root + sourcePath.makeCanonical(), m_root + destPath.makeCanonical(), overwrite);
        }
    }
}
