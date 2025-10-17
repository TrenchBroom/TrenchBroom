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

#include <QList>

#include "ui/UpdateVersion.h"

#include <optional>

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::ui
{

TEST_CASE("UpdateVersion")
{
  constexpr static auto _ = std::nullopt;

  CHECK_FALSE(
    UpdateVersion{SemanticVersion{1, 2, 3, _}}
    < UpdateVersion{SemanticVersion{1, 2, 3, _}});
  CHECK_FALSE(
    UpdateVersion{SemanticVersion{1, 2, 3, _}}
    < UpdateVersion{SemanticVersion{1, 2, 2, _}});
  CHECK(
    UpdateVersion{SemanticVersion{1, 2, 2, _}}
    < UpdateVersion{SemanticVersion{1, 2, 3, _}});

  CHECK_FALSE(
    UpdateVersion{SemanticVersion{1, 2, 3, 1}}
    < UpdateVersion{SemanticVersion{1, 2, 3, 1}});
  CHECK(
    UpdateVersion{SemanticVersion{1, 2, 3, 1}}
    < UpdateVersion{SemanticVersion{1, 2, 3, 2}});

  CHECK(
    UpdateVersion{SemanticVersion{1, 2, 3, 1}}
    < UpdateVersion{SemanticVersion{1, 2, 3, _}});
  CHECK_FALSE(
    UpdateVersion{SemanticVersion{1, 2, 3, _}}
    < UpdateVersion{SemanticVersion{1, 2, 3, 2}});

  CHECK_FALSE(
    UpdateVersion{TemporalVersion{2022, 2, _}}
    < UpdateVersion{TemporalVersion{2022, 2, _}});
  CHECK_FALSE(
    UpdateVersion{TemporalVersion{2022, 2, _}}
    < UpdateVersion{TemporalVersion{2022, 1, _}});
  CHECK_FALSE(
    UpdateVersion{TemporalVersion{2022, 2, _}}
    < UpdateVersion{TemporalVersion{2021, 2, _}});
  CHECK(
    UpdateVersion{TemporalVersion{2022, 2, _}}
    < UpdateVersion{TemporalVersion{2022, 3, _}});
  CHECK(
    UpdateVersion{TemporalVersion{2022, 2, _}}
    < UpdateVersion{TemporalVersion{2023, 1, _}});

  CHECK_FALSE(
    UpdateVersion{TemporalVersion{2022, 2, 1}}
    < UpdateVersion{TemporalVersion{2022, 2, 1}});
  CHECK(
    UpdateVersion{TemporalVersion{2022, 2, 1}}
    < UpdateVersion{TemporalVersion{2022, 2, 2}});

  CHECK(
    UpdateVersion{TemporalVersion{2022, 2, 1}}
    < UpdateVersion{TemporalVersion{2022, 2, _}});
  CHECK_FALSE(
    UpdateVersion{TemporalVersion{2022, 2, _}}
    < UpdateVersion{TemporalVersion{2022, 2, 1}});

  CHECK(
    UpdateVersion{SemanticVersion{1, 2, 3, _}}
    < UpdateVersion{TemporalVersion{2022, 2, _}});
  CHECK_FALSE(
    UpdateVersion{TemporalVersion{2022, 2, _}}
    < UpdateVersion{SemanticVersion{1, 2, 3, _}});
}

TEST_CASE("parseUpdateVersion")
{
  using T = std::tuple<QString, std::optional<UpdateVersion>>;

  // clang-format off
  const auto 
  [str,           expectedVersion] = GENERATE(values<T>({
  {"",            std::nullopt},
  {"asdf",        std::nullopt},
  {"v2025.1a",    std::nullopt},
  {"v3.2.x",      std::nullopt},
  {"v3.2.1",      SemanticVersion{3, 2, 1}},
  {"v2025.1",     TemporalVersion{2025, 1}},
  {"v2025.1-RC2", TemporalVersion{2025, 1, 2}},
  }));
  // clang-format on

  CAPTURE(str);

  CHECK(parseUpdateVersion(str) == expectedVersion);
}

TEST_CASE("chooseAsset")
{
  SECTION("with release candidates")
  {
    const auto assets = QList<upd::Asset>{
      {"TrenchBroom-Win64-AMD64-v2025.3-RC3-Release.zip", QUrl{}, 0},
      {"TrenchBroom-macOS-arm64-v2025.3-RC3-Release.zip", QUrl{}, 0},
      {"TrenchBroom-macOS-x86_64-v2025.3-RC3-Release.zip", QUrl{}, 0},
      {"TrenchBroom-Linux-x86_64-v2025.3-RC3-Release.zip", QUrl{}, 0},
    };

#if defined(_WIN32)
    CHECK(chooseAsset(assets) == assets[0]);
#elif defined(__APPLE__)
#if defined(__arm64__)
    CHECK(chooseAsset(assets) == assets[1]);
#else
    CHECK(chooseAsset(assets) == assets[2]);
#endif
#else
    CHECK(chooseAsset(assets) == assets[3]);
#endif
  }

  SECTION("with release versions")
  {
    const auto assets = QList<upd::Asset>{
      {"TrenchBroom-Win64-AMD64-v2025.3-Release.zip", QUrl{}, 0},
      {"TrenchBroom-macOS-arm64-v2025.3-Release.zip", QUrl{}, 0},
      {"TrenchBroom-macOS-x86_64-v2025.3-Release.zip", QUrl{}, 0},
      {"TrenchBroom-Linux-x86_64-v2025.3-Release.zip", QUrl{}, 0},
    };

#if defined(_WIN32)
    CHECK(chooseAsset(assets) == assets[0]);
#elif defined(__APPLE__)
#if defined(__arm64__)
    CHECK(chooseAsset(assets) == assets[1]);
#else
    CHECK(chooseAsset(assets) == assets[2]);
#endif
#else
    CHECK(chooseAsset(assets) == assets[3]);
#endif
  }
}

} // namespace tb::ui
