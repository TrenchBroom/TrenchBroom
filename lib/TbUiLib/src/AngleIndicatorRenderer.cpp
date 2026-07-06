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

#include "ui/AngleIndicatorRenderer.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "gl/ActiveShader.h"
#include "gl/GlInterface.h"
#include "gl/Shaders.h"
#include "render/Circle.h"
#include "render/RenderBatch.h"
#include "render/RenderContext.h"
#include "render/RenderService.h"
#include "render/Renderable.h"

#include "vm/mat_ext.h"
#include "vm/quat.h"
#include "vm/vec.h"

#include <sstream>

namespace tb::ui
{
namespace
{

class AngleIndicatorRenderer : public render::DirectRenderable
{
private:
  vm::vec3d m_position;
  render::Circle m_circle;

public:
  AngleIndicatorRenderer(
    const vm::vec3d& position,
    const float radius,
    const vm::axis::type axis,
    const vm::vec3d& startAxis,
    const vm::vec3d& endAxis)
    : m_position{position}
    , m_circle{radius, 24, true, axis, vm::vec3f{startAxis}, vm::vec3f{endAxis}}
  {
  }

  void prepare(gl::Gl& gl, gl::VboManager& vboManager) override
  {
    m_circle.prepare(gl, vboManager);
  }

  void render(render::RenderContext& renderContext) override
  {
    auto& gl = renderContext.gl();

    gl.disable(GL_DEPTH_TEST);

    gl.pushAttrib(GL_POLYGON_BIT);
    gl.disable(GL_CULL_FACE);
    gl.polygonMode(GL_FRONT_AND_BACK, GL_FILL);

    auto translation = render::MultiplyModelMatrix{
      renderContext.transformation(), vm::translation_matrix(vm::vec3f{m_position})};
    auto shader = gl::ActiveShader{
      gl, renderContext.shaderManager(), gl::Shaders::VaryingPUniformCShader};
    shader.set("Color", RgbaF{1.0f, 1.0f, 1.0f, 0.2f});
    m_circle.render(gl, shader.program());

    gl.enable(GL_DEPTH_TEST);
    gl.popAttrib();
  }
};

std::string angleString(const double angle)
{
  auto str = std::stringstream{};
  str.precision(2);
  str.setf(std::ios::fixed);
  str << angle;
  return str.str();
}

} // namespace

void renderAngleIndicator(
  render::RenderBatch& renderBatch,
  const double handleRadius,
  const vm::vec3d& center,
  const vm::vec3d& axis,
  const vm::vec3d& initialHandlePosition,
  const double angle)
{
  if (const auto radius = static_cast<float>(handleRadius);
      radius > 0.0f && std::abs(angle) > vm::Cd::almost_zero())
  {
    const auto startAxis = vm::normalize(initialHandlePosition - center);
    const auto endAxis = vm::quatd{axis, angle} * startAxis;

    renderBatch.addOneShot(new AngleIndicatorRenderer{
      center, radius, vm::find_abs_max_component(axis), startAxis, endAxis});
  }
}

void renderAngleText(
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch,
  const vm::vec3d& position,
  const double angleInDegrees)
{
  auto renderService = render::RenderService{renderContext, renderBatch};

  renderService.setForegroundColor(pref(Preferences::SelectedInfoOverlayTextColor));
  renderService.setBackgroundColor(pref(Preferences::SelectedInfoOverlayBackgroundColor));
  renderService.renderString(angleString(angleInDegrees), vm::vec3f{position});
}

} // namespace tb::ui
