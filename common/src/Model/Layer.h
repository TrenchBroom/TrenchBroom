/*
 Copyright (C) 2020 Kristian Duske

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

#include "Color.h"

#include <optional>
#include <string>

namespace TrenchBroom
{
namespace Model
{
class Layer
{
private:
  bool m_defaultLayer;
  std::string m_name;
  std::optional<int> m_sortIndex;
  std::optional<Color> m_color;
  bool m_omitFromExport;

public:
  explicit Layer(std::string name, bool defaultLayer = false);

  bool defaultLayer() const;

  const std::string& name() const;
  void setName(std::string name);

  bool hasSortIndex() const;
  int sortIndex() const;
  void setSortIndex(int sortIndex);

  const std::optional<Color>& color() const;
  void setColor(const Color& color);

  bool omitFromExport() const;
  void setOmitFromExport(bool omitFromExport);

  static int invalidSortIndex();
  static int defaultLayerSortIndex();
};
} // namespace Model
} // namespace TrenchBroom
