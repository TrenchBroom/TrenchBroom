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

#pragma once

#include "mdl/CompilationConfig.h"
#include "mdl/GameInfo.h"

#include <memory>
#include <string_view>
#include <vector>

namespace tb
{
class Logger;

namespace fs
{
class WritableFileSystem;
} // namespace fs

namespace mdl
{

class GameManager
{
private:
  std::unique_ptr<fs::WritableFileSystem> m_configFs;
  std::vector<GameInfo> m_gameInfos;

public:
  GameManager(
    std::unique_ptr<fs::WritableFileSystem> configFs, std::vector<GameInfo> gameInfos);

  GameManager(GameManager&&) noexcept;
  GameManager& operator=(GameManager&&) noexcept;

  ~GameManager();

  const std::vector<GameInfo>& gameInfos() const;

  const GameInfo* gameInfo(std::string_view gameName) const;
  GameInfo* gameInfo(std::string_view gameName);

  Result<void> updateCompilationConfig(
    std::string_view gameName, CompilationConfig compilationConfig, Logger& logger);

  Result<void> updateGameEngineConfig(
    std::string_view gameName, GameEngineConfig gameEngineConfig, Logger& logger);
};


Result<kdl::multi_value<GameManager, std::vector<std::string>>> initializeGameManager(
  const std::vector<std::filesystem::path>& gameConfigSearchDirs,
  const std::filesystem::path& userGameDir);

} // namespace mdl
} // namespace tb
