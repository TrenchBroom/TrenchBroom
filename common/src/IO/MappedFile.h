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

#ifndef TrenchBroom_MappedFile
#define TrenchBroom_MappedFile

#include "Exceptions.h"
#include "IO/Path.h"

#include <istream>
#include <memory>
#include <vector>

#ifdef _WIN32
// can't include Windows.h here
typedef void *HANDLE;
typedef unsigned long DWORD;
#endif

namespace TrenchBroom {
    namespace IO {
        /**
         A memory mapped file. There different subclasses of this class represent different ways of accessing the file or
         portions thereof.
         */
        class MappedFile {
        public:
            using Ptr = std::shared_ptr<MappedFile>;
            using List = std::vector<Ptr>;
        private:
            Path m_path;
        protected:
            const char* m_begin;
            const char* m_end;
        public:
            /**
             Creates a new memory mapped file at the given path.

             @param path the path to the file
             */
            MappedFile(const Path& path);
            virtual ~MappedFile();

            /**
             Returns the file path.

             @return the file path
             */
            const Path& path() const;

            /**
             Returns the file size.

             @return the file size
             */
            size_t size() const;

            /**
             Returns a pointer to the beginning of the file.

             @return a pointer to the beginning of the file
             */
            const char* begin() const;

            /**
             Returns a pointer to the end of the file.

             @return a pointer to the end of the file (exclusive)
             */
            const char* end() const;
        protected:
            /**
             Initializes this file with the given pointers to the beginning and end of the file. The size
             of the file is computed from the distance of the two pointers.

             @param begin the pointer to the beginning of the file
             @param end the pointer to the end of the file (exclusive)
             */
            void init(const char* begin, const char* end);
        };

        /**
         A file that acts as a view to a range in memory.
         */
        class MappedFileBufferView : public MappedFile {
        public:
            /**
             Creates a new view of the given range in memory.

             @param path the path of the file
             @param begin the pointer to the beginning of the view
             @param end the pointer to the end of the view (exclusive)
             */
            MappedFileBufferView(const Path& path, const char* begin, const char* end);
            MappedFileBufferView(const Path& path, const char* begin, size_t size);
        };

        /**
         A file that acts as a view to a file within a container file.
         */
        class MappedFileView : public MappedFileBufferView {
        private:
            MappedFile::Ptr m_container;
        public:
            /**
             Creates a view to a file within the given file.

             @param container the container file that contains the file
             @param path the path of the file at the subrange
             @param begin the pointer to the beginning of the subrange
             @param end the pointer to the end of the file (exclusive)
             */
            MappedFileView(MappedFile::Ptr container, const Path& path, const char* begin, const char* end);

            /**
             Creates a view to a file within the given file.

             @param container the container file that contains the file
             @param path the path of the file at the subrange
             @param begin the pointer to the beginning of the subrange
             @param size the size of the subrange
             */
            MappedFileView(MappedFile::Ptr container, const Path& path, const char* begin, size_t size);
        };

        /**
         Creates a new file that is not necessarily a mapped file at all. Rather, this class owns the range of
         memory that it represents and frees it when it is destroyed.
         */
        class MappedFileBuffer : public MappedFileBufferView {
        private:
            std::unique_ptr<char[]> m_buffer;
        public:
            /**
             Creates a new file with the given path, memory buffer and size.

             @param path the file path
             @param buffer the memory buffer
             @param size the size of the memory buffer
             */
            MappedFileBuffer(const Path& path, std::unique_ptr<char[]> buffer, size_t size);
        };

        /**
         A file that represents a C++ object. Calling the begin() and end() methods on an object file will return
         null pointers! Use the object() method to access the object.

         @tparam T the type of the object
         */
        template <typename T>
        class ObjectFile : public MappedFile {
        private:
            T m_object;
        public:
            /**
             Creates a new object file. The given file is moved into this object.

             @param object the C++ object that is represented by this file
             @param path the file path
             */
            template <typename S>
            ObjectFile(S&& object, const Path& path) :
            MappedFile(path),
            m_object(std::forward<S>(object)) {}

            const T& object() const {
                return m_object;
            }
        };


        /**
         Opens the file at the given path and returns a memory mapped file for it. If the file is opened for reading,
         it is cached across multiple calls of this function, but only as long as the file is still open when it is
         opened again. Once the last pointer to a file goes out of scope, it is closed and unloaded.

         @param path the path to the file
         @param mode the access mode
         @return a shared pointer to the memory mapped file
         @throws FileSystemException if the file cannot be opened or mapped into memory
         */
        MappedFile::Ptr openMappedFile(const Path& path, std::ios_base::openmode mode);

#ifdef _WIN32
        class WinMappedFile : public MappedFile {
        private:
            HANDLE m_fileHandle;
            HANDLE m_mappingHandle;
            char* m_address;
        public:
            WinMappedFile(const Path& path, std::ios_base::openmode mode);
            ~WinMappedFile();
        private:
            static void throwError(const Path& path, const String& functionName);
        };
#else
        class PosixMappedFile : public MappedFile {
        private:
            char* m_address;
            size_t m_size;
            int m_filedesc;
        public:
            PosixMappedFile(const Path& path, std::ios_base::openmode mode);
            ~PosixMappedFile();
        };
#endif
    }
}

#endif /* defined(TrenchBroom_MappedFile) */
