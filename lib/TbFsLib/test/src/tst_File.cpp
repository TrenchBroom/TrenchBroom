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

#include "fs/File.h"
#include "fs/ReaderException.h"
#include "fs/TestEnvironment.h"

#include "kd/result.h"

#include <cstring>
#include <memory>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::fs
{

TEST_CASE("OwningBufferFile")
{
  auto buffer = std::make_unique<char[]>(5);
  std::memcpy(buffer.get(), "hello", 5);
  const auto file = OwningBufferFile{std::move(buffer), 5};

  SECTION("size")
  {
    CHECK(file.size() == 5);
  }

  SECTION("reader")
  {
    auto reader = file.reader();
    CHECK(reader.readString(5) == "hello");
  }
}

TEST_CASE("CFile")
{
  auto env = TestEnvironment{};

  SECTION("createCFile")
  {
    CHECK(createCFile(env.dir() / "does_not_exist.txt").is_error());

    env.createFile("foo.txt", "hello world");
    const auto file = createCFile(env.dir() / "foo.txt") | kdl::value();
    CHECK(file->size() == 11);
  }

  SECTION("reader")
  {
    env.createFile("foo.txt", "hello world");
    const auto file = createCFile(env.dir() / "foo.txt") | kdl::value();

    auto reader = file->reader();
    CHECK(reader.readString(reader.size()) == "hello world");
  }

  SECTION("file")
  {
    env.createFile("foo.txt", "hello world");
    const auto file = createCFile(env.dir() / "foo.txt") | kdl::value();
    CHECK(file->file() != nullptr);
  }

  SECTION("buffer")
  {
    env.createFile("foo.txt", "hello world");
    const auto file = createCFile(env.dir() / "foo.txt") | kdl::value();

    const auto buffer = file->buffer();
    REQUIRE(buffer != nullptr);
    CHECK(buffer->size() == 11);
    CHECK(buffer->reader().readString(11) == "hello world");
  }

  SECTION("read")
  {
    env.createFile("foo.txt", "hello world");
    const auto file = createCFile(env.dir() / "foo.txt") | kdl::value();

    // Reader::subReaderFromBegin only validates the start position, not that
    // position + length stays within bounds, so a reader can be constructed that
    // requests more than the file actually contains. CFile::read() must catch this
    // itself, before ever calling fread.
    auto reader = file->reader().subReaderFromBegin(0, 1000);
    auto buffer = std::vector<char>(1000);
    CHECK_THROWS_AS(reader.read(buffer.data(), 1000), ReaderException);
  }
}

TEST_CASE("FileView")
{
  auto env = TestEnvironment{};
  env.createFile("foo.txt", "hello world");
  const auto file = createCFile(env.dir() / "foo.txt") | kdl::value();

  const auto view = FileView{file, 6, 5};

  SECTION("size")
  {
    CHECK(view.size() == 5);
  }

  SECTION("reader")
  {
    auto reader = view.reader();
    CHECK(reader.readString(5) == "world");
  }
}

} // namespace tb::fs
