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

#pragma once

#include "fs/TestEnvironment.h"
#include "ui/AppController.h"

#include <functional>
#include <optional>
#include <string_view>

namespace tb::ui
{

using WriteGameConfig = std::function<void(
  std::string_view gameName,
  std::string_view gameConfig,
  std::optional<std::string_view> compilationProfile,
  std::optional<std::string_view> gameEngineProfile)>;

using GameManagerInitializer =
  std::function<void(const WriteGameConfig& writeGameConfig)>;

class AppControllerFixture
{
private:
  fs::TestEnvironment m_testEnvironment;
  AppController m_appController;

public:
  explicit AppControllerFixture(
    const GameManagerInitializer& gameManagerInitializer = [](const auto&) {});
  ~AppControllerFixture();

  AppController& appController();
};

} // namespace tb::ui
