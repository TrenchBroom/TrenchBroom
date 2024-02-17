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

#include "vm/vec.h"

#include <cstdio>
#include <memory>
#include <string>
#include <string_view>

namespace TrenchBroom::IO
{
class BufferedReader;
class BufferReaderSource;
class CFile;
class ReaderSource;

/**
 * Accesses information from a stream of binary data. The underlying stream is represented
 * by a source, which can either be a file or a memory region. Allows reading and
 * converting data of various types for easier use.
 */
class Reader
{
protected:
  std::shared_ptr<ReaderSource> m_source;
  size_t m_position;

protected:
  /**
   * Creates a new reader using the given reader source.
   */
  explicit Reader(std::shared_ptr<ReaderSource> source);

public:
  virtual ~Reader();

  /**
   * Creates a new reader that reads from the given file.
   *
   * @param file the file to read from
   * @param size the file size
   * @return the reader
   *
   * @throw ReaderException if the reader cannot be created
   */
  static Reader from(const CFile& file, size_t size);

  /**
   * Creates a new reader that reads from the given memory region.
   *
   * @param begin the beginning of the memory region
   * @param end the end of the memory region (the position after the last byte)
   * @return the reader
   *
   * @throw ReaderException if the reader cannot be created
   */
  static Reader from(const char* begin, const char* end);

public:
  /**
   * Returns the size of the underlying reader source.
   */
  size_t size() const;

  /**
   * Returns the current position of the underlying reader source.
   */
  size_t position() const;

  /**
   * Indicates whether the end of the underlying reader source is reached.
   */
  bool eof() const;

  /**
   * Seeks to the given position relative to the start of the reader source.
   *
   * @throw ReaderException if the given position is out of bounds
   */
  void seekFromBegin(size_t position);

  /**
   * Seeks to the given position relative to the end of the reader source.
   *
   * @throw ReaderException if the given position is out of bounds
   */
  void seekFromEnd(size_t offset);

  /**
   * Forward seeks to the given position relative to the current position of the reader
   * source.
   *
   * @throw ReaderException if the given position is out of bounds
   */
  void seekForward(size_t offset);

  /**
   * Backward seeks to the given position relative to the current position of the reader
   * source.
   *
   * @throw ReaderException if the given position is out of bounds
   */
  void seekBackward(size_t offset);

  /**
   * Returns a reader for the given sub region of this reader's source.
   *
   * @param position the start position, relative to the start position of this reader
   * @param length the length of the sub region
   * @return the reader for the given sub region
   *
   * @throw ReaderException if the given sub region is out of bounds
   */
  Reader subReaderFromBegin(size_t position, size_t length) const;

  /**
   * Returns a reader for a sub region of this reader's source that starts at the given
   * position and ends at the end of the reader source.
   *
   * @param position the start position, relative to the start position of this reader
   * @return the reader for the given sub region
   *
   * @throw ReaderException if the given sub region is out of bounds
   */
  Reader subReaderFromBegin(size_t position) const;

  /**
   * Returns a reader for a sub region of this reader's source that starts at the current
   * position and that has the given length
   *
   * @param length the length of the sub region
   * @return the reader for the given sub region
   *
   * @throw ReaderException if the given sub region is out of bounds
   */
  Reader subReaderFromCurrent(size_t length) const;

  /**
   * Returns a reader for a sub region of this reader's source that starts at the given
   * offset to the current position and that has the given length
   *
   * @param offset the offset of the sub region, relative to the current position
   * @param length the length of the sub region
   * @return the reader for the given sub region
   *
   * @throw ReaderException if the given sub region is out of bounds
   */
  Reader subReaderFromCurrent(size_t offset, size_t length) const;

  /**
   * Buffers the contents of this reader's source if necessary and returns a buffered
   * reader that manages the buffered data and allows access to it.
   *
   * @return the buffered data
   *
   * @throw ReaderException if reading the data from the underlying reader source fails
   */
  virtual BufferedReader buffer() const;

  /**
   * Indicates whether the given number of bytes can be read from this reader.
   *
   * @param readSize the number of bytes to read
   * @return true if the given number of bytes can be read and false otherwise
   */
  bool canRead(size_t readSize) const;

  /**
   * Reads the given number of bytes into the given memory region.
   *
   * @param val the memory region to read into
   * @param size the number of bytes to read
   *
   * @throw ReaderException if reading fails
   */
  void read(unsigned char* val, size_t size);

  /**
   * Reads the given number of bytes into the given memory region.
   *
   * @param val the memory region to read into
   * @param size the number of bytes to read
   *
   * @throw ReaderException if reading fails
   */
  void read(char* val, size_t size);

  /**
   * Reads a value of the given type T, converts it into a value of the given type R and
   * returns that.
   *
   * @tparam T the type of the value to read, e.g. uint32_t
   * @tparam R the type of the value to convert to and return
   * @return the value
   *
   * @throw ReaderException if reading fails
   */
  template <typename T, typename R>
  R read()
  {
    T result;
    read(reinterpret_cast<char*>(&result), sizeof(T));
    return static_cast<R>(result);
  }

  /**
   * Reads a single signed char.
   *
   * @tparam T the type of the underlying value to read and convert to char
   * @return the char
   *
   * @throw ReaderException if reading fails
   */
  template <typename T>
  char readChar()
  {
    return read<T, char>();
  }

  /**
   * Reads a single unsigned char.
   *
   * @tparam T the type of the underlying value to read and convert to unsigned char
   * @return the char
   *
   * @throw ReaderException if reading fails
   */
  template <typename T>
  unsigned char readUnsignedChar()
  {
    return read<T, unsigned char>();
  }

  /**
   * Reads a value of the given type, converts it to int and returns that.
   *
   * @tparam T the type of the underlying value to read and convert to int
   * @return the int value
   *
   * @throw ReaderException if reading fails
   */
  template <typename T>
  int readInt()
  {
    return read<T, int>();
  }

  /**
   * Reads a value of the given type, converts it to unsigned int and returns that.
   *
   * @tparam T the type of the underlying value to read and convert to unsigned int
   * @return the unsigned int value
   *
   * @throw ReaderException if reading fails
   */
  template <typename T>
  unsigned int readUnsignedInt()
  {
    return read<T, unsigned int>();
  }

  /**
   * Reads a value of the given type, converts it to size_t and returns that.
   *
   * @tparam T the type of the underlying value to read and convert to size_t
   * @return the size_t value
   *
   * @throw ReaderException if reading fails
   */
  template <typename T>
  size_t readSize()
  {
    return read<T, size_t>();
  }

  /**
   * Reads a value of the given type, converts it to boolean and returns that.
   *
   * @tparam T the type of the underlying value to read and convert to boolean
   * @return the boolean value
   *
   * @throw ReaderException if reading fails
   */
  template <typename T>
  bool readBool()
  {
    return read<T, T>() != 0;
  }

  /**
   * Reads a value of the given type, converts it to 32bit float and returns that.
   *
   * @tparam T the type of the underlying value to read and convert to 32bit float
   * @return the 32bit float value
   *
   * @throw ReaderException if reading fails
   */
  template <typename T>
  float readFloat()
  {
    return read<T, float>();
  }

  /**
   * Reads a value of the given type, converts it to 64bit double and returns that.
   *
   * @tparam T the type of the underlying value to read and convert to 64bit double
   * @return the 64bit double value
   *
   * @throw ReaderException if reading fails
   */
  template <typename T>
  double readDouble()
  {
    return read<T, double>();
  }

  /**
   * Reads an ASCII string of the given length.
   *
   * @param size the number of characters to read
   * @return the string
   *
   * @throw ReaderException if reading fails
   */
  std::string readString(size_t size);

  template <typename R, size_t S, typename T = R>
  vm::vec<T, S> readVec()
  {
    vm::vec<T, S> result;
    for (size_t i = 0; i < S; ++i)
    {
      result[i] = read<T, R>();
    }
    return result;
  }

  /**
   * Reads values of the given type T, converts them to the given type R and stores them
   * in the given collection. The collection must support push_back.
   *
   * @tparam C the type of the collection
   * @tparam T the type of the value to read
   * @tparam R the type of the value to convert to
   * @param col the collection to read into
   * @param n the number of elements to read
   *
   * @throw ReaderException if reading fails
   */
  template <typename C, typename T, typename R>
  void read(C& col, const size_t n)
  {
    read<T, R>(std::back_inserter(col), n);
  }

  /**
   * Reads values of the given type T, converts them to the given type R and stores them
   * in the given output iterator.
   *
   * @tparam I the type of the given output iterator
   * @tparam T the type of the value to read
   * @tparam R the type of the value to convert to
   * @param out the output iterator to store values to
   * @param n the number of elements to read
   *
   * @throw ReaderException if reading fails
   */
  template <typename I, typename T, typename R>
  void read(I out, const size_t n)
  {
    for (size_t i = 0; i < n; ++i)
    {
      out += read<T, R>();
    }
  }

protected:
  void ensurePosition(size_t position) const;
};

/**
 * A special subtype of reader that can manage the lifetime of a region of memory. Such a
 * buffered reader will be created when calling the Reader::buffer() method.
 */
class BufferedReader : public Reader
{
protected:
  friend class Reader;

  /**
   * Creates a new buffered reader for the given memory region. If the given buffer is not
   * nullptr, it will be moved into this object and it will be destroyed when this object
   * is destroyed.
   */
  explicit BufferedReader(std::shared_ptr<BufferReaderSource> source);

public:
  BufferedReader buffer() const override;

  /**
   * Returns the beginning of the underlying buffer memory region.
   */
  const char* begin() const;

  /**
   * Returns the end of the underlying buffer memory region.
   */
  const char* end() const;
  /**
   * Returns a std::string_view view of the buffer.
   *
   * Caller's responsibility to ensure that the BufferedReader outlives the returned
   * std::string_view.
   */
  std::string_view stringView() const;
};
} // namespace TrenchBroom::IO
