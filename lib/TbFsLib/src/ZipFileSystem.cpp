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

#include "fs/ZipFileSystem.h"

#include "fs/File.h"

#include "kd/result.h"

#include <fmt/format.h>
#include <fmt/std.h>
#include <miniz/miniz.h>

#include <memory>
#include <mutex>
#include <string>

namespace tb::fs
{

namespace
{

/**
 * Helper to get the filename of a file in the zip archive
 */
std::string filename(mz_zip_archive& archive, const mz_uint fileIndex)
{
  // nameLen includes space for the null-terminator byte
  const auto nameLen = mz_zip_reader_get_filename(&archive, fileIndex, nullptr, 0);
  if (nameLen == 0)
  {
    return "";
  }

  auto result = std::string{};
  result.resize(static_cast<size_t>(nameLen - 1));

  // NOTE: this will overwrite the std::string's null terminator, which is permitted in
  // C++17 and later
  mz_zip_reader_get_filename(&archive, fileIndex, result.data(), nameLen);

  return result;
}
} // namespace

struct ZipFileSystem::State
{
  mz_zip_archive archive;
  bool initialized = false;
  std::mutex mutex;
};

ZipFileSystem::ZipFileSystem(std::shared_ptr<CFile> file)
  : ImageFileSystem{std::move(file)}
  , m_state{std::make_unique<State>()}
{
  mz_zip_zero_struct(&m_state->archive);
}

ZipFileSystem::~ZipFileSystem()
{
  if (m_state->initialized)
  {
    mz_zip_reader_end(&m_state->archive);
  }
}

Result<void> ZipFileSystem::doReadDirectory()
{
  contract_assert(!m_state->initialized);

  if (
    mz_zip_reader_init_cfile(&m_state->archive, m_file->file(), m_file->size(), 0)
    != MZ_TRUE)
  {
    return Error{"Error calling mz_zip_reader_init_cfile"};
  }

  m_state->initialized = true;

  const auto numFiles = mz_zip_reader_get_num_files(&m_state->archive);
  for (mz_uint i = 0; i < numFiles; ++i)
  {
    if (!mz_zip_reader_is_file_a_directory(&m_state->archive, i))
    {
      const auto path = std::filesystem::path{filename(m_state->archive, i)};
      addFile(path, [&, i, path]() -> Result<std::shared_ptr<File>> {
        auto loadFileGoard = std::lock_guard{m_state->mutex};

        auto stat = mz_zip_archive_file_stat{};
        if (!mz_zip_reader_file_stat(&m_state->archive, i, &stat))
        {
          return Error{fmt::format("mz_zip_reader_file_stat failed for {}", path)};
        }

        const auto uncompressedSize = static_cast<size_t>(stat.m_uncomp_size);
        auto data = std::make_unique<char[]>(uncompressedSize);
        auto* begin = data.get();

        if (!mz_zip_reader_extract_to_mem(
              &m_state->archive, i, begin, uncompressedSize, 0))
        {
          return Error{fmt::format("mz_zip_reader_extract_to_mem failed for {}", path)};
        }

        return std::static_pointer_cast<File>(
          std::make_shared<OwningBufferFile>(std::move(data), uncompressedSize));
      });
    }
  }

  const auto err = mz_zip_get_last_error(&m_state->archive);
  if (err != MZ_ZIP_NO_ERROR)
  {
    return Error{
      std::string{"Error while reading compressed file: "}
      + mz_zip_get_error_string(err)};
  }

  return kdl::void_success;
}

} // namespace tb::fs
