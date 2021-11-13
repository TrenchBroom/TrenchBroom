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

#include "Exceptions.h"
#include "IO/MapReader.h"

#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace TrenchBroom {
namespace Model {
struct EntityPropertyConfig;
class WorldNode;
} // namespace Model

namespace IO {
class ParserStatus;

class WorldReaderException : public Exception {
public:
  WorldReaderException();
  WorldReaderException(
    const std::vector<std::tuple<Model::MapFormat, std::string>>& parserExceptions);
};

/**
 * MapReader subclass for loading a whole .map file.
 */
class WorldReader : public MapReader {
  std::unique_ptr<Model::WorldNode> m_world;

public:
  WorldReader(
    std::string_view str, Model::MapFormat sourceAndTargetMapFormat,
    const Model::EntityPropertyConfig& entityPropertyConfig);

  std::unique_ptr<Model::WorldNode> read(const vm::bbox3& worldBounds, ParserStatus& status);

  /**
   * Try to parse the given string as the given map formats, in order.
   * Returns the world if parsing is successful, otherwise throws an exception.
   *
   * @param str the string to parse
   * @param mapFormatsToTry formats to try, in order
   * @param worldBounds world bounds
   * @param status status
   * @return the world node
   * @throws WorldReaderException if `str` can't be parsed by any of the given formats
   */
  static std::unique_ptr<Model::WorldNode> tryRead(
    std::string_view str, const std::vector<Model::MapFormat>& mapFormatsToTry,
    const vm::bbox3& worldBounds, const Model::EntityPropertyConfig& entityPropertyConfig,
    ParserStatus& status);

private:
  void sanitizeLayerSortIndicies(ParserStatus& status);

private: // implement MapReader interface
  Model::Node* onWorldNode(
    std::unique_ptr<Model::WorldNode> worldNode, ParserStatus& status) override;
  void onLayerNode(std::unique_ptr<Model::Node> layerNode, ParserStatus& status) override;
  void onNode(
    Model::Node* parentNode, std::unique_ptr<Model::Node> node, ParserStatus& status) override;
};
} // namespace IO
} // namespace TrenchBroom
