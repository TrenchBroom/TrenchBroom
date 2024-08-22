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

#include "Color.h"
#include "Macros.h"
#include "Renderer/LinkRenderer.h"

#include <memory>
#include <vector>

namespace TrenchBroom::View
{
class MapDocument; // FIXME: Renderer should not depend on View
}

namespace TrenchBroom::Renderer
{

class EntityLinkRenderer : public LinkRenderer
{
  std::weak_ptr<View::MapDocument> m_document;

  Color m_defaultColor = {0.5f, 1.0f, 0.5f, 1.0f};
  Color m_selectedColor = {1.0f, 0.0f, 0.0f, 1.0f};

public:
  explicit EntityLinkRenderer(std::weak_ptr<View::MapDocument> document);

  void setDefaultColor(const Color& color);
  void setSelectedColor(const Color& color);

private:
  std::vector<LinkRenderer::LineVertex> getLinks() override;

  deleteCopy(EntityLinkRenderer);
};

} // namespace TrenchBroom::Renderer
