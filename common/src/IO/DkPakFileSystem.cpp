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

#include "DkPakFileSystem.h"

#include "Error.h"
#include "IO/DiskFileSystem.h"
#include "IO/File.h"
#include "IO/ReaderException.h"

#include <kdl/result.h>
#include <kdl/string_format.h>

#include <cassert>
#include <cstring>
#include <memory>

namespace TrenchBroom::IO
{
namespace DkPakLayout
{
static const size_t HeaderMagicLength = 0x4;
static const size_t EntryLength = 0x48;
static const size_t EntryNameLength = 0x38;
static const std::string HeaderMagic = "PACK";
} // namespace DkPakLayout

namespace
{
Result<std::unique_ptr<char[]>> decompress(
  std::shared_ptr<File> file, const size_t uncompressedSize)
{
  try
  {
    auto reader = file->reader().buffer();

    auto result = std::make_unique<char[]>(uncompressedSize);
    auto* begin = result.get();
    auto* curTarget = begin;

    auto x = reader.readUnsignedChar<unsigned char>();
    while (!reader.eof() && x < 0xFF)
    {
      if (x < 0x40)
      {
        // x+1 bytes of uncompressed data follow (just read+write them as they are)
        const auto len = static_cast<size_t>(x) + 1;
        reader.read(curTarget, len);
        curTarget += len;
      }
      else if (x < 0x80)
      {
        // run-length encoded zeros, write (x - 62) zero-bytes to output
        const auto len = static_cast<size_t>(x) - 62;
        std::memset(curTarget, 0, len);
        curTarget += len;
      }
      else if (x < 0xC0)
      {
        // run-length encoded data, read one byte, write it (x-126) times to output
        const auto len = static_cast<size_t>(x) - 126;
        const auto data = reader.readInt<unsigned char>();
        std::memset(curTarget, data, len);
        curTarget += len;
      }
      else if (x < 0xFE)
      {
        // this references previously uncompressed data
        // read one byte to get _offset_
        // read (x-190) bytes from the already uncompressed and written output data,
        // starting at (offset+2) bytes before the current write position (and add them to
        // output, of course)
        const auto len = static_cast<size_t>(x) - 190;
        const auto offset = reader.readSize<unsigned char>();
        auto* from = curTarget - (offset + 2);

        assert(from >= begin);
        assert(from <= curTarget - len);

        std::memcpy(curTarget, from, len);
        curTarget += len;
      }

      x = reader.readUnsignedChar<unsigned char>();
    }

    return result;
  }
  catch (const ReaderException& e)
  {
    return Error{e.what()};
  }
}
} // namespace

Result<void> DkPakFileSystem::doReadDirectory()
{
  try
  {
    auto reader = m_file->reader();
    reader.seekFromBegin(DkPakLayout::HeaderMagicLength);

    const auto directoryAddress = reader.readSize<int32_t>();
    const auto directorySize = reader.readSize<int32_t>();
    const auto entryCount = directorySize / DkPakLayout::EntryLength;

    reader.seekFromBegin(directoryAddress);

    for (size_t i = 0; i < entryCount; ++i)
    {
      const auto entryName = reader.readString(DkPakLayout::EntryNameLength);
      const auto entryAddress = reader.readSize<int32_t>();
      const auto uncompressedSize = reader.readSize<int32_t>();
      const auto compressedSize = reader.readSize<int32_t>();
      const auto compressed = reader.readBool<int32_t>();
      const auto entrySize = compressed ? compressedSize : uncompressedSize;

      const auto entryPath = std::filesystem::path(kdl::str_to_lower(entryName));
      auto entryFile = std::make_shared<FileView>(m_file, entryAddress, entrySize);

      if (compressed)
      {
        addFile(
          entryPath,
          [entryFile = std::move(entryFile),
           uncompressedSize]() -> Result<std::shared_ptr<File>> {
            return decompress(entryFile, uncompressedSize).transform([&](auto data) {
              return std::static_pointer_cast<File>(
                std::make_shared<OwningBufferFile>(std::move(data), uncompressedSize));
            });
          });
      }
      else
      {
        addFile(entryPath, [entryFile = std::move(entryFile)]() { return entryFile; });
      }
    }
    return kdl::void_success;
  }
  catch (const ReaderException& e)
  {
    return Error{e.what()};
  }
}
} // namespace TrenchBroom::IO
