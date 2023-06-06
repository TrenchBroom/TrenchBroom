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

#pragma once

#include "IO/DiskIO.h"
#include "Uuid.h"

#include "kdl/invoke.h"

#include <filesystem>
#include <functional>
#include <string>

namespace TrenchBroom
{
namespace IO
{
class TestEnvironment
{
private:
  using SetupFunction = std::function<void(TestEnvironment&)>;
  std::filesystem::path m_sandboxPath;
  std::filesystem::path m_dir;

public:
  explicit TestEnvironment(
    const std::string& dir, const SetupFunction& setup = [](TestEnvironment&) {});
  explicit TestEnvironment(const SetupFunction& setup = [](TestEnvironment&) {});
  ~TestEnvironment();

  const std::filesystem::path& dir() const;

public:
  void createTestEnvironment(const SetupFunction& setup);
  void createDirectory(const std::filesystem::path& path);
  void createFile(const std::filesystem::path& path, const std::string& contents);

  bool deleteTestEnvironment();

  bool directoryExists(const std::filesystem::path& path) const;
  bool fileExists(const std::filesystem::path& path) const;

  std::string loadFile(const std::filesystem::path& path) const;

  template <typename F>
  auto withTempFile(const std::string& contents, const F& f)
  {
    const auto path = m_dir / generateUuid();
    auto removeFile = kdl::invoke_later{[&]() {
      // ignore errors
      auto error = std::error_code{};
      std::filesystem::remove(path, error);
    }};

    Disk::withOutputStream(path, [&](auto& stream) {
      stream << contents;
    }).if_error([](auto e) { throw std::runtime_error{e.msg}; });
    return f(path);
  }
};
} // namespace IO
} // namespace TrenchBroom
