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
#include "IO/PathQt.h"

#include <cstdio> // for FILE

namespace TrenchBroom
{
namespace IO
{
File::File(std::filesystem::path path)
  : m_path{std::move(path)}
{
}

File::~File() = default;

const std::filesystem::path& File::path() const
{
  return m_path;
}

OwningBufferFile::OwningBufferFile(
  std::filesystem::path path, std::unique_ptr<char[]> buffer, const size_t size)
  : File{std::move(path)}
  , m_buffer{std::move(buffer)}
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

NonOwningBufferFile::NonOwningBufferFile(
  std::filesystem::path path, const char* begin, const char* end)
  : File{std::move(path)}
  , m_begin{begin}
  , m_end{end}
{
  if (m_end < m_begin)
  {
    throw FileSystemException("Invalid buffer");
  }
}

Reader NonOwningBufferFile::reader() const
{
  return Reader::from(m_begin, m_end);
}

size_t NonOwningBufferFile::size() const
{
  return static_cast<size_t>(m_end - m_begin);
}

namespace
{
auto openPathAsFILE(const std::filesystem::path& path, const std::string& mode)
{
  // Windows: fopen() doesn't handle UTF-8. We have to use the nonstandard _wfopen
  // to open a Unicode path.
  //
  // All other platforms, just assume fopen() can handle UTF-8
  auto* file =
#ifdef _WIN32
    _wfopen(path.wstring().c_str(), QString::fromStdString(mode).toStdWString().c_str());
#else
    fopen(path.u8string().c_str(), mode.c_str());
#endif

  if (!file)
  {
    throw FileSystemException{"Cannot open file " + path.string()};
  }

  return std::unique_ptr<std::FILE, int (*)(std::FILE*)>{file, std::fclose};
}

size_t fileSize(std::FILE* file)
{
  const auto pos = std::ftell(file);
  if (pos < 0)
  {
    throw FileSystemException("ftell failed");
  }

  if (std::fseek(file, 0, SEEK_END) != 0)
  {
    throw FileSystemException("fseek failed");
  }

  const auto size = std::ftell(file);
  if (size < 0)
  {
    throw FileSystemException("ftell failed");
  }

  if (std::fseek(file, pos, SEEK_SET) != 0)
  {
    throw FileSystemException("fseek failed");
  }

  return static_cast<size_t>(size);
}
} // namespace

CFile::CFile(std::filesystem::path path)
  : File{std::move(path)}
  , m_file{openPathAsFILE(this->path(), "rb")}
  , m_size{fileSize(m_file.get())}
{
}

Reader CFile::reader() const
{
  return Reader::from(m_file.get(), m_size);
}

size_t CFile::size() const
{
  return m_size;
}

std::FILE* CFile::file() const
{
  return m_file.get();
}

FileView::FileView(
  std::filesystem::path path,
  std::shared_ptr<File> file,
  const size_t offset,
  const size_t length)
  : File{std::move(path)}
  , m_file{std::move(file)}
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
} // namespace IO
} // namespace TrenchBroom
