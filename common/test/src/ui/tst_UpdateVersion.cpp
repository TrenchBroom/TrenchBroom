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

#include "ui/UpdateVersion.h"

#include <optional>

#include "Catch2.h"

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
    < UpdateVersion{SemanticVersion{1, 2, 3, _}});
  CHECK_FALSE(
    UpdateVersion{SemanticVersion{1, 2, 3, 1}}
    < UpdateVersion{SemanticVersion{1, 2, 3, 1}});
  CHECK(
    UpdateVersion{SemanticVersion{1, 2, 3, 1}}
    < UpdateVersion{SemanticVersion{1, 2, 3, 2}});
  CHECK(
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
    < UpdateVersion{TemporalVersion{2022, 2, _}});
  CHECK_FALSE(
    UpdateVersion{TemporalVersion{2022, 2, 1}}
    < UpdateVersion{TemporalVersion{2022, 2, 1}});
  CHECK(
    UpdateVersion{TemporalVersion{2022, 2, 1}}
    < UpdateVersion{TemporalVersion{2022, 2, 2}});
  CHECK(
    UpdateVersion{TemporalVersion{2022, 2, _}}
    < UpdateVersion{TemporalVersion{2022, 2, 1}});

  CHECK(
    UpdateVersion{SemanticVersion{1, 2, 3, _}}
    < UpdateVersion{TemporalVersion{2022, 2, _}});
  CHECK_FALSE(
    UpdateVersion{TemporalVersion{2022, 2, _}}
    < UpdateVersion{SemanticVersion{1, 2, 3, _}});
}

} // namespace tb::ui
