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

#ifdef _WIN32
#include <windows.h>

#include <array>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#else
#include <limits.h>
#include <unistd.h>

#include <array>
#endif

namespace tb
{
namespace
{

std::filesystem::path canonicalParentPath(const std::filesystem::path& path)
{
  auto executablePath = path;
  if (executablePath.is_relative())
  {
    executablePath = std::filesystem::current_path() / executablePath;
  }

  std::error_code ec;
  const auto canonicalPath = std::filesystem::weakly_canonical(executablePath, ec);
  return ec ? executablePath.parent_path() : canonicalPath.parent_path();
}

std::filesystem::path detectExecutablePath()
{
#ifdef _WIN32
  constexpr auto MaxPathLength = 32768u;
  auto buffer = std::array<wchar_t, MaxPathLength>{};
  const auto length = GetModuleFileNameW(nullptr, buffer.data(), MaxPathLength);
  if (length == 0 || length == MaxPathLength)
  {
    return {};
  }
  return std::filesystem::path{std::wstring_view{buffer.data(), length}};
#elif defined(__APPLE__)
  auto size = uint32_t{0};
  _NSGetExecutablePath(nullptr, &size);
  if (size == 0)
  {
    return {};
  }

  auto buffer = std::string(size, '\0');
  if (_NSGetExecutablePath(buffer.data(), &size) != 0)
  {
    return {};
  }

  return std::filesystem::path{buffer.c_str()};
#else
  auto buffer = std::array<char, PATH_MAX>{};
  const auto length = readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
  if (length <= 0)
  {
    return {};
  }
  buffer[static_cast<size_t>(length)] = '\0';
  return std::filesystem::path{buffer.data()};
#endif
}

} // namespace

const std::filesystem::path& getTestExecutableDir()
{
  static const auto executableDir = [] {
    const auto executablePath = detectExecutablePath();
    return executablePath.empty() ? std::filesystem::current_path()
                                  : canonicalParentPath(executablePath);
  }();
  return executableDir;
}

std::filesystem::path getFixtureRoot()
{
  return getTestExecutableDir() / "fixture";
}

} // namespace tb
