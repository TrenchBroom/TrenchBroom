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

#ifndef TRENCHBROOM_FILE_H
#define TRENCHBROOM_FILE_H

#include "IO/Path.h"
#include "IO/Reader.h"

#include <cstdio>
#include <memory>

namespace TrenchBroom {
    namespace IO {
        class Reader;

        class File {
        private:
            Path m_path;
        protected:
            explicit File(const Path& path);
        public:
            virtual ~File();

            const Path& path() const;
            virtual Reader reader() const = 0;
            virtual size_t size() const = 0;
        };

        class OwningBufferFile : public File {
        private:
            std::unique_ptr<char[]> m_buffer;
            size_t m_size;
        public:
            OwningBufferFile(const Path& path, std::unique_ptr<char[]> buffer, size_t size);

            Reader reader() const override;
            size_t size() const override;
        };

        class NonOwningBufferFile : public File {
        private:
            const char* m_begin;
            const char* m_end;
        public:
            NonOwningBufferFile(const Path& path, const char* begin, const char* end);

            Reader reader() const override;
            size_t size() const override;
        };

        class CFile : public File {
        private:
            std::FILE* m_file;
            size_t m_size;
        public:
            explicit CFile(const Path& path);
            ~CFile() override;

            Reader reader() const override;
            size_t size() const override;

            std::FILE* file() const;
        };

        class FileView : public File {
        private:
            std::shared_ptr<File> m_file;
            size_t m_offset;
            size_t m_length;
        public:
            explicit FileView(const Path& path, std::shared_ptr<File> file, size_t offset, size_t length);

            Reader reader() const override;
            size_t size() const override;
        };

        // TODO: get rid of this, it's evil
        template <typename T>
        class ObjectFile : public File {
        private:
            T m_object;
        public:
            template <typename S>
            ObjectFile(const Path& path, S&& object) :
            File(path),
            m_object(std::forward<S>(object)) {}

            const T& object() const {
                return m_object;
            }

            Reader reader() const override {
                const auto addr = reinterpret_cast<const char*>(&m_object);
                return Reader::from(addr, addr + size());
            }

            size_t size() const override {
                return sizeof(m_object);
            }
        };
    }
}


#endif //TRENCHBROOM_FILE_H
