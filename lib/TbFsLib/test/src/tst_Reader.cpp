/*
 Copyright (C) 2018 Eric Wasylishen

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
#include "fs/File.h"
#include "fs/Reader.h"
#include "fs/ReaderException.h"

#include <filesystem>
#include <memory>
#include <string>

#include <catch2/catch_test_macros.hpp>

namespace tb::fs
{
namespace
{

const char* buff()
{
  static const auto* result = "abcdefghij_";
  return result;
}

std::shared_ptr<File> file()
{
  static auto result =
    Disk::openFile(getFixtureRoot() / "test/fs/Reader/10byte") | kdl::value();
  return result;
}

void createEmpty(Reader&& r)
{
  CHECK(r.size() == 0U);
  CHECK(r.position() == 0U);
  CHECK_NOTHROW(r.seekFromBegin(0U));
  CHECK_NOTHROW(r.seekFromEnd(0U));
  CHECK_NOTHROW(r.seekForward(0U));
  CHECK(!r.canRead(1U));
  CHECK(r.canRead(0U));
  CHECK(r.eof());
  CHECK_THROWS_AS(r.readChar<char>(), ReaderException);
}

void createNonEmpty(Reader&& r)
{
  CHECK(r.size() == 10U);
  CHECK(r.position() == 0U);
  CHECK(r.canRead(0U));
  CHECK(r.canRead(10U));
  CHECK(!r.canRead(11U));
  CHECK(!r.eof());

  // read a char
  CHECK(r.readChar<char>() == 'a');
  CHECK(r.position() == 1U);
  CHECK(r.canRead(1U));
  CHECK(r.canRead(9U));
  CHECK(!r.canRead(10U));

  // read remainder
  CHECK(r.readString(9) == std::string("bcdefghij"));
  CHECK(r.position() == 10U);
  CHECK(!r.canRead(1U));
  CHECK(r.canRead(0U));
  CHECK(r.eof());
  CHECK_THROWS_AS(r.readChar<char>(), ReaderException);
}

void seekFromBegin(Reader&& r)
{
  r.seekFromBegin(0U);
  CHECK(r.position() == 0U);

  r.seekFromBegin(1U);
  CHECK(r.position() == 1U);

  r.seekFromBegin(2U);
  CHECK(r.position() == 2U);

  CHECK_THROWS_AS(r.seekFromBegin(11U), ReaderException);
  CHECK(r.position() == 2U);
}

void seekFromEnd(Reader&& r)
{
  r.seekFromEnd(0U);
  CHECK(r.position() == 10U);

  r.seekFromEnd(1U);
  CHECK(r.position() == 9U);

  r.seekFromEnd(10U);
  CHECK(r.position() == 0U);

  CHECK_THROWS_AS(r.seekFromEnd(11U), ReaderException);
  CHECK(r.position() == 0U);
}

void seekForward(Reader&& r)
{
  r.seekForward(1U);
  CHECK(r.position() == 1U);

  r.seekForward(1U);
  CHECK(r.position() == 2U);

  CHECK_THROWS_AS(r.seekForward(9U), ReaderException);
  CHECK(r.position() == 2U);
}

void subReader(Reader&& r)
{
  auto s = r.subReaderFromBegin(5, 3);

  CHECK(s.size() == 3U);
  CHECK(s.position() == 0U);

  CHECK(s.readChar<char>() == 'f');
  CHECK(s.position() == 1U);

  CHECK(s.readChar<char>() == 'g');
  CHECK(s.position() == 2U);

  CHECK(s.readChar<char>() == 'h');
  CHECK(s.position() == 3U);

  CHECK_THROWS_AS(s.seekForward(1U), ReaderException);
  CHECK(s.position() == 3U);
}

} // namespace

TEST_CASE("Reader")
{
  SECTION("createEmpty")
  {
    SECTION("buffer-backed")
    {
      createEmpty(Reader::from(buff(), buff()));
    }

    SECTION("file-backed")
    {
      const auto emptyFile =
        Disk::openFile(getFixtureRoot() / "test/fs/Reader/empty") | kdl::value();
      createEmpty(emptyFile->reader());
    }
  }

  SECTION("createNonEmpty")
  {
    SECTION("buffer-backed")
    {
      createNonEmpty(Reader::from(buff(), buff() + 10));
    }

    SECTION("file-backed")
    {
      createNonEmpty(file()->reader());
    }
  }

  SECTION("seekFromBegin")
  {
    SECTION("buffer-backed")
    {
      seekFromBegin(Reader::from(buff(), buff() + 10));
    }

    SECTION("file-backed")
    {
      seekFromBegin(file()->reader());
    }
  }

  SECTION("seekFromEnd")
  {
    SECTION("buffer-backed")
    {
      seekFromEnd(Reader::from(buff(), buff() + 10));
    }

    SECTION("file-backed")
    {
      seekFromEnd(file()->reader());
    }
  }

  SECTION("seekForward")
  {
    SECTION("buffer-backed")
    {
      seekForward(Reader::from(buff(), buff() + 10));
    }

    SECTION("file-backed")
    {
      seekForward(file()->reader());
    }
  }

  SECTION("seekBackward")
  {
    SECTION("buffer-backed")
    {
      auto r = Reader::from(buff(), buff() + 10);

      r.seekForward(5U);
      CHECK(r.position() == 5U);

      r.seekBackward(2U);
      CHECK(r.position() == 3U);

      r.seekBackward(3U);
      CHECK(r.position() == 0U);

      CHECK_THROWS_AS(r.seekBackward(1U), ReaderException);
      CHECK(r.position() == 0U);
    }

    SECTION("file-backed")
    {
      auto r = file()->reader();

      r.seekForward(5U);
      CHECK(r.position() == 5U);

      r.seekBackward(2U);
      CHECK(r.position() == 3U);

      r.seekBackward(3U);
      CHECK(r.position() == 0U);

      CHECK_THROWS_AS(r.seekBackward(1U), ReaderException);
      CHECK(r.position() == 0U);
    }
  }

  SECTION("copyConstructor")
  {
    auto reader = Reader::from(buff(), buff() + 10);
    REQUIRE(reader.readString(4) == "abcd");
    REQUIRE(reader.canRead(6));
    REQUIRE_FALSE(reader.canRead(7));

    auto copy = Reader{reader};
    CHECK(reader.canRead(6) == copy.canRead(6));
    CHECK(reader.canRead(7) == copy.canRead(7));

    CHECK(reader.readString(2) == copy.readString(2));

    reader.seekFromBegin(0);
    copy.seekFromBegin(0);
    CHECK(reader.readString(2) == copy.readString(2));
  }

  SECTION("subReader")
  {
    SECTION("buffer-backed")
    {
      subReader(Reader::from(buff(), buff() + 10));
    }

    SECTION("file-backed")
    {
      subReader(file()->reader());
    }
  }

  SECTION("subReaderFromBeginToEnd")
  {
    SECTION("buffer-backed")
    {
      auto r = Reader::from(buff(), buff() + 10);
      auto s = r.subReaderFromBegin(5);

      CHECK(s.size() == 5U);
      CHECK(s.readString(5) == "fghij");
    }

    SECTION("file-backed")
    {
      auto r = file()->reader();
      auto s = r.subReaderFromBegin(5);

      CHECK(s.size() == 5U);
      CHECK(s.readString(5) == "fghij");
    }
  }

  SECTION("subReaderFromCurrent")
  {
    SECTION("buffer-backed")
    {
      auto r = Reader::from(buff(), buff() + 10);
      r.seekFromBegin(2U);

      auto s = r.subReaderFromCurrent(1, 3);
      CHECK(s.size() == 3U);
      CHECK(s.readString(3) == "def");

      auto s2 = r.subReaderFromCurrent(5);
      CHECK(s2.size() == 5U);
      CHECK(s2.readString(5) == "cdefg");
    }

    SECTION("file-backed")
    {
      auto r = file()->reader();
      r.seekFromBegin(2U);

      auto s = r.subReaderFromCurrent(1, 3);
      CHECK(s.size() == 3U);
      CHECK(s.readString(3) == "def");

      auto s2 = r.subReaderFromCurrent(5);
      CHECK(s2.size() == 5U);
      CHECK(s2.readString(5) == "cdefg");
    }
  }

  SECTION("invalidRange")
  {
    CHECK_THROWS_AS(Reader::from(buff() + 1, buff()), ReaderException);
  }
}

TEST_CASE("BufferedReader")
{
  SECTION("subReaderOutlivesParent")
  {
    // OwningBufferReaderSource (the source behind a BufferedReader obtained from a
    // file-backed reader) must keep its buffer alive independently of the
    // BufferedReader it was created from, since a sub-reader derived from it can
    // outlive its parent.
    auto sub = [] {
      auto buffered = file()->reader().buffer();
      return buffered.subReaderFromBegin(5, 3);
    }();

    CHECK(sub.readString(3) == "fgh");
  }

  SECTION("buffer")
  {
    auto buffered = file()->reader().buffer();

    SECTION("begin, end and stringView expose the underlying memory region")
    {
      CHECK(buffered.end() == buffered.begin() + buffered.size());
      CHECK(buffered.stringView() == "abcdefghij");
    }

    SECTION("buffering an already buffered reader returns itself")
    {
      auto buffered2 = buffered.buffer();
      CHECK(buffered2.begin() == buffered.begin());
      CHECK(buffered2.end() == buffered.end());
    }

    SECTION("buffering a sub-reader of a buffered reader still shares the same buffer")
    {
      auto sub = buffered.subReaderFromBegin(5, 3).buffer();
      CHECK(sub.stringView() == "fgh");
    }
  }
}

} // namespace tb::fs
