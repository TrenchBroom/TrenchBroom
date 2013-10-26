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

#include "PakFileSystem.h"

#include "CollectionUtils.h"
#include "IO/DiskFileSystem.h"
#include "IO/IOUtils.h"

#include <cassert>

namespace TrenchBroom {
    namespace IO {
        namespace PakLayout {
            static const size_t HeaderAddress     = 0x0;
            static const size_t HeaderMagicLength = 0x4;
            static const size_t EntryLength       = 0x40;
            static const size_t EntryNameLength   = 0x38;
            static const String HeaderMagic       = "PACK";
        }

        PakFileSystem::Directory::Directory(const Path& path) :
        m_path(path) {}

        void PakFileSystem::Directory::addFile(const Path& path, MappedFile::Ptr file) {
            if (path.length() == 1) {
                m_files[path.firstComponent().asString()] = file;
            } else {
                Directory& dir = findOrCreateDirectory(path.deleteLastComponent());
                dir.addFile(path.lastComponent().asString(), file);
            }
        }

        bool PakFileSystem::Directory::directoryExists(const Path& path) const {
            if (path.isEmpty())
                return true;
            DirMap::const_iterator it = m_directories.find(path.firstComponent().asString());
            if (it == m_directories.end())
                return false;
            return it->second.directoryExists(path.deleteFirstComponent());
        }
        
        bool PakFileSystem::Directory::fileExists(const Path& path) const {
            if (path.length() == 1)
                return m_files.count(path.asString()) > 0;
            DirMap::const_iterator it = m_directories.find(path.firstComponent().asString());
            if (it == m_directories.end())
                return false;
            return it->second.fileExists(path.deleteFirstComponent());
        }

        const PakFileSystem::Directory& PakFileSystem::Directory::findDirectory(const Path& path) const {
            if (path.isEmpty())
                return *this;
            DirMap::const_iterator it = m_directories.find(path.firstComponent().asString());
            if (it == m_directories.end())
                throw new FileSystemException("Path does not exist: '" + (m_path + path).asString() + "'");
            return it->second.findDirectory(path.deleteFirstComponent());
        }
        
        const MappedFile::Ptr PakFileSystem::Directory::findFile(const Path& path) const {
            assert(!path.isEmpty());
            
            const String name = path.firstComponent().asString();
            if (path.length() == 1) {
                FileMap::const_iterator it = m_files.find(name);
                if (it == m_files.end())
                    throw new FileSystemException("File does not exist: '" + (m_path + path).asString() + "'");
                return it->second;
            }
            DirMap::const_iterator it = m_directories.find(name);
            if (it == m_directories.end())
                throw new FileSystemException("File does not exist: '" + (m_path + path).asString() + "'");
            return it->second.findFile(path.deleteFirstComponent());
        }
        
        Path::List PakFileSystem::Directory::contents() const {
            Path::List contents;
            
            DirMap::const_iterator dIt, dEnd;
            for (dIt = m_directories.begin(), dEnd = m_directories.end(); dIt != dEnd; ++dIt)
                contents.push_back(Path(dIt->first));
            
            FileMap::const_iterator fIt, fEnd;
            for (fIt = m_files.begin(), fEnd = m_files.end(); fIt != fEnd; ++fIt)
                contents.push_back(Path(fIt->first));
            
            return contents;
        }

        PakFileSystem::Directory& PakFileSystem::Directory::findOrCreateDirectory(const Path& path) {
            if (path.isEmpty())
                return *this;
            const String name = path.firstComponent().asString();
            DirMap::iterator it = m_directories.lower_bound(name);
            if (it == m_directories.end() || name != it->first)
                it = m_directories.insert(it, std::make_pair(name, Directory(m_path + Path(name))));
            return it->second.findOrCreateDirectory(path.deleteFirstComponent());
        }

        PakFileSystem::PakFileSystem(const Path& path, MappedFile::Ptr file) :
        m_path(path),
        m_file(file),
        m_root(Path("")) {
            readDirectory();
        }
        

        void PakFileSystem::readDirectory() {
            char magic[PakLayout::HeaderMagicLength];
            char entryNameBuffer[PakLayout::EntryNameLength + 1];
            entryNameBuffer[PakLayout::EntryNameLength] = 0;
            
            const char* cursor = m_file->begin() + PakLayout::HeaderAddress;
            readBytes(cursor, magic, PakLayout::HeaderMagicLength);
            
            const size_t directoryAddress = readSize<int32_t>(cursor);
            const size_t directorySize = readSize<int32_t>(cursor);
            const size_t entryCount = directorySize / PakLayout::EntryLength;
            
            assert(m_file->begin() + directoryAddress + directorySize <= m_file->end());
            cursor = m_file->begin() + directoryAddress;
            
            for (size_t i = 0; i < entryCount; ++i) {
                readBytes(cursor, entryNameBuffer, PakLayout::EntryNameLength);
                const String entryName(entryNameBuffer);
                const size_t entryAddress = readSize<int32_t>(cursor);
                const size_t entryLength = readSize<int32_t>(cursor);
                assert(m_file->begin() + entryAddress + entryLength <= m_file->end());
                
                const char* entryBegin = m_file->begin() + entryAddress;
                const char* entryEnd = entryBegin + entryLength;
                const Path filePath(StringUtils::toLower(entryName));

                m_root.addFile(filePath, MappedFile::Ptr(new MappedFileView(entryBegin, entryEnd)));
            }
        }
        
        bool PakFileSystem::doDirectoryExists(const Path& path) const {
            const Path searchPath = path.makeLowerCase().makeCanonical();
            return m_root.directoryExists(searchPath);
        }
        
        bool PakFileSystem::doFileExists(const Path& path) const {
            const Path searchPath = path.makeLowerCase().makeCanonical();
            return m_root.fileExists(searchPath);
        }
        
        Path::List PakFileSystem::doGetDirectoryContents(const Path& path) const {
            const Path searchPath = path.makeLowerCase().makeCanonical();
            const Directory& directory = m_root.findDirectory(path);
            return directory.contents();
        }
        
        const MappedFile::Ptr PakFileSystem::doOpenFile(const Path& path) const {
            const Path searchPath = path.makeLowerCase().makeCanonical();
            return m_root.findFile(path);
        }
    }
}
