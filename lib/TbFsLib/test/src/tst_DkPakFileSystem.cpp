/*
 Copyright (C) 2026 Kristian Duske

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

#include "TestEnvironment.h"
#include "fs/DiskIO.h"
#include "fs/DkPakFileSystem.h"
#include "fs/File.h" // IWYU pragma: keep
#include "fs/TestUtils.h"

#include "kd/result.h"

#include <string>

#include <catch2/catch_test_macros.hpp>

namespace tb::fs
{

TEST_CASE("DkPakFileSystem")
{
  const auto fsTestPath = getFixtureRoot() / "test/fs/";

  SECTION("doReadDirectory")
  {
    SECTION("decompresses entries covering every RLE opcode and both loop exits")
    {
      // dkpak_rle.pak was generated to exercise DkPakFileSystem::decompress() beyond
      // what any legitimate compressed game asset does:
      // - "terminated.bin": opcode 0x01 (raw copy "AB"), 0x41 (a run of 3 zero bytes),
      //   0x82 'C' (a run of 4 'C' bytes), 0xFE (unmatched by any RLE case, must be a
      //   no-op), 0xC0 0x07 (a back-reference copying "AB" from the start of the
      //   output), then 0xFF followed by a padding byte, so the terminator is reached
      //   without also exhausting the reader
      // - "unterminated.bin": no terminator opcode at all; the read that consumes the
      //   entry's last byte lands exactly on eof(), so the loop must exit via
      //   `!reader.eof()` rather than by seeing `x == 0xFF`
      // - "plain.txt": an uncompressed entry, for a sanity check alongside the above
      // - "corrupt.bin": opcode 0x01 claims a 2 byte raw copy, but only 1 byte of
      //   entry data follows, so decompress() must turn the resulting ReaderException
      //   into an Error rather than letting it escape
      auto fs = openFS<DkPakFileSystem>(fsTestPath / "Pak/dkpak_rle.pak");

      // uses buffer().stringView() rather than readString(), since the decompressed
      // content contains embedded null bytes that readString would truncate at
      const auto terminated = fs->openFile("terminated.bin") | kdl::value();
      const auto terminatedContent = terminated->reader().buffer().stringView();
      CHECK(std::string{terminatedContent} == std::string{"AB\0\0\0CCCCAB", 11});

      const auto unterminated = fs->openFile("unterminated.bin") | kdl::value();
      auto unterminatedReader = unterminated->reader();
      CHECK(unterminatedReader.readString(unterminatedReader.size()) == "Z");

      const auto plain = fs->openFile("plain.txt") | kdl::value();
      auto plainReader = plain->reader();
      CHECK(plainReader.readString(plainReader.size()) == "hello");

      CHECK(fs->openFile("corrupt.bin").is_error());
    }

    SECTION("returns an error if the directory cannot be read")
    {
      // dkpak_truncated.pak claims a one entry directory starting right after the
      // header, but the file ends there, so reading the entry's name runs out of
      // bounds
      const auto file =
        Disk::openFile(fsTestPath / "Pak/dkpak_truncated.pak") | kdl::value();
      const auto result = createImageFileSystem<DkPakFileSystem>(file);
      CHECK(result.is_error());
    }
  }
}

} // namespace tb::fs
