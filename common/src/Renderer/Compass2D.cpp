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

#include "Compass2D.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"

namespace TrenchBroom
{
namespace Renderer
{
void Compass2D::doRenderCompass(
  RenderContext& renderContext, const vm::mat4x4f& transform)
{
  const auto& camera = renderContext.camera();
  const auto axis = vm::find_abs_max_component(camera.direction());

  auto& prefs = PreferenceManager::instance();
  if (axis != vm::axis::z)
  {
    renderSolidAxis(renderContext, transform, prefs.get(Preferences::ZAxisColor));
  }
  if (axis != vm::axis::x)
  {
    renderSolidAxis(
      renderContext,
      transform * vm::mat4x4f::rot_90_y_ccw(),
      prefs.get(Preferences::XAxisColor));
  }
  if (axis != vm::axis::y)
  {
    renderSolidAxis(
      renderContext,
      transform * vm::mat4x4f::rot_90_x_cw(),
      prefs.get(Preferences::YAxisColor));
  }
}
} // namespace Renderer
} // namespace TrenchBroom
