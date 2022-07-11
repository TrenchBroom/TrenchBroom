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

namespace TrenchBroom {
namespace IO {
Reader::Source::~Source() = default;

size_t Reader::Source::size() const {
  return doGetSize();
}

size_t Reader::Source::position() const {
  return doGetPosition();
}

bool Reader::Source::canRead(const size_t readSize) const {
  return position() + readSize <= size();
}

void Reader::Source::read(char* val, const size_t size) {
  ensurePosition(position() + size);
  doRead(val, size);
}

void Reader::Source::seek(const size_t position) {
  ensurePosition(position);
  doSeek(position);
}

void Reader::Source::ensurePosition(const size_t position) const {
  if (position > size()) {
    throw ReaderException(
      "Position " + std::to_string(position) + " is out of bounds for reader of size " +
      std::to_string(size()));
  }
}

std::unique_ptr<Reader::Source> Reader::Source::clone() const {
  return subSource(position(), size() - position());
}

std::unique_ptr<Reader::Source> Reader::Source::subSource(size_t position, size_t length) const {
  ensurePosition(position + length);
  return doGetSubSource(position, length);
}

std::unique_ptr<Reader::BufferSource> Reader::Source::buffer() const {
  return doBuffer();
}

Reader::FileSource::FileSource(std::FILE* file, const size_t offset, const size_t length)
  : m_file(file)
  , m_offset(offset)
  , m_length(length)
  , m_position(0) {
  assert(m_file != nullptr);
  std::rewind(m_file);
}

size_t Reader::FileSource::doGetSize() const {
  return m_length;
}

size_t Reader::FileSource::doGetPosition() const {
  return m_position;
}

void Reader::FileSource::doRead(char* val, const size_t size) {
  // We might consider removing this check under the assumption that the file position is set in the
  // constructor of this reader and that no other reader will access the file while this reader is
  // in use. This may be a reasonable assumption, since we usually read files one by one.

  const auto pos = std::ftell(m_file);
  if (pos < 0) {
    throwError("ftell failed");
  }
  if (static_cast<size_t>(pos) != m_offset + m_position) {
    if (std::fseek(m_file, static_cast<long>(m_offset + m_position), SEEK_SET) != 0) {
      throwError("fseek failed");
    }
  }
  if (std::fread(val, 1, size, m_file) != size) {
    throwError("fread failed");
  }
  m_position += size;
}

void Reader::FileSource::doSeek(const size_t position) {
  m_position = position;
}

std::unique_ptr<Reader::Source> Reader::FileSource::doGetSubSource(
  const size_t position, const size_t length) const {
  return std::make_unique<FileSource>(m_file, m_offset + position, length);
}

std::unique_ptr<Reader::BufferSource> Reader::FileSource::doBuffer() const {
  std::fseek(m_file, static_cast<long>(m_offset), SEEK_SET);

#if defined __APPLE__
  // AppleClang doesn't support std::shared_ptr<T[]> (new as of C++17)
  auto buffer = OwningBufferSource::BufferType{new char[m_length], std::default_delete<char[]>{}};
#else
  // G++ doesn't support using std::shared_ptr<T> to manage T[]
  auto buffer = std::shared_ptr<char[]>{new char[m_length]};
#endif

  const auto read = std::fread(buffer.get(), 1, m_length, m_file);
  if (read != m_length) {
    throwError("fread failed");
  }

  if (std::fseek(m_file, static_cast<long>(m_offset + m_position), SEEK_SET) != 0) {
    throwError("fseek failed");
  }

  const char* begin = buffer.get();
  const char* end = begin + m_length;
  return std::make_unique<OwningBufferSource>(std::move(buffer), begin, end);
}

void Reader::FileSource::throwError(const std::string& msg) const {
  if (std::feof(m_file)) {
    throw ReaderException(msg + ": unexpected end of file");
  } else {
    throw ReaderException(msg + ": " + std::strerror(errno));
  }
}

Reader::BufferSource::BufferSource(const char* begin, const char* end)
  : m_begin(begin)
  , m_end(end)
  , m_current(begin) {
  if (m_begin > m_end) {
    throw ReaderException("Invalid buffer");
  }
}

const char* Reader::BufferSource::begin() const {
  return m_begin;
}

const char* Reader::BufferSource::end() const {
  return m_end;
}

size_t Reader::BufferSource::doGetSize() const {
  return static_cast<size_t>(m_end - m_begin);
}

size_t Reader::BufferSource::doGetPosition() const {
  return static_cast<size_t>(m_current - m_begin);
}

void Reader::BufferSource::doRead(char* val, const size_t size) {
  std::memcpy(val, m_current, size);
  m_current += size;
}

void Reader::BufferSource::doSeek(const size_t position) {
  m_current = m_begin + position;
}

std::unique_ptr<Reader::Source> Reader::BufferSource::doGetSubSource(
  const size_t position, const size_t length) const {
  return std::make_unique<BufferSource>(m_begin + position, m_begin + position + length);
}

std::unique_ptr<Reader::BufferSource> Reader::BufferSource::doBuffer() const {
  return std::make_unique<BufferSource>(m_begin, m_end);
}

Reader::OwningBufferSource::OwningBufferSource(
  BufferType buffer, const char* begin, const char* end)
  : BufferSource{begin, end}
  , m_buffer{std::move(buffer)} {}

std::unique_ptr<Reader::BufferSource> Reader::OwningBufferSource::doBuffer() const {
  return std::make_unique<Reader::OwningBufferSource>(m_buffer, begin(), end());
}

Reader::Reader(std::unique_ptr<Source> source)
  : m_source(std::move(source)) {}

Reader& Reader::operator=(const Reader& other) {
  m_source = other.m_source->clone();
  return *this;
}

Reader::Reader(const Reader& other)
  : m_source{other.m_source->clone()} {}

Reader Reader::from(std::FILE* file) {
  return Reader(std::make_unique<FileSource>(file, 0, fileSize(file)));
}

Reader Reader::from(const char* begin, const char* end) {
  return Reader(std::make_unique<BufferSource>(begin, end));
}

size_t Reader::size() const {
  return m_source->size();
}

size_t Reader::position() const {
  return m_source->position();
}

bool Reader::eof() const {
  return position() == size();
}

void Reader::seekFromBegin(const size_t position) {
  m_source->seek(position);
}

void Reader::seekFromEnd(const size_t offset) {
  seekFromBegin(size() - offset);
}

void Reader::seekForward(const size_t offset) {
  seekFromBegin(position() + offset);
}

void Reader::seekBackward(const size_t offset) {
  if (offset > position()) {
    throw ReaderException(
      "Cannot seek beyond start of reader at position " + std::to_string(position()) +
      " with offset " + std::to_string(offset));
  }
  seekFromBegin(position() - offset);
}

Reader Reader::subReaderFromBegin(const size_t position, const size_t length) const {
  return Reader(m_source->subSource(position, length));
}

Reader Reader::subReaderFromBegin(const size_t position) const {
  return subReaderFromBegin(position, size() - position);
}

Reader Reader::subReaderFromCurrent(const size_t offset, const size_t length) const {
  return subReaderFromBegin(position() + offset, length);
}

Reader Reader::subReaderFromCurrent(const size_t length) const {
  return subReaderFromCurrent(0, length);
}

BufferedReader Reader::buffer() const {
  return BufferedReader{m_source->buffer()};
}

bool Reader::canRead(const size_t readSize) const {
  return m_source->canRead(readSize);
}

void Reader::read(unsigned char* val, const size_t size) {
  read(reinterpret_cast<char*>(val), size);
}

void Reader::read(char* val, const size_t size) {
  m_source->read(val, size);
}

std::string Reader::readString(const size_t size) {
  std::vector<char> buffer;
  buffer.resize(size + 1);
  buffer[size] = 0;
  read(buffer.data(), size);
  return std::string(buffer.data());
}

BufferedReader::BufferedReader(std::unique_ptr<BufferSource> source)
  : Reader{std::move(source)} {}

const char* BufferedReader::begin() const {
  // This cast is safe since this reader can only host a buffer source!
  const auto& bufferSource = *static_cast<const BufferSource*>(m_source.get());
  return bufferSource.begin();
}

const char* BufferedReader::end() const {
  // This cast is safe since this reader can only host a buffer source!
  const auto& bufferSource = *static_cast<const BufferSource*>(m_source.get());
  return bufferSource.end();
}

std::string_view BufferedReader::stringView() const {
  return std::string_view(begin(), size());
}
} // namespace IO
} // namespace TrenchBroom
