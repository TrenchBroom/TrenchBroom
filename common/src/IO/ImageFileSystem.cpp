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

#include "Ensure.h"
#include "IO/DiskFileSystem.h"
#include "IO/File.h"

#include <cassert>
#include <memory>

namespace TrenchBroom {
    namespace IO {
        ImageFileSystemBase::FileEntry::~FileEntry() = default;

       std::shared_ptr<File> ImageFileSystemBase::FileEntry::open() const {
            return doOpen();
        }

        ImageFileSystemBase::SimpleFileEntry::SimpleFileEntry(std::shared_ptr<File> file) :
        m_file(std::move(file)) {}

        std::shared_ptr<File> ImageFileSystemBase::SimpleFileEntry::doOpen() const {
            return m_file;
        }

        ImageFileSystemBase::CompressedFileEntry::CompressedFileEntry(std::shared_ptr<File> file, const size_t uncompressedSize) :
        m_file(file),
        m_uncompressedSize(uncompressedSize) {}

        std::shared_ptr<File> ImageFileSystemBase::CompressedFileEntry::doOpen() const {
            auto data = decompress(m_file, m_uncompressedSize);
            return std::make_shared<OwningBufferFile>(m_file->path(), std::move(data), m_uncompressedSize);
        }

        ImageFileSystemBase::Directory::Directory(const Path& path) :
        m_path(path) {}

        void ImageFileSystemBase::Directory::addFile(const Path& path, std::shared_ptr<File> file) {
            addFile(path, std::make_unique<SimpleFileEntry>(file));
        }

        void ImageFileSystemBase::Directory::addFile(const Path& path, std::unique_ptr<FileEntry> file) {
            ensure(file != nullptr, "file is null");
            const auto filename = path.lastComponent();
            if (path.length() == 1) {
                // silently overwrite duplicates, the latest entries win
                m_files[filename] = std::move(file);
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

        const ImageFileSystemBase::FileEntry& ImageFileSystemBase::Directory::findFile(const Path& path) const {
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

        std::vector<Path> ImageFileSystemBase::Directory::contents() const {
            std::vector<Path> contents;

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

        ImageFileSystemBase::ImageFileSystemBase(std::shared_ptr<FileSystem> next, const Path& path) :
        FileSystem(std::move(next)),
        m_path(path),
        m_root(Path()) {}


        ImageFileSystemBase::~ImageFileSystemBase() = default;

        void ImageFileSystemBase::initialize() {
            try {
                doReadDirectory();
            } catch (const std::exception& e) {
                throw FileSystemException("Could not initialize image file system '" + m_path.asString() + "': " + e.what());
            }
        }

        void ImageFileSystemBase::reload() {
            m_root = Directory(Path());
            initialize();
        }

        bool ImageFileSystemBase::doDirectoryExists(const Path& path) const {
            const auto searchPath = path.makeLowerCase().makeCanonical();
            return m_root.directoryExists(searchPath);
        }

        bool ImageFileSystemBase::doFileExists(const Path& path) const {
            const auto searchPath = path.makeLowerCase().makeCanonical();
            return m_root.fileExists(searchPath);
        }

        std::vector<Path> ImageFileSystemBase::doGetDirectoryContents(const Path& path) const {
            const auto searchPath = path.makeLowerCase().makeCanonical();
            const auto& directory = m_root.findDirectory(path);
            return directory.contents();
        }

        std::shared_ptr<File> ImageFileSystemBase::doOpenFile(const Path& path) const {
            const auto searchPath = path.makeLowerCase().makeCanonical();
            return m_root.findFile(path).open();
        }

        ImageFileSystem::ImageFileSystem(std::shared_ptr<FileSystem> next, const Path& path) :
        ImageFileSystemBase(std::move(next), path),
        m_file(std::make_shared<CFile>(path)) {
            ensure(m_path.isAbsolute(), "path must be absolute");
        }
    }
}
