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

#include "fs/CachedFileTree.h"
#include "fs/TraversalMode.h"

#include "kd/path_utils.h"

#include <filesystem>
#include <optional>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace tb::fs
{
namespace
{

using TestFileEntry = CachedFileEntry<int>;
using TestDirectoryEntry = CachedDirectoryEntry<int>;
using TestEntry = CachedEntry<int>;

TestEntry makeRoot()
{
  return TestDirectoryEntry{{}, {}, {}};
}

void insertFile(TestEntry& root, const std::filesystem::path& path, const int payload)
{
  auto& directoryEntry =
    findOrCreateCachedDirectory(path.parent_path(), std::get<TestDirectoryEntry>(root));
  addCachedEntry(directoryEntry, TestFileEntry{path.filename(), payload});
}

} // namespace

TEST_CASE("CachedFileTree")
{
  SECTION("findOrCreateCachedDirectory")
  {
    SECTION("builds intermediate directories")
    {
      auto root = makeRoot();
      auto& dir =
        findOrCreateCachedDirectory("a/b/c", std::get<TestDirectoryEntry>(root));
      CHECK(dir.name == "c");

      const auto* aEntry = findCachedEntry(kdl::path_to_lower("a"), root);
      REQUIRE(aEntry != nullptr);
      CHECK(isDirectoryEntry(*aEntry));

      const auto* bEntry = findCachedEntry(kdl::path_to_lower("a/b"), root);
      REQUIRE(bEntry != nullptr);
      CHECK(isDirectoryEntry(*bEntry));

      const auto* cEntry = findCachedEntry(kdl::path_to_lower("a/b/c"), root);
      REQUIRE(cEntry != nullptr);
      CHECK(isDirectoryEntry(*cEntry));
    }

    SECTION("is idempotent")
    {
      auto root = makeRoot();
      findOrCreateCachedDirectory("a/b", std::get<TestDirectoryEntry>(root));
      findOrCreateCachedDirectory("a/b", std::get<TestDirectoryEntry>(root));

      const auto& rootDir = std::get<TestDirectoryEntry>(root);
      REQUIRE(rootDir.entries.size() == 1);

      const auto& aDir = std::get<TestDirectoryEntry>(rootDir.entries.front());
      CHECK(aDir.entries.size() == 1);
    }

    SECTION("replaces a file entry that collides with a directory path")
    {
      auto root = makeRoot();
      insertFile(root, "a", 1);

      const auto* fileEntry = findCachedEntry(kdl::path_to_lower("a"), root);
      REQUIRE(fileEntry != nullptr);
      CHECK_FALSE(isDirectoryEntry(*fileEntry));

      findOrCreateCachedDirectory("a/b", std::get<TestDirectoryEntry>(root));

      const auto* dirEntry = findCachedEntry(kdl::path_to_lower("a"), root);
      REQUIRE(dirEntry != nullptr);
      CHECK(isDirectoryEntry(*dirEntry));

      const auto* bEntry = findCachedEntry(kdl::path_to_lower("a/b"), root);
      REQUIRE(bEntry != nullptr);
      CHECK(isDirectoryEntry(*bEntry));
    }
  }

  SECTION("findCachedEntry")
  {
    auto root = makeRoot();
    insertFile(root, "SomeDir/File.txt", 1);

    const auto* entry = findCachedEntry(kdl::path_to_lower("somedir/file.txt"), root);
    REQUIRE(entry != nullptr);
    CHECK(getEntryName(*entry) == "File.txt");

    CHECK(findCachedEntry(kdl::path_to_lower("somedir/missing.txt"), root) == nullptr);
  }

  SECTION("findChildEntry")
  {
    auto root = makeRoot();
    insertFile(root, "File.txt", 1);

    auto& rootDir = std::get<TestDirectoryEntry>(root);
    const auto entryIt = findChildEntry(rootDir, std::filesystem::path{"file.txt"});
    REQUIRE(entryIt != rootDir.entries.end());
    CHECK(getEntryName(*entryIt) == "File.txt");

    CHECK(
      findChildEntry(rootDir, std::filesystem::path{"missing.txt"})
      == rootDir.entries.end());
  }

  SECTION("withCacheEntry")
  {
    SECTION("locates an entry and reports its payload")
    {
      auto root = makeRoot();
      insertFile(root, "a/File.txt", 42);

      const auto payload = withCachedEntry(
        kdl::path_to_lower("a/file.txt"),
        root,
        std::filesystem::path{},
        [](const TestEntry& entry, const std::filesystem::path&) {
          return std::get<TestFileEntry>(entry).payload;
        },
        -1);

      CHECK(payload == 42);
    }

    SECTION("accumulates the true stored case")
    {
      auto root = makeRoot();
      insertFile(root, "SomeDir/File.txt", 1);

      const auto path = withCachedEntry(
        kdl::path_to_lower("somedir/file.txt"),
        root,
        std::filesystem::path{},
        [](
          const TestEntry&, const std::filesystem::path& entryPath) { return entryPath; },
        std::filesystem::path{});

      CHECK(path == "SomeDir/File.txt");
    }

    SECTION("yields the default result for a missing path")
    {
      auto root = makeRoot();
      insertFile(root, "a/file.txt", 1);

      const auto payload = withCachedEntry(
        kdl::path_to_lower("a/missing.txt"),
        root,
        std::filesystem::path{},
        [](const TestEntry& entry, const std::filesystem::path&) {
          return std::get<TestFileEntry>(entry).payload;
        },
        -1);

      CHECK(payload == -1);
    }

    SECTION("invokes the callback for a case-mismatched path with the true stored case")
    {
      auto root = makeRoot();
      insertFile(root, "SomeDir/File.txt", 1);

      auto calledPath = std::optional<std::filesystem::path>{};
      withCachedEntry(
        kdl::path_to_lower("SOMEDIR/FILE.TXT"),
        root,
        std::filesystem::path{},
        [&](const TestEntry&, const std::filesystem::path& entryPath) {
          calledPath = entryPath;
        });

      REQUIRE(calledPath.has_value());
      CHECK(*calledPath == "SomeDir/File.txt");
    }

    SECTION("does not invoke the callback for a missing path")
    {
      auto root = makeRoot();
      insertFile(root, "a/file.txt", 1);

      auto called = false;
      withCachedEntry(
        kdl::path_to_lower("a/missing.txt"),
        root,
        std::filesystem::path{},
        [&](const TestEntry&, const std::filesystem::path&) { called = true; });

      CHECK_FALSE(called);
    }
  }

  SECTION("collectCachedEntries")
  {
    SECTION("recursively enumerates with true-cased paths")
    {
      auto root = makeRoot();
      insertFile(root, "SomeDir/File1.txt", 1);
      insertFile(root, "SomeDir/NestedDir/File2.txt", 2);
      insertFile(root, "OtherFile.txt", 3);

      auto result = std::vector<std::filesystem::path>{};
      collectCachedEntries(root, {}, 0, TraversalMode::Recursive, result);

      CHECK_THAT(
        result,
        Catch::Matchers::UnorderedEquals(std::vector<std::filesystem::path>{
          "SomeDir",
          "SomeDir/File1.txt",
          "SomeDir/NestedDir",
          "SomeDir/NestedDir/File2.txt",
          "OtherFile.txt",
        }));
    }

    SECTION("respects TraversalMode::Flat / depth limits")
    {
      auto root = makeRoot();
      insertFile(root, "SomeDir/File1.txt", 1);
      insertFile(root, "SomeDir/NestedDir/File2.txt", 2);
      insertFile(root, "OtherFile.txt", 3);

      auto result = std::vector<std::filesystem::path>{};
      collectCachedEntries(root, {}, 0, TraversalMode::Flat, result);

      CHECK_THAT(
        result,
        Catch::Matchers::UnorderedEquals(std::vector<std::filesystem::path>{
          "SomeDir",
          "OtherFile.txt",
        }));
    }
  }
}

} // namespace tb::fs
