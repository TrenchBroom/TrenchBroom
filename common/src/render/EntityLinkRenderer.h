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

#include "Color.h"
#include "Macros.h"
#include "render/LinkRenderer.h"

#include <vector>

namespace tb::mdl
{
class Map;
}

namespace tb::render
{

class EntityLinkRenderer : public LinkRenderer
{
  mdl::Map& m_map;

  Color m_defaultColor = RgbaF{0.5f, 1.0f, 0.5f, 1.0f};
  Color m_selectedColor = RgbaF{1.0f, 0.0f, 0.0f, 1.0f};

public:
  explicit EntityLinkRenderer(mdl::Map& map);

  void setDefaultColor(const Color& color);
  void setSelectedColor(const Color& color);

private:
  std::vector<LinkRenderer::LineVertex> getLinks() override;

  deleteCopy(EntityLinkRenderer);
};

} // namespace tb::render
