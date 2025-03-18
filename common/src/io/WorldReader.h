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

#pragma once

#include "Result.h"
#include "io/MapReader.h"

#include <memory>
#include <vector>

namespace kdl
{
class task_manager;
}

namespace tb::mdl
{
struct EntityPropertyConfig;
class WorldNode;
} // namespace tb::mdl

namespace tb::io
{
class ParserStatus;

/**
 * MapReader subclass for loading a whole .map file.
 */
class WorldReader : public MapReader
{
  std::unique_ptr<mdl::WorldNode> m_worldNode;

public:
  WorldReader(
    std::string_view str,
    mdl::MapFormat sourceAndTargetMapFormat,
    const mdl::EntityPropertyConfig& entityPropertyConfig);

  Result<std::unique_ptr<mdl::WorldNode>> read(
    const vm::bbox3d& worldBounds, ParserStatus& status, kdl::task_manager& taskManager);

  /**
   * Try to parse the given string as the given map formats, in order.
   * Returns the world if parsing is successful, otherwise returns an error.
   *
   * @param str the string to parse
   * @param mapFormatsToTry formats to try, in order
   * @param worldBounds world bounds
   * @param status status
   * @param taskManager the task manager to use for parallel tasks
   * @return the world node or an error if `str` can't be parsed by any of the given
   * formats
   */
  static Result<std::unique_ptr<mdl::WorldNode>> tryRead(
    std::string_view str,
    const std::vector<mdl::MapFormat>& mapFormatsToTry,
    const vm::bbox3d& worldBounds,
    const mdl::EntityPropertyConfig& entityPropertyConfig,
    ParserStatus& status,
    kdl::task_manager& taskManager);

private: // implement MapReader interface
  mdl::Node* onWorldNode(
    std::unique_ptr<mdl::WorldNode> worldNode, ParserStatus& status) override;
  void onLayerNode(std::unique_ptr<mdl::Node> layerNode, ParserStatus& status) override;
  void onNode(
    mdl::Node* parentNode,
    std::unique_ptr<mdl::Node> node,
    ParserStatus& status) override;
};

} // namespace tb::io
