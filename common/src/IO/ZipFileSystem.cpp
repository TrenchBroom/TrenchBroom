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

#include "ZipFileSystem.h"

#include "CollectionUtils.h"
#include "IO/CharArrayReader.h"
#include "IO/DiskFileSystem.h"
#include "IO/IOUtils.h"

#include <miniz/miniz.h>

#include <cassert>
#include <cstring>
#include <memory>
#include <string>

namespace TrenchBroom {
    namespace IO {
        // MinizArchive

        class MinizArchive {
        public:
            mz_zip_archive archive;

            MinizArchive(const char* begin, const size_t size) {
                mz_zip_zero_struct(&archive);

                if (mz_zip_reader_init_mem(&archive, begin, size, 0) != MZ_TRUE) {
                    throw FileSystemException("Error calling mz_zip_reader_init_mem");
                }
            }

            virtual ~MinizArchive() {
                mz_zip_reader_end(&archive);
            }

            /**
             * Helper to get the filename of a file in the zip archive
             */
            std::string filename(const mz_uint fileIndex) {
                // nameLen includes space for the null-terminator byte
                const size_t nameLen = mz_zip_reader_get_filename(&archive, fileIndex, nullptr, 0);
                if (nameLen == 0) {
                    return "";
                }

                std::string result;
                result.resize(nameLen - 1);

                // NOTE: this will overwrite the std::string's null terminator, which is permitted in C++17 and later
                mz_zip_reader_get_filename(&archive, fileIndex, result.data(), nameLen);

                return result;
            }
        };

        // ZipFileSystem::ZipCompressedFile

        ZipFileSystem::ZipCompressedFile::ZipCompressedFile(std::shared_ptr<MinizArchive> archive, unsigned int fileIndex) :
        m_archive(std::move(archive)),
        m_fileIndex(fileIndex) {}

        MappedFile::Ptr ZipFileSystem::ZipCompressedFile::doOpen() const {
            const auto path = Path(m_archive->filename(m_fileIndex));

            mz_zip_archive_file_stat stat;
            if (!mz_zip_reader_file_stat(&m_archive->archive, m_fileIndex, &stat)) {
                throw FileSystemException("mz_zip_reader_file_stat failed for " + path.asString());
            }

            const auto uncompressedSize = stat.m_uncomp_size;
            auto data = std::make_unique<char[]>(uncompressedSize);
            auto* begin = data.get();

            if (!mz_zip_reader_extract_to_mem(&m_archive->archive, m_fileIndex, begin, uncompressedSize, 0)) {
                throw FileSystemException("mz_zip_reader_extract_to_mem failed for " + path.asString());
            }

            return std::make_shared<MappedFileBuffer>(path, std::move(data), uncompressedSize);
        }

        // ZipFileSystem

        ZipFileSystem::ZipFileSystem(const Path& path, MappedFile::Ptr file) :
        ZipFileSystem(nullptr, path, std::move(file)) {}

        ZipFileSystem::ZipFileSystem(std::shared_ptr<FileSystem> next, const Path& path, MappedFile::Ptr file) :
        ImageFileSystem(std::move(next), path, std::move(file)) {
            initialize();
        }

        void ZipFileSystem::doReadDirectory() {
            auto archive = std::make_shared<MinizArchive>(m_file->begin(), m_file->size());

            const mz_uint numFiles = mz_zip_reader_get_num_files(&archive->archive);
            for (mz_uint i = 0; i < numFiles; ++i) {
                if (!mz_zip_reader_is_file_a_directory(&archive->archive, i)) {
                    const auto path = Path(archive->filename(i));
                    m_root.addFile(path, std::make_unique<ZipCompressedFile>(archive, i));
                }
            }

            if (auto err = mz_zip_get_last_error(&archive->archive); err != MZ_ZIP_NO_ERROR) {
                throw FileSystemException(String("Error while reading compressed file: ") + mz_zip_get_error_string(err));
            }
        }
    }
}
