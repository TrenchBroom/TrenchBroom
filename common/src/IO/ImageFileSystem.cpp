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

#include "ImageFileSystem.h"

#include "CollectionUtils.h"
#include "IO/DiskFileSystem.h"
#include "IO/IOUtils.h"

#include <cassert>
#include <memory>

namespace TrenchBroom {
    namespace IO {
        ImageFileSystemBase::File::~File() = default;

        MappedFile::Ptr ImageFileSystemBase::File::open() const {
            return doOpen();
        }

        ImageFileSystemBase::SimpleFile::SimpleFile(MappedFile::Ptr file) :
        m_file(file) {}
        
        MappedFile::Ptr ImageFileSystemBase::SimpleFile::doOpen() const {
            return m_file;
        }

        ImageFileSystemBase::LinkFile::LinkFile(const FileSystem& fs, const Path& path, const Path& resolvedPath) :
        m_fs(fs),
        m_path(path),
        m_resolvedPath(resolvedPath) {}

        MappedFile::Ptr ImageFileSystemBase::LinkFile::doOpen() const {
            const auto linkedFile = m_fs.openFile(m_resolvedPath);
            return std::make_shared<MappedFileView>(linkedFile, m_path, linkedFile->begin(), linkedFile->end());
        }

        ImageFileSystemBase::CompressedFile::CompressedFile(MappedFile::Ptr file, const size_t uncompressedSize) :
        m_file(file),
        m_uncompressedSize(uncompressedSize) {}

        MappedFile::Ptr ImageFileSystemBase::CompressedFile::doOpen() const {
            auto data = decompress(m_file, m_uncompressedSize);
            return MappedFile::Ptr(new MappedFileBuffer(m_file->path(), std::move(data), m_uncompressedSize));
        }

        ImageFileSystemBase::Directory::Directory(const Path& path) :
        m_path(path) {}
        
        void ImageFileSystemBase::Directory::addFile(const Path& path, MappedFile::Ptr file) {
            addFile(path, std::make_unique<SimpleFile>(file));
        }

        void ImageFileSystemBase::Directory::addFile(const Path& path, std::unique_ptr<File> file) {
            ensure(file != nullptr, "file is null");
            const auto filename = path.lastComponent();
            if (path.length() == 1) {
                // silently overwrite duplicates, the latest entries win
                MapUtils::insertOrReplace(m_files, filename, std::move(file));
            } else {
                auto& dir = findOrCreateDirectory(path.deleteLastComponent());
                dir.addFile(filename, std::move(file));
            }
        }
        
        bool ImageFileSystemBase::Directory::directoryExists(const Path& path) const {
            if (path.isEmpty()) {
                return true;
            }

            auto it = m_directories.find(path.firstComponent());
            if (it == std::end(m_directories)) {
                return false;
            } else {
                return it->second->directoryExists(path.deleteFirstComponent());
            }
        }
        
        bool ImageFileSystemBase::Directory::fileExists(const Path& path) const {
            if (path.length() == 1) {
                return m_files.count(path) > 0;
            }

            auto it = m_directories.find(path.firstComponent());
            if (it == std::end(m_directories)) {
                return false;
            } else {
                return it->second->fileExists(path.deleteFirstComponent());
            }
        }
        
        const ImageFileSystemBase::Directory& ImageFileSystemBase::Directory::findDirectory(const Path& path) const {
            if (path.isEmpty()) {
                return *this;
            }

            auto it = m_directories.find(path.firstComponent());
            if (it == std::end(m_directories)) {
                throw FileSystemException("Path does not exist: '" + (m_path + path).asString() + "'");
            } else {
                return it->second->findDirectory(path.deleteFirstComponent());
            }
        }
        
        const ImageFileSystemBase::File& ImageFileSystemBase::Directory::findFile(const Path& path) const {
            assert(!path.isEmpty());
            
            const auto name = path.firstComponent();
            if (path.length() == 1) {
                auto it = m_files.find(name);
                if (it == std::end(m_files)) {
                    throw FileSystemException("File not found: '" + (m_path + path).asString() + "'");
                } else {
                    return *it->second;
                }
            } else {
                auto it = m_directories.find(name);
                if (it == std::end(m_directories)) {
                    throw FileSystemException("File not found: '" + (m_path + path).asString() + "'");
                } else {
                    return it->second->findFile(path.deleteFirstComponent());
                }
            }
        }

        Path::List ImageFileSystemBase::Directory::contents() const {
            Path::List contents;
            
            for (const auto& entry : m_directories) {
                contents.push_back(Path(entry.first));
            }

            for (const auto& entry : m_files) {
                contents.push_back(Path(entry.first));
            }

            return contents;
        }
        
        ImageFileSystemBase::Directory& ImageFileSystemBase::Directory::findOrCreateDirectory(const Path& path) {
            if (path.isEmpty()) {
                return *this;
            }

            const auto name = path.firstComponent();
            auto it = m_directories.lower_bound(name);
            if (it == std::end(m_directories) || name != it->first) {
                it = m_directories.insert(it, std::make_pair(name, new Directory(m_path + Path(name))));
            }
            return it->second->findOrCreateDirectory(path.deleteFirstComponent());
        }

        ImageFileSystemBase::ImageFileSystemBase(const Path& path) :
        m_path(path),
        m_root(Path("")) {}


        ImageFileSystemBase::~ImageFileSystemBase() = default;

        void ImageFileSystemBase::initialize() {
            doReadDirectory();
        }

        Path ImageFileSystemBase::doMakeAbsolute(const Path& relPath) const {
            return m_path + relPath.makeCanonical();
        }
        
        bool ImageFileSystemBase::doDirectoryExists(const Path& path) const {
            const auto searchPath = path.makeLowerCase();
            return m_root.directoryExists(searchPath);
        }
        
        bool ImageFileSystemBase::doFileExists(const Path& path) const {
            const auto searchPath = path.makeLowerCase();
            return m_root.fileExists(searchPath);
        }
        
        Path::List ImageFileSystemBase::doGetDirectoryContents(const Path& path) const {
            const auto searchPath = path.makeLowerCase();
            const auto& directory = m_root.findDirectory(path);
            return directory.contents();
        }
        
        const MappedFile::Ptr ImageFileSystemBase::doOpenFile(const Path& path) const {
            const auto searchPath = path.makeLowerCase();
            return m_root.findFile(path).open();
        }

        ImageFileSystem::ImageFileSystem(const Path& path, MappedFile::Ptr file) :
        ImageFileSystemBase(path),
        m_file(file) {}

        ImageFileSystem::~ImageFileSystem() = default;
    }
}
