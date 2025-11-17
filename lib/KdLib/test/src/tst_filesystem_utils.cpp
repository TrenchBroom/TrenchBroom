/*
 Copyright 2025 Kristian Duske

 Permission is hereby granted, free of charge, to any person obtaining a copy of this
 software and associated documentation files (the "Software"), to deal in the Software
 without restriction, including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
*/

#include "kd/filesystem_utils.h"

#include <iterator>
#include <string>

#include <catch2/catch_test_macros.hpp>

namespace kdl
{

namespace
{

const auto read_all = [](auto& stream) {
  return std::string{
    std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
};

} // namespace

TEST_CASE("filesystem_utils")
{
  const auto fixture_dir = std::filesystem::current_path() / "fixture" / "with_stream";
  std::filesystem::create_directories(fixture_dir);

  {
    auto stream = std::ofstream{fixture_dir / "test.txt"};
    REQUIRE(stream.good());
    stream << "some content";
  }

  SECTION("with_stream")
  {
    SECTION("with_istream")
    {
      CHECK(
        with_istream(fixture_dir / "does not exist.txt", read_all)
        == result_error{"Failed to open stream"});

      CHECK(with_istream(fixture_dir / "test.txt", read_all) == "some content");
    }

    SECTION("with_ostream")
    {
      REQUIRE(with_ostream(
        fixture_dir / "test.txt", std::ios::out | std::ios::app, [](auto& stream) {
          stream << "\nmore content";
        }));
      CHECK(
        with_istream(fixture_dir / "test.txt", read_all) == "some content\nmore content");

      REQUIRE(with_ostream(fixture_dir / "some_other_name.txt", [](auto& stream) {
        stream << "some text...";
      }));
      CHECK(
        with_istream(fixture_dir / "some_other_name.txt", read_all) == "some text...");
    }
  }

  SECTION("read_file")
  {
    CHECK(
      read_file(fixture_dir / "does not exist.txt")
      == result_error{"Failed to open stream"});
    CHECK(read_file(fixture_dir / "test.txt") == "some content");
  }
}

} // namespace kdl
