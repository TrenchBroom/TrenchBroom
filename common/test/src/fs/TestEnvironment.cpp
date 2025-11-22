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

#include "TestEnvironment.h"

#include <QString>

#include "Macros.h"

#include <fmt/format.h>

#include <algorithm>
#include <fstream>
#include <string>

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>


namespace tb::fs
{

namespace
{

auto addNonAsciiDirs(const std::filesystem::path& rootPath)
{
  // have a non-ASCII character in the directory name to help catch
  // filename encoding bugs
  const auto cyrillic = "Кристиян";
  const auto hiraganaLetterSmallA = "ぁ";
  return rootPath / cyrillic / hiraganaLetterSmallA;
}

} // namespace

TestEnvironment::TestEnvironment(
  const std::filesystem::path& dir, const SetupFunction& setup)
  : m_sandboxPath{std::filesystem::current_path() / generateUuid()}
  , m_dir{addNonAsciiDirs(m_sandboxPath) / dir}
{
  if (!dir.is_relative())
  {
    throw std::runtime_error{fmt::format("'{}' is not a relative path", dir.string())};
  }

  createTestEnvironment(setup);
}

TestEnvironment::TestEnvironment(const SetupFunction& setup)
  : TestEnvironment{Catch::getResultCapture().getCurrentTestName(), setup}
{
}

TestEnvironment::~TestEnvironment()
{
  assertResult(deleteTestEnvironment());
}

const std::filesystem::path& TestEnvironment::dir() const
{
  return m_dir;
}

void TestEnvironment::createTestEnvironment(const SetupFunction& setup)
{
  deleteTestEnvironment();
  createDirectory({});
  setup(*this);
}

void TestEnvironment::createDirectory(const std::filesystem::path& path)
{
  std::filesystem::create_directories(m_dir / path);
}

void TestEnvironment::createFile(
  const std::filesystem::path& path, const std::string& contents)
{
  auto stream = std::ofstream{m_dir / path, std::ios::out};
  stream << contents;
}

void TestEnvironment::createSymLink(
  const std::filesystem::path& target, const std::filesystem::path& link)
{
  if (std::filesystem::is_directory(m_dir / target))
  {
    std::filesystem::create_directory_symlink(m_dir / target, m_dir / link);
  }
  else
  {
    std::filesystem::create_symlink(m_dir / target, m_dir / link);
  }
}

bool TestEnvironment::remove(const std::filesystem::path& path)
{
  return std::filesystem::remove(m_dir / path);
}

bool TestEnvironment::deleteTestEnvironment()
{
  return std::filesystem::remove_all(m_sandboxPath);
}

bool TestEnvironment::directoryExists(const std::filesystem::path& path) const
{
  return std::filesystem::is_directory(m_dir / path);
}

bool TestEnvironment::fileExists(const std::filesystem::path& path) const
{
  return std::filesystem::is_regular_file(m_dir / path);
}

std::vector<std::filesystem::path> TestEnvironment::directoryContents(
  const std::filesystem::path& path) const
{
  auto result = std::vector<std::filesystem::path>{};
  for (const auto& entry : std::filesystem::directory_iterator{m_dir / path})
  {
    result.push_back(entry.path().lexically_relative(m_dir));
  }
  std::ranges::sort(result);
  return result;
}

std::string TestEnvironment::loadFile(const std::filesystem::path& path) const
{
  auto stream = std::ifstream{m_dir / path, std::ios::in};
  return std::string{std::istreambuf_iterator<char>{stream}, {}};
}

} // namespace tb::fs
