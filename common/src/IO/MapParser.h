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

#include "Model/MapFormat.h"

#include "vm/forward.h"

#include <cassert>
#include <string>
#include <vector>

namespace TrenchBroom
{
struct FileLocation;
}

namespace TrenchBroom::Model
{
class EntityProperty;
class BrushFaceAttributes;
} // namespace TrenchBroom::Model

namespace TrenchBroom::IO
{
class ParserStatus;

class MapParser
{
public:
  virtual ~MapParser();

protected: // subclassing interface for users of the parser
  virtual void onBeginEntity(
    const FileLocation& startLocation,
    std::vector<Model::EntityProperty> properties,
    ParserStatus& status) = 0;
  virtual void onEndEntity(const FileLocation& endLocation, ParserStatus& status) = 0;
  virtual void onBeginBrush(const FileLocation& location, ParserStatus& status) = 0;
  virtual void onEndBrush(const FileLocation& endLocation, ParserStatus& status) = 0;
  virtual void onStandardBrushFace(
    const FileLocation& location,
    Model::MapFormat targetMapFormat,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    const vm::vec3d& point3,
    const Model::BrushFaceAttributes& attribs,
    ParserStatus& status) = 0;
  virtual void onValveBrushFace(
    const FileLocation& location,
    Model::MapFormat targetMapFormat,
    const vm::vec3d& point1,
    const vm::vec3d& point2,
    const vm::vec3d& point3,
    const Model::BrushFaceAttributes& attribs,
    const vm::vec3d& uAxis,
    const vm::vec3d& vAxis,
    ParserStatus& status) = 0;
  virtual void onPatch(
    const FileLocation& startLocation,
    const FileLocation& endLocation,
    Model::MapFormat targetMapFormat,
    size_t rowCount,
    size_t columnCount,
    std::vector<vm::vec<double, 5>> controlPoints,
    std::string materialName,
    ParserStatus& status) = 0;
};

} // namespace TrenchBroom::IO
