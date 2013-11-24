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
                if (!path.isAbsolute())
                    throw FileSystemException("Cannot handle relative path: '" + path.asString() + "'");
                return ::wxDirExists(path.asString());
            }
            
            bool fileExists(const Path& path) {
                if (!path.isAbsolute())
                    throw FileSystemException("Cannot handle relative path: '" + path.asString() + "'");
                return ::wxFileExists(path.asString());
            }
            
            Path::List getDirectoryContents(const Path& path) {
                if (!path.isAbsolute())
                    throw FileSystemException("Cannot get contents of relative path: '" + path.asString() + "'");
                
                wxDir dir(path.asString());
                if (!dir.IsOpened())
                    throw FileSystemException("Cannot open directory: '" + path.asString() + "'");
                
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
                if (!path.isAbsolute())
                    throw FileSystemException("Cannot open file at relative path: '" + path.asString() + "'");
                if (!fileExists(path))
                    throw FileSystemException("File not found: '" + path.asString() + "'");
#ifdef _WIN32
                return MappedFile::Ptr(new WinMappedFile(path, std::ios::in));
#else
                return MappedFile::Ptr(new PosixMappedFile(path, std::ios::in));
#endif
            }
            
            Path getCurrentWorkingDir() {
                return Path(::wxGetCwd().ToStdString());
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
            return getPath() + fixPath(relPath);
        }
        
        Path DiskFileSystem::fixPath(const Path& path) const {
            if (path.isAbsolute())
                throw FileSystemException("Cannot handle absolute path: '" + path.asString() + "'");
            if (path.isEmpty())
                return path;
            return fixCase(path.makeCanonical());
        }
        
        Path DiskFileSystem::fixCase(const Path& path) const {
            if (path.isEmpty() || !Disk::isCaseSensitive())
                return path;
            const String str = (m_root + path).asString();
            if (::wxFileExists(str) || ::wxDirExists(str))
                return path;
            
            Path result("");
            Path remainder(path);
            if (remainder.isEmpty())
                return result;
            
            while (!remainder.isEmpty()) {
                const Path currentPath = m_root + result;
                const Path nextPath = currentPath + remainder.firstComponent();
                const String nextPathStr = nextPath.asString();
                if (!::wxDirExists(nextPathStr) &&
                    !::wxFileExists(nextPathStr)) {
                    const Path::List content = Disk::getDirectoryContents(currentPath);
                    const Path part = Disk::findCaseSensitivePath(content, remainder.firstComponent());
                    if (part.isEmpty())
                        return path;
                    result = result + part;
                } else {
                    result = result + remainder.firstComponent();
                }
                remainder = remainder.deleteFirstComponent();
            }
            return result;
        }
        
        bool DiskFileSystem::doDirectoryExists(const Path& path) const {
            return Disk::directoryExists(m_root + fixPath(path));
        }
        
        bool DiskFileSystem::doFileExists(const Path& path) const {
            return Disk::fileExists(m_root + fixPath(path));
        }
        
        Path::List DiskFileSystem::doGetDirectoryContents(const Path& path) const {
            return Disk::getDirectoryContents(m_root + fixPath(path));
        }
        
        const MappedFile::Ptr DiskFileSystem::doOpenFile(const Path& path) const {
            return Disk::openFile(m_root + fixPath(path));
        }
        
        WritableDiskFileSystem::WritableDiskFileSystem(const Path& root, const bool create) :
        DiskFileSystem(root, !create) {
            if (create && !Disk::directoryExists(m_root) && !::wxMkdir(m_root.asString()))
                throw FileSystemException("Could not create directory '" + m_root.asString() + "'");
        }
        
        void WritableDiskFileSystem::doCreateDirectory(const Path& path) {
            if (fileExists(path) || directoryExists(path))
                throw FileSystemException("Could not create directory '" + path.asString() + "'");
            if (!::wxMkdir((m_root + fixPath(path)).asString()))
                throw FileSystemException("Could not create directory '" + path.asString() + "'");
        }
        
        void WritableDiskFileSystem::doDeleteFile(const Path& path) {
            if (!fileExists(path))
                throw FileSystemException("Could not delete file '" + path.asString() + "'");
            if (!::wxRemoveFile((m_root + fixPath(path)).asString()))
                throw FileSystemException("Could not delete file '" + path.asString() + "'");
        }
        
        void WritableDiskFileSystem::doMoveFile(const Path& sourcePath, const Path& destPath, const bool overwrite) {
            if (!overwrite && fileExists(destPath))
                throw FileSystemException("Could not move file '" + sourcePath.asString() + "' to '" + destPath.asString() + "'");
            if (!::wxRenameFile((m_root + fixPath(sourcePath)).asString(),
                                (m_root + fixPath(destPath)).asString(),
                                overwrite))
                throw FileSystemException("Could not move file '" + sourcePath.asString() + "' to '" + destPath.asString() + "'");
        }
    }
}
