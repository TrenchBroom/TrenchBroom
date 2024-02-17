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

#include "vecmath/constants.h"
#include "vecmath/forward.h"
#include "vecmath/util.h"

#include <memory>
#include <vector>

namespace TrenchBroom
{
namespace Renderer
{
class AttrString;
class PointHandleRenderer;
class PrimitiveRenderer;
enum class PrimitiveRendererCullingPolicy;
enum class PrimitiveRendererOcclusionPolicy;
class RenderBatch;
class RenderContext;
class TextAnchor;
class TextRenderer;

class RenderService
{
private:
  using OcclusionPolicy = PrimitiveRendererOcclusionPolicy;
  using CullingPolicy = PrimitiveRendererCullingPolicy;
  class HeadsUpTextAnchor;

  RenderContext& m_renderContext;
  RenderBatch& m_renderBatch;
  std::unique_ptr<TextRenderer> m_textRenderer;
  std::unique_ptr<PointHandleRenderer> m_pointHandleRenderer;
  std::unique_ptr<PrimitiveRenderer> m_primitiveRenderer;

  Color m_foregroundColor;
  Color m_backgroundColor;
  float m_lineWidth;
  OcclusionPolicy m_occlusionPolicy;
  CullingPolicy m_cullingPolicy;

public:
  RenderService(RenderContext& renderContext, RenderBatch& renderBatch);
  ~RenderService();

  deleteCopyAndMove(RenderService);

  void setForegroundColor(const Color& foregroundColor);
  void setBackgroundColor(const Color& backgroundColor);
  void setLineWidth(float lineWidth);

  void setShowOccludedObjects();
  void setShowOccludedObjectsTransparent();
  void setHideOccludedObjects();

  void setShowBackfaces();
  void setCullBackfaces();

  void renderString(const AttrString& string, const vm::vec3f& position);
  void renderString(const AttrString& string, const TextAnchor& position);
  void renderHeadsUp(const AttrString& string);

  void renderString(const std::string& string, const vm::vec3f& position);
  void renderString(const std::string& string, const TextAnchor& position);
  void renderHeadsUp(const std::string& string);

  void renderHandles(const std::vector<vm::vec3f>& positions);
  void renderHandle(const vm::vec3f& position);
  void renderHandleHighlight(const vm::vec3f& position);

  void renderHandles(const std::vector<vm::segment3f>& positions);
  void renderHandle(const vm::segment3f& position);
  void renderHandleHighlight(const vm::segment3f& position);

  void renderHandles(const std::vector<vm::polygon3f>& positions);
  void renderHandle(const vm::polygon3f& position);
  void renderHandleHighlight(const vm::polygon3f& position);

  void renderLine(const vm::vec3f& start, const vm::vec3f& end);
  void renderLines(const std::vector<vm::vec3f>& positions);
  void renderLineStrip(const std::vector<vm::vec3f>& positions);
  void renderCoordinateSystem(const vm::bbox3f& bounds);

  void renderPolygonOutline(const std::vector<vm::vec3f>& positions);
  void renderFilledPolygon(const std::vector<vm::vec3f>& positions);

  void renderBounds(const vm::bbox3f& bounds);

  void renderCircle(
    const vm::vec3f& position,
    vm::axis::type normal,
    size_t segments,
    float radius,
    const vm::vec3f& startAxis,
    const vm::vec3f& endAxis);
  void renderCircle(
    const vm::vec3f& position,
    vm::axis::type normal,
    size_t segments,
    float radius,
    float startAngle = 0.0f,
    float angleLength = vm::Cf::two_pi());

  void renderFilledCircle(
    const vm::vec3f& position,
    vm::axis::type normal,
    size_t segments,
    float radius,
    const vm::vec3f& startAxis,
    const vm::vec3f& endAxis);
  void renderFilledCircle(
    const vm::vec3f& position,
    vm::axis::type normal,
    size_t segments,
    float radius,
    float startAngle = 0.0f,
    float angleLength = vm::Cf::two_pi());

private:
  void flush();
};
} // namespace Renderer
} // namespace TrenchBroom
