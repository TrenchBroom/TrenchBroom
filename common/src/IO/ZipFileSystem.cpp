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

#include "ZipFileSystem.h"

#include "IO/DiskFileSystem.h"
#include "IO/File.h"

#include <memory>
#include <string>

namespace TrenchBroom
{
namespace IO
{
// ZipFileSystem

ZipFileSystem::ZipFileSystem(Path path)
  : ZipFileSystem{nullptr, std::move(path)}
{
}

ZipFileSystem::ZipFileSystem(std::shared_ptr<FileSystem> next, Path path)
  : ImageFileSystem{std::move(next), std::move(path)}
{
  initialize();
}

ZipFileSystem::~ZipFileSystem()
{
  mz_zip_reader_end(&m_archive);
}

void ZipFileSystem::doReadDirectory()
{
  mz_zip_zero_struct(&m_archive);

  if (mz_zip_reader_init_cfile(&m_archive, m_file->file(), m_file->size(), 0) != MZ_TRUE)
  {
    throw FileSystemException{"Error calling mz_zip_reader_init_cfile"};
  }

  const auto numFiles = mz_zip_reader_get_num_files(&m_archive);
  for (mz_uint i = 0; i < numFiles; ++i)
  {
    if (!mz_zip_reader_is_file_a_directory(&m_archive, i))
    {
      const auto path = Path{filename(i)};
      addFile(path, [=]() -> std::shared_ptr<File> {
        auto stat = mz_zip_archive_file_stat{};
        if (!mz_zip_reader_file_stat(&m_archive, i, &stat))
        {
          throw FileSystemException{
            "mz_zip_reader_file_stat failed for " + path.asString()};
        }

        const auto uncompressedSize = static_cast<size_t>(stat.m_uncomp_size);
        auto data = std::make_unique<char[]>(uncompressedSize);
        auto* begin = data.get();

        if (!mz_zip_reader_extract_to_mem(&m_archive, i, begin, uncompressedSize, 0))
        {
          throw FileSystemException{
            "mz_zip_reader_extract_to_mem failed for " + path.asString()};
        }

        return std::make_shared<OwningBufferFile>(
          path, std::move(data), uncompressedSize);
      });
    }
  }

  const auto err = mz_zip_get_last_error(&m_archive);
  if (err != MZ_ZIP_NO_ERROR)
  {
    throw FileSystemException{
      std::string{"Error while reading compressed file: "}
      + mz_zip_get_error_string(err)};
  }
}

/**
 * Helper to get the filename of a file in the zip archive
 */
std::string ZipFileSystem::filename(const mz_uint fileIndex)
{
  // nameLen includes space for the null-terminator byte
  const auto nameLen = mz_zip_reader_get_filename(&m_archive, fileIndex, nullptr, 0);
  if (nameLen == 0)
  {
    return "";
  }

  auto result = std::string{};
  result.resize(static_cast<size_t>(nameLen - 1));

  // NOTE: this will overwrite the std::string's null terminator, which is permitted in
  // C++17 and later
  mz_zip_reader_get_filename(&m_archive, fileIndex, result.data(), nameLen);

  return result;
}
} // namespace IO
} // namespace TrenchBroom
