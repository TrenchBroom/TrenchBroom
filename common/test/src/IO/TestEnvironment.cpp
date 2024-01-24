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

#include "TestEnvironment.h"

#include "IO/PathQt.h"
#include "Macros.h"

#include <fstream>
#include <string>

#include "Catch2.h"

namespace TrenchBroom::IO
{

TestEnvironment::TestEnvironment(const std::string& dir, const SetupFunction& setup)
  : m_sandboxPath{std::filesystem::current_path() / generateUuid()}
  , m_dir{m_sandboxPath / dir}
{
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

static bool deleteDirectoryAbsolute(const std::filesystem::path& absolutePath)
{
  return std::filesystem::remove_all(absolutePath);
}

bool TestEnvironment::deleteTestEnvironment()
{
  return deleteDirectoryAbsolute(m_sandboxPath);
}

bool TestEnvironment::directoryExists(const std::filesystem::path& path) const
{
  return std::filesystem::is_directory(m_dir / path);
}

bool TestEnvironment::fileExists(const std::filesystem::path& path) const
{
  return std::filesystem::is_regular_file(m_dir / path);
}

std::string TestEnvironment::loadFile(const std::filesystem::path& path) const
{
  auto stream = std::ifstream{m_dir / path, std::ios::in};
  return std::string{std::istreambuf_iterator<char>{stream}, {}};
}

} // namespace TrenchBroom::IO
