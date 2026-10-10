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

#include <filesystem>
#include <iterator>
#include <string>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

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
  const auto fixture_dir =
    std::filesystem::temp_directory_path() / "trenchbroom-kdlib-with_stream";
  std::filesystem::create_directories(fixture_dir);

  {
    auto stream = std::ofstream{fixture_dir / "test.txt"};
    REQUIRE(stream.good());
    stream << "some content";
  }

  {
    auto stream = std::ofstream{fixture_dir / "link_target.txt"};
    REQUIRE(stream.good());
    stream << "linked content";
  }

  std::filesystem::remove(fixture_dir / "link.txt");
  std::filesystem::create_symlink(
    fixture_dir / "link_target.txt", fixture_dir / "link.txt");

  SECTION("with_stream")
  {
    SECTION("with_istream")
    {
      CHECK(
        with_istream(fixture_dir / "does not exist.txt", read_all)
        == result_error{"Failed to open stream"});

      CHECK(with_istream(fixture_dir / "test.txt", read_all) == "some content");
      CHECK(with_istream(fixture_dir / "link.txt", read_all) == "linked content");
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

      REQUIRE(with_ostream(
        fixture_dir / "link.txt", std::ios::out | std::ios::app, [](auto& stream) {
          stream << "\nmore linked content";
        }));
      CHECK(
        with_istream(fixture_dir / "link_target.txt", read_all)
        == "linked content\nmore linked content");
      CHECK(
        with_istream(fixture_dir / "link.txt", read_all)
        == "linked content\nmore linked content");
    }
  }

  SECTION("read_file")
  {
    CHECK(
      read_file(fixture_dir / "does not exist.txt")
      == result_error{"Failed to open stream"});
    CHECK(read_file(fixture_dir / "test.txt") == "some content");
  }

  SECTION("read_lines")
  {
    CHECK(
      read_lines(fixture_dir / "does not exist.txt")
      == result_error{"Failed to open stream"});

    using T = std::tuple<std::string, std::vector<std::string>>;

    const auto [content, expected_lines] = GENERATE(values<T>({
      {"", {}},
      {"line 1", {"line 1"}},
      {"line 1\nline 2\nline 3", {"line 1", "line 2", "line 3"}},
      {"line 1\n\nline 3", {"line 1", "", "line 3"}},
      {"\nline 2", {"", "line 2"}},
      // a newline at the end of the file ends the last line and doesn't start another one
      {"line 1\nline 2\n", {"line 1", "line 2"}},
      {"line 1\n\n", {"line 1", ""}},
      {"\n", {""}},
    }));

    CAPTURE(content);

    REQUIRE(
      with_ostream(fixture_dir / "lines.txt", [&](auto& stream) { stream << content; }));
    CHECK(read_lines(fixture_dir / "lines.txt") == expected_lines);
  }
}

} // namespace kdl
