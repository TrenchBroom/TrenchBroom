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

#include "View/Animation.h"

class QWidget;

namespace tb::Renderer
{
class MapRenderer;
}

namespace tb::View
{
class FlashSelectionAnimation : public Animation
{
private:
  static const Type AnimationType;

  Renderer::MapRenderer& m_renderer;
  QWidget* m_view;

public:
  FlashSelectionAnimation(
    Renderer::MapRenderer& renderer, QWidget* view, double duration);

private:
  void doUpdate(double progress) override;
};


} // namespace tb::View
