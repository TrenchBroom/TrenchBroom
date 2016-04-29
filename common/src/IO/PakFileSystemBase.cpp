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

#include "PakFileSystemBase.h"

#include "CollectionUtils.h"
#include "IO/DiskFileSystem.h"
#include "IO/IOUtils.h"

#include <cassert>

namespace TrenchBroom {
    namespace IO {
        PakFileSystemBase::File::~File() {}
        
        MappedFile::Ptr PakFileSystemBase::File::open() {
            return doOpen();
        }
        
        PakFileSystemBase::SimpleFile::SimpleFile(const char* begin, const char* end) :
        m_file(new MappedFileView(begin, end)) {}
        
        MappedFile::Ptr PakFileSystemBase::SimpleFile::doOpen() {
            return m_file;
        }
        
        PakFileSystemBase::Directory::Directory(const Path& path) :
        m_path(path) {}
        
        PakFileSystemBase::Directory::~Directory() {
            MapUtils::clearAndDelete(m_directories);
            MapUtils::clearAndDelete(m_files);
        }
        
        void PakFileSystemBase::Directory::addFile(const Path& path, File* file) {
            assert(file != NULL);
            const String filename = path.lastComponent().asString();
            if (path.length() == 1) {
                if (!MapUtils::insertOrFail(m_files, filename, file))
                    throw new FileSystemException("File '" + filename + "' already exists in directory '" + m_path.asString() + "'");
            } else {
                Directory& dir = findOrCreateDirectory(path.deleteLastComponent());
                dir.addFile(filename, file);
            }
        }
        
        bool PakFileSystemBase::Directory::directoryExists(const Path& path) const {
            if (path.isEmpty())
                return true;
            DirMap::const_iterator it = m_directories.find(path.firstComponent().asString());
            if (it == m_directories.end())
                return false;
            return it->second->directoryExists(path.deleteFirstComponent());
        }
        
        bool PakFileSystemBase::Directory::fileExists(const Path& path) const {
            if (path.length() == 1)
                return m_files.count(path.asString()) > 0;
            DirMap::const_iterator it = m_directories.find(path.firstComponent().asString());
            if (it == m_directories.end())
                return false;
            return it->second->fileExists(path.deleteFirstComponent());
        }
        
        const PakFileSystemBase::Directory& PakFileSystemBase::Directory::findDirectory(const Path& path) const {
            if (path.isEmpty())
                return *this;
            DirMap::const_iterator it = m_directories.find(path.firstComponent().asString());
            if (it == m_directories.end())
                throw new FileSystemException("Path does not exist: '" + (m_path + path).asString() + "'");
            return it->second->findDirectory(path.deleteFirstComponent());
        }
        
        const MappedFile::Ptr PakFileSystemBase::Directory::findFile(const Path& path) const {
            assert(!path.isEmpty());
            
            const String name = path.firstComponent().asString();
            if (path.length() == 1) {
                FileMap::const_iterator it = m_files.find(name);
                if (it == m_files.end())
                    throw new FileSystemException("File not found: '" + (m_path + path).asString() + "'");
                return it->second->open();
            }
            DirMap::const_iterator it = m_directories.find(name);
            if (it == m_directories.end())
                throw new FileSystemException("File not found: '" + (m_path + path).asString() + "'");
            return it->second->findFile(path.deleteFirstComponent());
        }
        
        Path::List PakFileSystemBase::Directory::contents() const {
            Path::List contents;
            
            DirMap::const_iterator dIt, dEnd;
            for (dIt = m_directories.begin(), dEnd = m_directories.end(); dIt != dEnd; ++dIt)
                contents.push_back(Path(dIt->first));
            
            FileMap::const_iterator fIt, fEnd;
            for (fIt = m_files.begin(), fEnd = m_files.end(); fIt != fEnd; ++fIt)
                contents.push_back(Path(fIt->first));
            
            return contents;
        }
        
        PakFileSystemBase::Directory& PakFileSystemBase::Directory::findOrCreateDirectory(const Path& path) {
            if (path.isEmpty())
                return *this;
            const String name = path.firstComponent().asString();
            DirMap::iterator it = m_directories.lower_bound(name);
            if (it == m_directories.end() || name != it->first)
                it = m_directories.insert(it, std::make_pair(name, new Directory(m_path + Path(name))));
            return it->second->findOrCreateDirectory(path.deleteFirstComponent());
        }
        
        PakFileSystemBase::PakFileSystemBase(const Path& path, MappedFile::Ptr file) :
        m_path(path),
        m_file(file),
        m_root(Path("")) {}
        
        
        PakFileSystemBase::~PakFileSystemBase() {}

        void PakFileSystemBase::initialize() {
            doReadDirectory();
        }
        
        Path PakFileSystemBase::doMakeAbsolute(const Path& relPath) const {
            return m_path + relPath.makeCanonical();
        }
        
        bool PakFileSystemBase::doDirectoryExists(const Path& path) const {
            const Path searchPath = path.makeLowerCase();
            return m_root.directoryExists(searchPath);
        }
        
        bool PakFileSystemBase::doFileExists(const Path& path) const {
            const Path searchPath = path.makeLowerCase();
            return m_root.fileExists(searchPath);
        }
        
        Path::List PakFileSystemBase::doGetDirectoryContents(const Path& path) const {
            const Path searchPath = path.makeLowerCase();
            const Directory& directory = m_root.findDirectory(path);
            return directory.contents();
        }
        
        const MappedFile::Ptr PakFileSystemBase::doOpenFile(const Path& path) const {
            const Path searchPath = path.makeLowerCase();
            return m_root.findFile(path);
        }
    }
}
