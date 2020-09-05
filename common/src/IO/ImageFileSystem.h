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

#ifndef ImageFileSystem_h
#define ImageFileSystem_h

#include "IO/FileSystem.h"
#include "IO/Path.h"

#include <kdl/string_compare.h>

#include <map>
#include <memory>

namespace TrenchBroom {
    namespace IO {
        class CFile;
        class File;

        class ImageFileSystemBase : public FileSystem {
        protected:
            class FileEntry {
            public:
                virtual ~FileEntry();

                std::shared_ptr<File> open() const;
            private:
                virtual std::shared_ptr<File> doOpen() const = 0;
            };

            class SimpleFileEntry : public FileEntry {
            private:
                std::shared_ptr<File> m_file;
            public:
                explicit SimpleFileEntry(std::shared_ptr<File> file);
            private:
                std::shared_ptr<File> doOpen() const override;
            };

            class CompressedFileEntry : public FileEntry {
            private:
                std::shared_ptr<File> m_file;
                const size_t m_uncompressedSize;
            public:
                CompressedFileEntry(std::shared_ptr<File> file, size_t uncompressedSize);
                ~CompressedFileEntry() override = default;
            private:
                std::shared_ptr<File> doOpen() const override;
                virtual std::unique_ptr<char[]> decompress(std::shared_ptr<File> file, size_t uncompressedSize) const = 0;
            };

            class Directory {
            private:
                using DirMap  = std::map<Path, std::unique_ptr<Directory>, Path::Less<kdl::ci::string_less>>;
                using FileMap = std::map<Path, std::unique_ptr<FileEntry>, Path::Less<kdl::ci::string_less>>;

                Path m_path;
                DirMap m_directories;
                FileMap m_files;
            public:
                explicit Directory(const Path& path);

                void addFile(const Path& path, std::shared_ptr<File> file);
                void addFile(const Path& path, std::unique_ptr<FileEntry> file);

                bool directoryExists(const Path& path) const;
                bool fileExists(const Path& path) const;

                const Directory& findDirectory(const Path& path) const;
                const FileEntry& findFile(const Path& path) const;
                std::vector<Path> contents() const;
            private:
                Directory& findOrCreateDirectory(const Path& path);
            };
        protected:
            Path m_path;
            Directory m_root;
        protected:
            ImageFileSystemBase(std::shared_ptr<FileSystem> next, const Path& path);
        public:
            ~ImageFileSystemBase() override;
        protected:
            void initialize();
        public:
            /**
             * Reload this file system.
             */
            void reload();
        private:
            bool doDirectoryExists(const Path& path) const override;
            bool doFileExists(const Path& path) const override;

            std::vector<Path> doGetDirectoryContents(const Path& path) const override;
            std::shared_ptr<File> doOpenFile(const Path& path) const override;
        private:
            virtual void doReadDirectory() = 0;
        };

        class ImageFileSystem : public ImageFileSystemBase {
        protected:
            std::shared_ptr<CFile> m_file;
        protected:
            ImageFileSystem(std::shared_ptr<FileSystem> next, const Path& path);
        };
    }
}

#endif /* ImageFileSystem_h */
