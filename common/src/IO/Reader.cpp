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

#include "Reader.h"

#include "IO/IOUtils.h"
#include "IO/ReaderException.h"

#include <cassert>
#include <cerrno>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

namespace TrenchBroom::IO
{


/**
 * Abstract base class for a reader source.
 */
class ReaderSource
{
public:
  virtual ~ReaderSource() = default;

  /**
   * Returns the size of this reader source.
   */
  virtual size_t size() const = 0;

  /**
   * Reads the given number of bytes and stores them in the memory region pointed to by
   * val.
   *
   * @param val the memory region to read the bytes into
   * @param size the number of bytes to read
   *
   * @throw ReaderException if the given number of bytes cannot be read
   */
  virtual void read(char* val, size_t position, size_t size) = 0;

  /**
   * Returns a source for a sub region of this reader source.
   *
   * @param offset theoffset of the sub region
   * @param length the length of the sub region
   * @return a reader source for the specified sub region
   *
   * @throw ReaderException if the given sub region is out of bounds or if reading fails
   */
  virtual std::shared_ptr<ReaderSource> subSource(size_t offset, size_t length) const = 0;

  /**
   * Ensures that the contents of this reader are buffered in memory and returns the
   * buffered memory region.
   *
   * If this reader source is already buffered in memory, then the returned region
   * pointers will just point to this source's memory buffer, and no additional memory
   * will be allocated.
   *
   * If this reader source is not already buffered in memory, then this method will
   * allocate a buffer to read the contents of this source into. The returned pointers
   * will point to the begin and end of that buffer, and the buffer itself will also be
   * returned.
   *
   * @return a tuple containing of two pointers, the first of which points to the
   * beginning of a memory region and the second of which points to its end, and
   * optionally a pointer to the newly allocated memory that holds the data
   *
   * @throw ReaderException if reading fails
   */
  virtual std::shared_ptr<BufferReaderSource> buffer() const = 0;
};

/**
 * A reader source that reads from a memory region. Does not take ownership of the
 * memory region and will not deallocate it.
 */
class BufferReaderSource : public ReaderSource
{
private:
  const char* m_begin;
  const char* m_end;

public:
  /**
   * Creates a new reader source for the given memory region.
   *
   * @param begin the beginning of the memory region
   * @param end the end of the memory region (as in, the position after the last byte),
   * must not be before the given beginning
   *
   * @throw ReaderException if the given memory region is invalid
   */
  BufferReaderSource(const char* begin, const char* end)
    : m_begin{begin}
    , m_end{end}
  {
    if (m_begin > m_end)
    {
      throw ReaderException{"Invalid buffer"};
    }
  }

  /**
   * Returns the beginning of the underlying memory region.
   */
  const char* begin() const { return m_begin; }

  /**
   * Returns the end of the underlying memory region.
   */
  const char* end() const { return m_end; }

  size_t size() const override { return size_t(m_end - m_begin); }

  void read(char* val, const size_t position, const size_t size) override
  {
    std::memcpy(val, m_begin + position, size);
  }

  std::shared_ptr<ReaderSource> subSource(
    const size_t offset, const size_t length) const override
  {
    return std::make_shared<BufferReaderSource>(
      m_begin + offset, m_begin + offset + length);
  }

  std::shared_ptr<BufferReaderSource> buffer() const override
  {
    return std::make_shared<BufferReaderSource>(m_begin, m_end);
  }
};

class OwningBufferReaderSource : public BufferReaderSource
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
  BufferType m_buffer;

public:
  OwningBufferReaderSource(BufferType buffer, const char* begin, const char* end)
    : BufferReaderSource{begin, end}
    , m_buffer{std::move(buffer)}
  {
  }

  std::shared_ptr<BufferReaderSource> buffer() const override
  {
    return std::make_shared<OwningBufferReaderSource>(m_buffer, begin(), end());
  }
};

/**
 * A reader source that reads directly from a file. Note that the seek position of the
 * underlying C file is kept in sync with this file source's position automatically,
 * that is, two readers can read from the same underlying file without causing problems.
 */
class FileReaderSource : public ReaderSource
{
private:
  std::FILE* m_file;
  size_t m_offset;
  size_t m_length;

public:
  /**
   * Creates a new reader source for the given underlying file at the given offset and
   * length.
   *
   * @param file the file
   * @param offset the offset into the file at which this reader source should begin
   * @param length the length of this reader source
   */
  FileReaderSource(std::FILE* file, const size_t offset, const size_t length)
    : m_file{file}
    , m_offset{offset}
    , m_length{length}
  {
    assert(m_file != nullptr);
    std::rewind(m_file);
  }

public:
  size_t size() const override { return m_length; }

  void read(char* val, const size_t position, const size_t size) override
  {
    const auto pos = std::ftell(m_file);
    if (pos < 0)
    {
      throwError("ftell failed");
    }
    if (size_t(pos) != m_offset + position)
    {
      if (std::fseek(m_file, long(m_offset + position), SEEK_SET) != 0)
      {
        throwError("fseek failed");
      }
    }
    if (std::fread(val, 1, size, m_file) != size)
    {
      throwError("fread failed");
    }
  }

  std::shared_ptr<ReaderSource> subSource(
    const size_t offset, const size_t length) const override
  {
    return std::make_shared<FileReaderSource>(m_file, m_offset + offset, length);
  }

  std::shared_ptr<BufferReaderSource> buffer() const override
  {
    std::fseek(m_file, long(m_offset), SEEK_SET);

#if defined __APPLE__
    // AppleClang doesn't support std::shared_ptr<T[]> (new as of C++17)
    auto buffer = OwningBufferReaderSource::BufferType{
      new char[m_length], std::default_delete<char[]> {}};
#else
    // G++ doesn't support using std::shared_ptr<T> to manage T[]
    auto buffer = std::shared_ptr<char[]>{new char[m_length]};
#endif

    const auto read = std::fread(buffer.get(), 1, m_length, m_file);
    if (read != m_length)
    {
      throwError("fread failed");
    }

    const char* begin = buffer.get();
    const char* end = begin + m_length;
    return std::make_shared<OwningBufferReaderSource>(std::move(buffer), begin, end);
  }

private:
  [[noreturn]] void throwError(const std::string& msg) const
  {
    if (std::feof(m_file))
    {
      throw ReaderException{msg + ": unexpected end of file"};
    }
    else
    {
      throw ReaderException{msg + ": " + std::strerror(errno)};
    }
  }
};

Reader::Reader(std::shared_ptr<ReaderSource> source)
  : m_source{std::move(source)}
  , m_position{0}
{
}

Reader::~Reader() = default;

Reader Reader::from(std::FILE* file)
{
  return Reader{std::make_shared<FileReaderSource>(file, 0, fileSize(file))};
}

Reader Reader::from(const char* begin, const char* end)
{
  return Reader{std::make_shared<BufferReaderSource>(begin, end)};
}

size_t Reader::size() const
{
  return m_source->size();
}

size_t Reader::position() const
{
  return m_position;
}

bool Reader::eof() const
{
  return position() == size();
}

void Reader::seekFromBegin(const size_t position)
{
  ensurePosition(position);
  m_position = position;
}

void Reader::seekFromEnd(const size_t offset)
{
  seekFromBegin(size() - offset);
}

void Reader::seekForward(const size_t offset)
{
  seekFromBegin(position() + offset);
}

void Reader::seekBackward(const size_t offset)
{
  if (offset > position())
  {
    throw ReaderException{
      "Cannot seek beyond start of reader at position " + std::to_string(position())
      + " with offset " + std::to_string(offset)};
  }
  seekFromBegin(position() - offset);
}

Reader Reader::subReaderFromBegin(const size_t position, const size_t length) const
{
  ensurePosition(position);
  return Reader{m_source->subSource(position, length)};
}

Reader Reader::subReaderFromBegin(const size_t position) const
{
  return subReaderFromBegin(position, size() - position);
}

Reader Reader::subReaderFromCurrent(const size_t offset, const size_t length) const
{
  return subReaderFromBegin(position() + offset, length);
}

Reader Reader::subReaderFromCurrent(const size_t length) const
{
  return subReaderFromCurrent(0, length);
}

BufferedReader Reader::buffer() const
{
  return BufferedReader{m_source->buffer()};
}

bool Reader::canRead(const size_t readSize) const
{
  return position() + readSize <= size();
}

void Reader::read(unsigned char* val, const size_t size)
{
  read(reinterpret_cast<char*>(val), size);
}

void Reader::read(char* val, const size_t size)
{
  ensurePosition(position() + size);
  m_source->read(val, position(), size);
  m_position += size;
}

std::string Reader::readString(const size_t size)
{
  auto buffer = std::vector<char>(size + 1, 0);
  read(buffer.data(), size);
  return {buffer.data()};
}

void Reader::ensurePosition(const size_t position) const
{
  if (position > size())
  {
    throw ReaderException{
      "Position " + std::to_string(position) + " is out of bounds for reader of size "
      + std::to_string(size())};
  }
}

BufferedReader::BufferedReader(std::shared_ptr<BufferReaderSource> source)
  : Reader{std::move(source)}
{
}

BufferedReader BufferedReader::buffer() const
{
  return *this;
}

const char* BufferedReader::begin() const
{
  // This cast is safe since this reader can only host a buffer source!
  const auto& bufferSource = *static_cast<const BufferReaderSource*>(m_source.get());
  return bufferSource.begin();
}

const char* BufferedReader::end() const
{
  // This cast is safe since this reader can only host a buffer source!
  const auto& bufferSource = *static_cast<const BufferReaderSource*>(m_source.get());
  return bufferSource.end();
}

std::string_view BufferedReader::stringView() const
{
  return std::string_view{begin(), size()};
}
} // namespace TrenchBroom::IO
