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

#include "File.h"

#include "Exceptions.h"
#include "IO/IOUtils.h"

namespace TrenchBroom {
    namespace IO {
        File::File(const Path& path) :
        m_path(path) {}

        File::~File() = default;

        const Path& File::path() const {
            return m_path;
        }

        OwningBufferFile::OwningBufferFile(const Path& path, std::unique_ptr<char[]> buffer, const size_t size) :
        File(path),
        m_buffer(std::move(buffer)),
        m_size(size) {}

        Reader OwningBufferFile::reader() const {
            return Reader::from(m_buffer.get(), m_buffer.get() + m_size);
        }

        size_t OwningBufferFile::size() const {
            return m_size;
        }

        NonOwningBufferFile::NonOwningBufferFile(const Path& path, const char* begin, const char* end) :
        File(path),
        m_begin(begin),
        m_end(end) {
            if (m_end < m_begin) {
                throw FileSystemException("Invalid buffer");
            }
        }

        Reader NonOwningBufferFile::reader() const {
            return Reader::from(m_begin, m_end);
        }

        size_t NonOwningBufferFile::size() const {
            return static_cast<size_t>(m_end - m_begin);
        }

        CFile::CFile(const Path& path) :
        File(path) {
            m_file = openPathAsFILE(path, "rb");
            if (m_file == nullptr) {
                throw FileSystemException("Cannot open file " + path.asString());
            }
            m_size = fileSize(m_file);
        }

        CFile::~CFile() {
            if (m_file != nullptr) {
                std::fclose(m_file);
            }
        }

        Reader CFile::reader() const {
            return Reader::from(m_file);
        }

        size_t CFile::size() const {
            return m_size;
        }

        std::FILE* CFile::file() const {
            return m_file;
        }

        FileView::FileView(const Path& path, std::shared_ptr<File> file, const size_t offset, const size_t length) :
        File(path),
        m_file(std::move(file)),
        m_offset(offset),
        m_length(length) {}

        Reader FileView::reader() const {
            return m_file->reader().subReaderFromBegin(m_offset, m_length);
        }

        size_t FileView::size() const {
            return m_length;
        }
    }
}
