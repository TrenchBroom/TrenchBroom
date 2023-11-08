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

#include "RenderService.h"

#include "AttrString.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/Camera.h"
#include "Renderer/FontDescriptor.h"
#include "Renderer/IndexRangeMapBuilder.h"
#include "Renderer/IndexRangeRenderer.h"
#include "Renderer/PointHandleRenderer.h"
#include "Renderer/PrimitiveRenderer.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/TextAnchor.h"
#include "Renderer/TextRenderer.h"

#include <vecmath/forward.h>
#include <vecmath/polygon.h>
#include <vecmath/segment.h>
#include <vecmath/vec.h>
#include <vecmath/vec_ext.h>

namespace TrenchBroom
{
namespace Renderer
{
Renderer::FontDescriptor makeRenderServiceFont();
Renderer::FontDescriptor makeRenderServiceFont()
{
  return Renderer::FontDescriptor(
    pref(Preferences::RendererFontPath()),
    static_cast<size_t>(pref(Preferences::RendererFontSize)));
}

class RenderService::HeadsUpTextAnchor : public TextAnchor
{
private:
  vm::vec3f offset(const Camera& camera, const vm::vec2f& size) const override
  {
    vm::vec3f off = getOffset(camera);
    return vm::vec3f(off.x() - size.x() / 2.0f, off.y() - size.y(), off.z());
  }

  vm::vec3f position(const Camera& camera) const override
  {
    return camera.unproject(getOffset(camera));
  }

  vm::vec3f getOffset(const Camera& camera) const
  {
    const auto w = static_cast<float>(camera.viewport().width);
    const auto h = static_cast<float>(camera.viewport().height);
    return vm::vec3f(w / 2.0f, h - 20.0f, 0.0f);
  }
};

RenderService::RenderService(RenderContext& renderContext, RenderBatch& renderBatch)
  : m_renderContext(renderContext)
  , m_renderBatch(renderBatch)
  , m_textRenderer(std::make_unique<TextRenderer>(makeRenderServiceFont()))
  , m_pointHandleRenderer(std::make_unique<PointHandleRenderer>())
  , m_primitiveRenderer(std::make_unique<PrimitiveRenderer>())
  , m_foregroundColor(1.0f, 1.0f, 1.0f, 1.0f)
  , m_backgroundColor(0.0f, 0.0f, 0.0f, 1.0f)
  , m_lineWidth(1.0f)
  , m_occlusionPolicy(PrimitiveRendererOcclusionPolicy::Transparent)
  , m_cullingPolicy(PrimitiveRendererCullingPolicy::CullBackfaces)
{
}

RenderService::~RenderService()
{
  flush();
}

void RenderService::setForegroundColor(const Color& foregroundColor)
{
  m_foregroundColor = foregroundColor;
}

void RenderService::setBackgroundColor(const Color& backgroundColor)
{
  m_backgroundColor = backgroundColor;
}

void RenderService::setLineWidth(const float lineWidth)
{
  m_lineWidth = lineWidth;
}

void RenderService::setShowOccludedObjects()
{
  m_occlusionPolicy = PrimitiveRendererOcclusionPolicy::Show;
}

void RenderService::setShowOccludedObjectsTransparent()
{
  m_occlusionPolicy = PrimitiveRendererOcclusionPolicy::Transparent;
}

void RenderService::setHideOccludedObjects()
{
  m_occlusionPolicy = PrimitiveRendererOcclusionPolicy::Hide;
}

void RenderService::setShowBackfaces()
{
  m_cullingPolicy = PrimitiveRendererCullingPolicy::ShowBackfaces;
}

void RenderService::setCullBackfaces()
{
  m_cullingPolicy = PrimitiveRendererCullingPolicy::CullBackfaces;
}

void RenderService::renderString(const AttrString& string, const vm::vec3f& position)
{
  renderString(
    string, SimpleTextAnchor(position, TextAlignment::Bottom, vm::vec2f(0.0f, 16.0f)));
}

void RenderService::renderString(const AttrString& string, const TextAnchor& position)
{
  if (m_occlusionPolicy != PrimitiveRendererOcclusionPolicy::Hide)
  {
    m_textRenderer->renderStringOnTop(
      m_renderContext, m_foregroundColor, m_backgroundColor, string, position);
  }
  else
  {
    m_textRenderer->renderString(
      m_renderContext, m_foregroundColor, m_backgroundColor, string, position);
  }
}

void RenderService::renderHeadsUp(const AttrString& string)
{
  m_textRenderer->renderStringOnTop(
    m_renderContext, m_foregroundColor, m_backgroundColor, string, HeadsUpTextAnchor());
}

void RenderService::renderString(const std::string& string, const vm::vec3f& position)
{
  renderString(AttrString(string), position);
}

void RenderService::renderString(const std::string& string, const TextAnchor& position)
{
  renderString(AttrString(string), position);
}

void RenderService::renderHeadsUp(const std::string& string)
{
  renderHeadsUp(AttrString(string));
}

void RenderService::renderHandles(const std::vector<vm::vec3f>& positions)
{
  for (const vm::vec3f& position : positions)
    renderHandle(position);
}

void RenderService::renderHandle(const vm::vec3f& position)
{
  m_pointHandleRenderer->addPoint(m_foregroundColor, position);
}

void RenderService::renderHandleHighlight(const vm::vec3f& position)
{
  m_pointHandleRenderer->addHighlight(m_foregroundColor, position);
}

void RenderService::renderHandles(const std::vector<vm::segment3f>& positions)
{
  for (const vm::segment3f& position : positions)
    renderHandle(position);
}

void RenderService::renderHandle(const vm::segment3f& position)
{
  m_primitiveRenderer->renderLine(
    m_foregroundColor, m_lineWidth, m_occlusionPolicy, position.start(), position.end());
  renderHandle(position.center());
}

void RenderService::renderHandleHighlight(const vm::segment3f& position)
{
  m_primitiveRenderer->renderLine(
    m_foregroundColor,
    2.0f * m_lineWidth,
    m_occlusionPolicy,
    position.start(),
    position.end());
  renderHandleHighlight(position.center());
}

void RenderService::renderHandles(const std::vector<vm::polygon3f>& positions)
{
  for (const vm::polygon3f& position : positions)
    renderHandle(position);
}

void RenderService::renderHandle(const vm::polygon3f& position)
{
  setShowBackfaces();
  m_primitiveRenderer->renderFilledPolygon(
    mixAlpha(m_foregroundColor, 0.07f),
    m_occlusionPolicy,
    m_cullingPolicy,
    position.vertices());
  renderHandle(position.center());
  setCullBackfaces();
}

void RenderService::renderHandleHighlight(const vm::polygon3f& position)
{
  m_primitiveRenderer->renderPolygon(
    m_foregroundColor, 2.0f * m_lineWidth, m_occlusionPolicy, position.vertices());
  renderHandleHighlight(position.center());
}

void RenderService::renderLine(const vm::vec3f& start, const vm::vec3f& end)
{
  m_primitiveRenderer->renderLine(
    m_foregroundColor, m_lineWidth, m_occlusionPolicy, start, end);
}

void RenderService::renderLines(const std::vector<vm::vec3f>& positions)
{
  m_primitiveRenderer->renderLines(
    m_foregroundColor, m_lineWidth, m_occlusionPolicy, positions);
}

void RenderService::renderLineStrip(const std::vector<vm::vec3f>& positions)
{
  m_primitiveRenderer->renderLineStrip(
    m_foregroundColor, m_lineWidth, m_occlusionPolicy, positions);
}

void RenderService::renderCoordinateSystem(const vm::bbox3f& bounds)
{
  const Color& x = pref(Preferences::XAxisColor);
  const Color& y = pref(Preferences::YAxisColor);
  const Color& z = pref(Preferences::ZAxisColor);

  if (m_renderContext.render2D())
  {
    const Camera& camera = m_renderContext.camera();
    switch (vm::find_abs_max_component(camera.direction()))
    {
    case vm::axis::x:
      m_primitiveRenderer->renderCoordinateSystemYZ(
        y, z, m_lineWidth, m_occlusionPolicy, bounds);
      break;
    case vm::axis::y:
      m_primitiveRenderer->renderCoordinateSystemXZ(
        x, z, m_lineWidth, m_occlusionPolicy, bounds);
      break;
    default:
      m_primitiveRenderer->renderCoordinateSystemXY(
        x, y, m_lineWidth, m_occlusionPolicy, bounds);
      break;
    }
  }
  else
  {
    m_primitiveRenderer->renderCoordinateSystem3D(
      x, y, z, m_lineWidth, m_occlusionPolicy, bounds);
  }
}

void RenderService::renderPolygonOutline(const std::vector<vm::vec3f>& positions)
{
  m_primitiveRenderer->renderPolygon(
    m_foregroundColor, m_lineWidth, m_occlusionPolicy, positions);
}

void RenderService::renderFilledPolygon(const std::vector<vm::vec3f>& positions)
{
  m_primitiveRenderer->renderFilledPolygon(
    m_foregroundColor, m_occlusionPolicy, m_cullingPolicy, positions);
}

void RenderService::renderBounds(const vm::bbox3f& bounds)
{
  const vm::vec3f p1(bounds.min.x(), bounds.min.y(), bounds.min.z());
  const vm::vec3f p2(bounds.min.x(), bounds.min.y(), bounds.max.z());
  const vm::vec3f p3(bounds.min.x(), bounds.max.y(), bounds.min.z());
  const vm::vec3f p4(bounds.min.x(), bounds.max.y(), bounds.max.z());
  const vm::vec3f p5(bounds.max.x(), bounds.min.y(), bounds.min.z());
  const vm::vec3f p6(bounds.max.x(), bounds.min.y(), bounds.max.z());
  const vm::vec3f p7(bounds.max.x(), bounds.max.y(), bounds.min.z());
  const vm::vec3f p8(bounds.max.x(), bounds.max.y(), bounds.max.z());

  std::vector<vm::vec3f> positions;
  positions.reserve(12 * 2);
  positions.push_back(p1);
  positions.push_back(p2);
  positions.push_back(p1);
  positions.push_back(p3);
  positions.push_back(p1);
  positions.push_back(p5);
  positions.push_back(p2);
  positions.push_back(p4);
  positions.push_back(p2);
  positions.push_back(p6);
  positions.push_back(p3);
  positions.push_back(p4);
  positions.push_back(p3);
  positions.push_back(p7);
  positions.push_back(p4);
  positions.push_back(p8);
  positions.push_back(p5);
  positions.push_back(p6);
  positions.push_back(p5);
  positions.push_back(p7);
  positions.push_back(p6);
  positions.push_back(p8);
  positions.push_back(p7);
  positions.push_back(p8);

  renderLines(positions);
}

void RenderService::renderCircle(
  const vm::vec3f& position,
  const vm::axis::type normal,
  const size_t segments,
  const float radius,
  const vm::vec3f& startAxis,
  const vm::vec3f& endAxis)
{
  const std::pair<float, float> angles = startAngleAndLength(normal, startAxis, endAxis);
  renderCircle(position, normal, segments, radius, angles.first, angles.second);
}

void RenderService::renderCircle(
  const vm::vec3f& position,
  const vm::axis::type normal,
  const size_t segments,
  const float radius,
  const float startAngle,
  const float angleLength)
{
  const std::vector<vm::vec3f> positions =
    circle2D(radius, normal, startAngle, angleLength, segments) + position;
  m_primitiveRenderer->renderLineStrip(
    m_foregroundColor, m_lineWidth, m_occlusionPolicy, positions);
}

void RenderService::renderFilledCircle(
  const vm::vec3f& position,
  const vm::axis::type normal,
  const size_t segments,
  const float radius,
  const vm::vec3f& startAxis,
  const vm::vec3f& endAxis)
{
  const std::pair<float, float> angles = startAngleAndLength(normal, startAxis, endAxis);
  renderFilledCircle(position, normal, segments, radius, angles.first, angles.second);
}

void RenderService::renderFilledCircle(
  const vm::vec3f& position,
  const vm::axis::type normal,
  const size_t segments,
  const float radius,
  const float startAngle,
  const float angleLength)
{
  const std::vector<vm::vec3f> positions =
    circle2D(radius, normal, startAngle, angleLength, segments) + position;
  m_primitiveRenderer->renderFilledPolygon(
    m_foregroundColor, m_occlusionPolicy, m_cullingPolicy, positions);
}

void RenderService::flush()
{
  m_renderBatch.addOneShot(m_primitiveRenderer.release());
  m_renderBatch.addOneShot(m_pointHandleRenderer.release());
  m_renderBatch.addOneShot(m_textRenderer.release());
}
} // namespace Renderer
} // namespace TrenchBroom
