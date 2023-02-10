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

#include "WadFileSystem.h"

#include "IO/File.h"
#include "IO/Reader.h"
#include "Logger.h"

#include <kdl/string_format.h>
#include <kdl/string_utils.h>

namespace TrenchBroom
{
namespace IO
{
namespace WadLayout
{
static const size_t MinFileSize = 12;
static const size_t MagicOffset = 0;
static const size_t MagicSize = 4;
static const size_t NumEntriesAddress = 4;
static const size_t DirOffsetAddress = 8;
static const size_t DirEntryTypeOffset = 4;
static const size_t DirEntryNameOffset = 3;
static const size_t DirEntryNameSize = 16;
static const size_t DirEntrySize = 32;
// static const size_t PalLength             = 256;
// static const size_t TexWidthOffset        = 16;
// static const size_t TexDataOffset         = 24;
// static const size_t MaxTextureSize        = 1024;
} // namespace WadLayout

namespace WadEntryType
{
// static const char WEStatus    = 'B';
// static const char WEConsole   = 'C';
// static const char WEMip       = 'D';
// static const char WEPalette   = '@';
}

WadFileSystem::WadFileSystem(Path path, Logger& logger)
  : WadFileSystem{nullptr, std::move(path), logger}
{
}

WadFileSystem::WadFileSystem(std::shared_ptr<FileSystem> next, Path path, Logger& logger)
  : ImageFileSystem{std::move(next), std::move(path)}
  , m_logger{logger}
{
  initialize();
}

void WadFileSystem::doReadDirectory()
{
  auto reader = m_file->reader();
  if (reader.size() < WadLayout::MinFileSize)
  {
    throw FileSystemException{"File does not contain a directory."};
  }

  reader.seekFromBegin(WadLayout::MagicOffset);
  const auto magic = reader.readString(WadLayout::MagicSize);
  if (kdl::str_to_lower(magic) != "wad2" && kdl::str_to_lower(magic) != "wad3")
  {
    throw FileSystemException{"Unknown wad file type '" + magic + "'"};
  }

  reader.seekFromBegin(WadLayout::NumEntriesAddress);
  const auto entryCount = reader.readSize<int32_t>();

  if (reader.size() < WadLayout::MinFileSize + entryCount * WadLayout::DirEntrySize)
  {
    throw FileSystemException{"File does not contain a directory"};
  }

  reader.seekFromBegin(WadLayout::DirOffsetAddress);
  const auto directoryOffset = reader.readSize<int32_t>();

  if (m_file->size() < directoryOffset + entryCount * WadLayout::DirEntrySize)
  {
    throw FileSystemException{"File directory is out of bounds."};
  }

  reader.seekFromBegin(directoryOffset);
  for (size_t i = 0; i < entryCount; ++i)
  {
    const auto entryAddress = reader.readSize<int32_t>();
    const auto entrySize = reader.readSize<int32_t>();

    if (m_file->size() < entryAddress + entrySize)
    {
      throw FileSystemException{
        kdl::str_to_string("File entry at address ", entryAddress, " is out of bounds")};
    }

    reader.seekForward(WadLayout::DirEntryTypeOffset);
    const auto entryType = reader.readString(1);
    reader.seekForward(WadLayout::DirEntryNameOffset);
    const auto entryName = reader.readString(WadLayout::DirEntryNameSize);
    if (entryName.empty())
    {
      m_logger.warn() << "Skipping WAD file entry with empty name at address "
                      << entryAddress;
      continue;
    }

    const auto path = IO::Path{entryName}.addExtension(entryType);
    auto file = std::make_shared<FileView>(path, m_file, entryAddress, entrySize);
    addFile(path, [file = std::move(file)]() { return file; });
  }
}
} // namespace IO
} // namespace TrenchBroom
