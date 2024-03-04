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

#include "Error.h"

#include "kdl/result.h"

#include <cstdio>
#include <cstring>

namespace TrenchBroom::IO
{

File::File() = default;

File::~File() = default;

OwningBufferFile::OwningBufferFile(std::unique_ptr<char[]> buffer, const size_t size)
  : m_buffer{std::move(buffer)}
  , m_size{size}
{
}

Reader OwningBufferFile::reader() const
{
  return Reader::from(m_buffer.get(), m_buffer.get() + m_size);
}

size_t OwningBufferFile::size() const
{
  return m_size;
}

namespace
{
Result<kdl::resource<std::FILE*>> openPathAsFILE(
  const std::filesystem::path& path, const std::string& mode)
{
  // Windows: fopen() doesn't handle UTF-8. We have to use the nonstandard _wfopen
  // to open a Unicode path.
  //
  // All other platforms, just assume fopen() can handle UTF-8
  //
  // mode is assumed to be ASCII (one byte per char)
  auto* file =
#ifdef _WIN32
    _wfopen(path.wstring().c_str(), std::wstring{mode.begin(), mode.end()}.c_str());
#else
    fopen(path.string().c_str(), mode.c_str());
#endif

  if (!file)
  {
    return Error{"Cannot open file " + path.string()};
  }

  return kdl::resource{file, std::fclose};
}

Result<size_t> fileSize(std::FILE* file)
{
  const auto pos = std::ftell(file);
  if (pos < 0)
  {
    return Error{"ftell failed"};
  }

  if (std::fseek(file, 0, SEEK_END) != 0)
  {
    return Error{"fseek failed"};
  }

  const auto size = std::ftell(file);
  if (size < 0)
  {
    return Error{"ftell failed"};
  }

  if (std::fseek(file, pos, SEEK_SET) != 0)
  {
    return Error{"fseek failed"};
  }

  return static_cast<size_t>(size);
}
} // namespace

CFile::CFile(kdl::resource<std::FILE*> file, const size_t size)
  : m_file{std::move(file)}
  , m_size{size}
{
}

Reader CFile::reader() const
{
  return Reader::from(*this, m_size);
}

size_t CFile::size() const
{
  return m_size;
}

std::FILE* CFile::file() const
{
  return *m_file;
}

std::unique_ptr<OwningBufferFile> CFile::buffer() const
{
  if (std::fseek(file(), 0, SEEK_SET))
  {
    return nullptr;
  }

  auto buffer = std::make_unique<char[]>(size());
  if (std::fread(buffer.get(), 1, size(), file()) != size())
  {
    return nullptr;
  }

  return std::make_unique<OwningBufferFile>(std::move(buffer), size());
}

Result<void> CFile::read(char* val, const size_t position, const size_t size) const
{
  auto guard = std::lock_guard{m_mutex};

  const auto currentPosition = std::ftell(m_file.get());
  if (currentPosition < 0)
  {
    return makeError("ftell failed");
  }
  if (size_t(currentPosition) != position)
  {
    if (std::fseek(*m_file, long(position), SEEK_SET) != 0)
    {
      return makeError("fseek failed");
    }
  }
  if (std::fread(val, 1, size, *m_file) != size)
  {
    return makeError("fread failed");
  }

  return kdl::void_success;
}

Result<CFile::BufferType> CFile::buffer(const size_t position, const size_t size) const
{
#if defined __APPLE__
  // AppleClang doesn't support std::shared_ptr<T[]> (new as of C++17)
  auto buffer = BufferType{new char[size], std::default_delete<char[]>{}};
#else
  // G++ doesn't support using std::shared_ptr<T> to manage T[]
  auto buffer = std::shared_ptr<char[]>{new char[size]};
#endif

  return read(buffer.get(), position, size).transform([&]() {
    return std::move(buffer);
  });
}

Error CFile::makeError(const std::string& msg) const
{
  return std::feof(*m_file) ? Error{msg + ": unexpected end of file"}
                            : Error{msg + ": " + std::strerror(errno)};
}

Result<std::shared_ptr<CFile>> createCFile(const std::filesystem::path& path)
{
  return openPathAsFILE(path, "rb").and_then([](auto file) {
    return fileSize(*file).transform([&](auto size) {
      // NOLINTNEXTLINE
      return std::shared_ptr<CFile>{new CFile{std::move(file), size}};
    });
  });
}

FileView::FileView(std::shared_ptr<File> file, const size_t offset, const size_t length)
  : m_file{std::move(file)}
  , m_offset{offset}
  , m_length{length}
{
}

Reader FileView::reader() const
{
  return m_file->reader().subReaderFromBegin(m_offset, m_length);
}

size_t FileView::size() const
{
  return m_length;
}
} // namespace TrenchBroom::IO
