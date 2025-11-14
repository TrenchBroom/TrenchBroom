/*
 Copyright (C) 2010 Kristian Duske

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

#include "Result.h"
#include "io/Reader.h"

#include "kdl/resource.h"

#include <cstdio>
#include <filesystem>
#include <memory>
#include <mutex>

namespace tb::io
{
/**
 * Represents an opened (logical) file. A logical file can be backed by a physical file on
 * the disk, a memory buffer, or a portion thereof.
 */
class File
{
protected:
  File();

public:
  virtual ~File();

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
class OwningBufferFile : public File
{
private:
  std::unique_ptr<char[]> m_buffer;
  size_t m_size;

public:
  /**
   * Creates a new file with the given buffer and size.
   *
   * @param buffer the memory buffer
   * @param size the size of the file
   */
  OwningBufferFile(std::unique_ptr<char[]> buffer, size_t size);

  Reader reader() const override;
  size_t size() const override;
};

/**
 * A file that is backed by a physical file on the disk. The file is opened in the
 * constructor and closed in the destructor.
 */
class CFile : public File
{
public:
#if defined __APPLE__
  // AppleClang doesn't support std::shared_ptr<T[]> (new as of C++17)
  using BufferType = std::shared_ptr<char>;
#else
  // G++ doesn't support using std::shared_ptr<T> to manage T[]
  using BufferType = std::shared_ptr<char[]>;
#endif
private:
  kdl::resource<std::FILE*> m_file;
  size_t m_size;
  mutable std::mutex m_mutex;

  /**
   * Creates a new file with the given file ptr and size in bytes.
   */
  CFile(kdl::resource<std::FILE*> file, size_t size);

public:
  friend Result<std::shared_ptr<CFile>> createCFile(const std::filesystem::path& path);

  Reader reader() const override;
  size_t size() const override;

  /**
   * Returns the underlying file.
   */
  std::FILE* file() const;

  std::unique_ptr<OwningBufferFile> buffer() const;

private:
  friend class FileReaderSource;

  Result<void> read(char* val, size_t position, size_t size) const;
  Result<BufferType> buffer(size_t position, size_t size) const;

  Error makeError(const std::string& msg) const;
};

Result<std::shared_ptr<CFile>> createCFile(const std::filesystem::path& path);

/**
 * A file that is backed by a portion of a physical file.
 */
class FileView : public File
{
private:
  std::shared_ptr<File> m_file;
  size_t m_offset;
  size_t m_length;

public:
  /**
   * Creates a new file with the given host file, offset and length.
   *
   * @param file the host file that contains the data of this file
   * @param offset the offset into the host file
   * @param length the length of the portion of the host file
   */
  explicit FileView(std::shared_ptr<File> file, size_t offset, size_t length);

  Reader reader() const override;
  size_t size() const override;
};

} // namespace tb::io
