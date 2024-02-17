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

#include "Layer.h"

#include "kdl/reflection_impl.h"

#include "vecmath/vec_io.h"

namespace TrenchBroom::Model
{

kdl_reflect_impl(Layer);

Layer::Layer(std::string name, const bool defaultLayer)
  : m_defaultLayer{defaultLayer}
  , m_name{std::move(name)}
{
}

bool Layer::defaultLayer() const
{
  return m_defaultLayer;
}

const std::string& Layer::name() const
{
  return m_name;
}

void Layer::setName(std::string name)
{
  m_name = std::move(name);
}

bool Layer::hasSortIndex() const
{
  return m_sortIndex.has_value();
}

int Layer::sortIndex() const
{
  if (defaultLayer())
  {
    return defaultLayerSortIndex();
  }

  return m_sortIndex.value_or(invalidSortIndex());
}

void Layer::setSortIndex(const int sortIndex)
{
  m_sortIndex = sortIndex;
}

const std::optional<Color>& Layer::color() const
{
  return m_color;
}

void Layer::setColor(const Color& color)
{
  m_color = color;
}

bool Layer::omitFromExport() const
{
  return m_omitFromExport;
}

void Layer::setOmitFromExport(const bool omitFromExport)
{
  m_omitFromExport = omitFromExport;
}

int Layer::invalidSortIndex()
{
  return std::numeric_limits<int>::max();
}

int Layer::defaultLayerSortIndex()
{
  return -1;
}

} // namespace TrenchBroom::Model
