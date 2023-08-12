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

#include "IdPakFileSystem.h"

#include "IO/DiskFileSystem.h"
#include "IO/File.h"
#include "IO/Reader.h"

#include <kdl/string_format.h>

#include <memory>
#include <string>

namespace TrenchBroom
{
namespace IO
{
namespace PakLayout
{
static const size_t HeaderAddress = 0x0;
static const size_t HeaderMagicLength = 0x4;
static const size_t EntryLength = 0x40;
static const size_t EntryNameLength = 0x38;
static const std::string HeaderMagic = "PACK";
} // namespace PakLayout

IdPakFileSystem::IdPakFileSystem(std::filesystem::path path)
  : ImageFileSystem{std::move(path)}
{
  initialize();
}

void IdPakFileSystem::doReadDirectory()
{
  char magic[PakLayout::HeaderMagicLength];

  auto reader = m_file->reader();
  reader.seekForward(PakLayout::HeaderAddress);
  reader.read(magic, PakLayout::HeaderMagicLength);

  const auto directoryAddress = reader.readSize<int32_t>();
  const auto directorySize = reader.readSize<int32_t>();
  const auto entryCount = directorySize / PakLayout::EntryLength;

  reader.seekFromBegin(directoryAddress);

  for (size_t i = 0; i < entryCount; ++i)
  {
    const auto entryName = reader.readString(PakLayout::EntryNameLength);
    const auto entryAddress = reader.readSize<int32_t>();
    const auto entrySize = reader.readSize<int32_t>();

    const auto entryPath = std::filesystem::path{kdl::str_to_lower(entryName)};
    auto entryFile = std::make_shared<FileView>(m_file, entryAddress, entrySize);
    addFile(entryPath, [entryFile = std::move(entryFile)]() { return entryFile; });
  }
}
} // namespace IO
} // namespace TrenchBroom
