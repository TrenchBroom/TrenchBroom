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

#pragma once

#include "IO/Path.h"
#include "IO/Reader.h"

#include <cstdio>
#include <memory>

namespace TrenchBroom {
    namespace IO {
        /**
         * Represents an opened (logical) file. A logical file can be backed by a physical file on the disk, a memory
         * buffer, or a portion thereof. A special case is a file that is backed by a C++ object. These files are used
         * to insert information into the virtual file system.
         */
        class File {
        private:
            Path m_path;
        protected:
            /**
             * Creates a new file with the given path.
             *
             * @param path the path of the file
             */
            explicit File(const Path& path);
        public:
            virtual ~File();

            /**
             * Returns the path of this file.
             */
            const Path& path() const;

            /**
             * Returns a reader to access the contents of this file.
             */
            virtual Reader reader() const = 0;

            /**
             * Returns the size of this file in bytes.
             */
            virtual size_t size() const = 0;
        };

        /**
         * A file that is backed by a memory buffer. The file takes ownership of the buffer.
         */
        class OwningBufferFile : public File {
        private:
            std::unique_ptr<char[]> m_buffer;
            size_t m_size;
        public:
            /**
             * Creates a new file with the given path, buffer and size.
             *
             * @param path the path of the file
             * @param buffer the memory buffer
             * @param size the size of the file
             */
            OwningBufferFile(const Path& path, std::unique_ptr<char[]> buffer, size_t size);

            Reader reader() const override;
            size_t size() const override;
        };

        /**
         * A file that is backed by a memory buffer. The file does not take ownership of the buffer.
         */
        class NonOwningBufferFile : public File {
        private:
            const char* m_begin;
            const char* m_end;
        public:
            /**
             * Creates a new file with the given path and buffer.
             *
             * @param path the path of the file
             * @param begin the start of the memory buffer
             * @param end the end of the memory buffer (position after the last byte)
             *
             * @throw FileSystemException if the given buffer pointers are invalid
             */
            NonOwningBufferFile(const Path& path, const char* begin, const char* end);

            Reader reader() const override;
            size_t size() const override;
        };

        /**
         * A file that is backed by a physical file on the disk. The file is opened in the constructor and closed in
         * the destructor.
         */
        class CFile : public File {
        private:
            std::FILE* m_file;
            size_t m_size;
        public:
            /**
             * Creates a new file with the given path and opens the file for reading.
             *
             * @param path the path of the file
             *
             * @throw FileSystemException if the file cannot be opened
             */
            explicit CFile(const Path& path);
            ~CFile() override;

            Reader reader() const override;
            size_t size() const override;

            /**
             * Returns the underlying file.
             */
            std::FILE* file() const;
        };

        /**
         * A file that is backed by a portion of a physical file.
         */
        class FileView : public File {
        private:
            std::shared_ptr<File> m_file;
            size_t m_offset;
            size_t m_length;
        public:
            /**
             * Creates a new file with the given path, host file, offset and length.
             *
             * @param path the file path
             * @param file the host file that contains the data of this file
             * @param offset the offset into the host file
             * @param length the length of the portion of the host file
             */
            explicit FileView(const Path& path, std::shared_ptr<File> file, size_t offset, size_t length);

            Reader reader() const override;
            size_t size() const override;
        };

        // TODO: get rid of this, it's evil
        /**
         * A file that is backed by a C++ object. These kinds of files are used to insert custom objects into the virtual
         * filesystem. An example would be shader objects which are parsed by the shader file system.
         *
         * @tparam T the type of the object represented by this file
         */
        template <typename T>
        class ObjectFile : public File {
        private:
            T m_object;
        public:
            /**
             * Creates a new file with the given path and object.
             *
             * @tparam S the type of the given object, must be convertible to T
             * @param path the file path
             * @param object the object
             */
            template <typename S>
            ObjectFile(const Path& path, S&& object) :
            File(path),
            m_object(std::forward<S>(object)) {}

            Reader reader() const override {
                const auto addr = reinterpret_cast<const char*>(&m_object);
                return Reader::from(addr, addr + size());
            }

            size_t size() const override {
                return sizeof(m_object);
            }

            /**
             * Returns the object that backs this file.
             */
            const T& object() const {
                return m_object;
            }
        };
    }
}


#endif //TRENCHBROOM_FILE_H
