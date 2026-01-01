/*
 Copyright (C) 2025 Kristian Duske

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

#include <QString>

#include "ui/CatchConfig.h"
#include "ui/QPathUtils.h"

#include <fmt/format.h>
#include <fmt/std.h>

#include <filesystem>
#include <string>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::ui
{

TEST_CASE("pathAsQPath")
{
  using T = std::tuple<std::filesystem::path, QString>;

  // clang-format off
  #ifdef _WIN32
  const auto 
  [fsPath,                             qPath] = GENERATE(values<T>({
  {LR"()",                              R"()"},
  {LR"(file.txt)",                      R"(file.txt)"},
  {LR"(home\user\file.txt)",            R"(home/user/file.txt)"},
  {LR"(C:\Users\user\file.txt)",        R"(C:/Users/user/file.txt)"},
  {LR"(C:\Users\Кристиян\ぁ\file.txt)", R"(C:/Users/Кристиян/ぁ/file.txt)"},
  }));
  #else
  const auto 
  [fsPath,                             qPath] = GENERATE(values<T>({
  {R"()",                              R"()"},
  {R"(file.txt)",                      R"(file.txt)"},
  {R"(/home/user/file.txt)",           R"(/home/user/file.txt)"},
  {R"(/home/Кристиян/ぁ/file.txt)",    R"(/home/Кристиян/ぁ/file.txt)"},
  }));
  #endif
  // clang-format on

  CAPTURE(fmt::format("{}", fsPath));

  CHECK(pathAsQPath(fsPath) == qPath);
}

TEST_CASE("pathAsQString")
{
  using T = std::tuple<std::filesystem::path, std::string>;

  // clang-format off
  #ifdef _WIN32
  const auto 
  [fsPath,                             qPath] = GENERATE(values<T>({
  {LR"()",                              R"()"},
  {LR"(file.txt)",                      R"(file.txt)"},
  {LR"(home\user\file.txt)",            R"(home\user\file.txt)"},
  {LR"(C:\Users\user\file.txt)",        R"(C:\Users\user\file.txt)"},
  {LR"(C:\Users\Кристиян\ぁ\file.txt)", R"(C:\Users\Кристиян\ぁ\file.txt)"},
  }));
  #else
  const auto 
  [fsPath,                             qPath] = GENERATE(values<T>({
  {R"()",                              R"()"},
  {R"(file.txt)",                      R"(file.txt)"},
  {R"(/home/user/file.txt)",           R"(/home/user/file.txt)"},
  {R"(/home/Кристиян/ぁ/file.txt)",    R"(/home/Кристиян/ぁ/file.txt)"},
  }));
  #endif
  // clang-format on

  CAPTURE(fmt::format("{}", fsPath));

  CHECK(pathAsQString(fsPath) == QString::fromStdString(qPath));
}

TEST_CASE("pathAsGenericQString")
{
  using T = std::tuple<std::filesystem::path, QString>;

  // clang-format off
  #ifdef _WIN32
  const auto 
  [fsPath,                             qPath] = GENERATE(values<T>({
  {LR"()",                              R"()"},
  {LR"(file.txt)",                      R"(file.txt)"},
  {LR"(home\user\file.txt)",           R"(home/user/file.txt)"},
  {LR"(C:\Users\user\file.txt)",        R"(C:/Users/user/file.txt)"},
  {LR"(C:\Users\Кристиян\ぁ\file.txt)", R"(C:/Users/Кристиян/ぁ/file.txt)"},
  }));
  #else
  const auto 
  [fsPath,                             qPath] = GENERATE(values<T>({
  {R"()",                              R"()"},
  {R"(file.txt)",                      R"(file.txt)"},
  {R"(/home/user/file.txt)",           R"(/home/user/file.txt)"},
  {R"(/home/Кристиян/ぁ/file.txt)",    R"(/home/Кристиян/ぁ/file.txt)"},
  }));
  #endif
  // clang-format on

  CAPTURE(fmt::format("{}", fsPath));

  CHECK(pathAsGenericQString(fsPath) == qPath);
}

TEST_CASE("pathFromQString")
{
  using T = std::tuple<QString, std::filesystem::path>;

  // clang-format off
  #ifdef _WIN32
  const auto 
  [qPath,                              fsPath] = GENERATE(values<T>({
  {R"()",                              LR"()"},
  {R"(file.txt)",                      LR"(file.txt)"},
  {R"(home\user\file.txt)",            LR"(home\user\file.txt)"},
  {R"(C:\Users\user\file.txt)",        LR"(C:\Users\user\file.txt)"},
  {R"(C:\Users\Кристиян\ぁ\file.txt)", LR"(C:\Users\Кристиян\ぁ\file.txt)"},
  {R"(C:/Users/user/file.txt)",        LR"(C:\Users\user\file.txt)"},
  {R"(C:/Users/Кристиян/ぁ/file.txt)", LR"(C:\Users\Кристиян\ぁ\file.txt)"},
  }));
  #else 
  const auto 
  [qPath,                              fsPath] = GENERATE(values<T>({
  {R"()",                              R"()"},
  {R"(file.txt)",                      R"(file.txt)"},
  {R"(C:\Users\user\file.txt)",        R"(C:/Users/user/file.txt)"},
  {R"(/home/user/file.txt)",           R"(/home/user/file.txt)"},
  {R"(/home/Кристиян/ぁ/file.txt)",    R"(/home/Кристиян/ぁ/file.txt)"},
  }));
  #endif
  // clang-format on

  CAPTURE(qPath);

  // We cannot just use a CHECK macro here because it triggers Catch2's builtin string
  // conversion for std::filesystem::path, which throws on windows if the path contains
  // wide characters. Instead, we use fmt::format, which handles everything correctly.
  if (pathFromQString(qPath) != fsPath)
  {
    CAPTURE(fmt::format("{}", pathFromQString(qPath)), fmt::format("{}", fsPath));
    FAIL();
  }
}

} // namespace tb::ui
