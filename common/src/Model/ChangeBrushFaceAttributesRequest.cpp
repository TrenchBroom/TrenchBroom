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

#include "ChangeBrushFaceAttributesRequest.h"

#include "Macros.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushNode.h"

#include <cassert>
#include <string>

namespace tb::Model
{

namespace
{
template <typename T>
T evaluateValueOp(
  const T oldValue, const T newValue, const ChangeBrushFaceAttributesRequest::ValueOp op)
{
  switch (op)
  {
  case ChangeBrushFaceAttributesRequest::ValueOp::Set:
    return newValue;
  case ChangeBrushFaceAttributesRequest::ValueOp::Add:
    return oldValue + newValue;
  case ChangeBrushFaceAttributesRequest::ValueOp::Mul:
    return oldValue * newValue;
  case ChangeBrushFaceAttributesRequest::ValueOp::None:
    return oldValue;
    switchDefault();
  }
}

template <typename T>
std::optional<T> evaluateValueOp(
  const std::optional<T>& oldValue,
  const T oldValueFallback,
  const std::optional<T>& newValue,
  const ChangeBrushFaceAttributesRequest::ValueOp op)
{
  switch (op)
  {
  case ChangeBrushFaceAttributesRequest::ValueOp::Set:
    return newValue;
  case ChangeBrushFaceAttributesRequest::ValueOp::Add:
    return oldValue.value_or(oldValueFallback) + newValue.value_or(T{});
  case ChangeBrushFaceAttributesRequest::ValueOp::Mul:
    return oldValue.value_or(oldValueFallback) * newValue.value_or(T{});
  case ChangeBrushFaceAttributesRequest::ValueOp::None:
    return oldValue;
    switchDefault();
  }
}

template <typename T>
T evaluateFlagOp(
  const T oldValue, const T newValue, const ChangeBrushFaceAttributesRequest::FlagOp op)
{
  switch (op)
  {
  case ChangeBrushFaceAttributesRequest::FlagOp::Replace:
    return newValue;
  case ChangeBrushFaceAttributesRequest::FlagOp::Set:
    return oldValue | newValue;
  case ChangeBrushFaceAttributesRequest::FlagOp::Unset:
    return oldValue & ~newValue;
  case ChangeBrushFaceAttributesRequest::FlagOp::None:
    return oldValue;
    switchDefault();
  }
}

template <typename T>
std::optional<T> evaluateFlagOp(
  const std::optional<T>& oldValue,
  const T oldValueFallback,
  const std::optional<T>& newValue,
  const ChangeBrushFaceAttributesRequest::FlagOp op)
{
  switch (op)
  {
  case ChangeBrushFaceAttributesRequest::FlagOp::Replace:
    return newValue;
  case ChangeBrushFaceAttributesRequest::FlagOp::Set:
    return oldValue.value_or(oldValueFallback) | newValue.value_or(T{});
  case ChangeBrushFaceAttributesRequest::FlagOp::Unset:
    return oldValue.value_or(oldValueFallback) & ~newValue.value_or(T{});
  case ChangeBrushFaceAttributesRequest::FlagOp::None:
    return oldValue;
    switchDefault();
  }
}
} // namespace

ChangeBrushFaceAttributesRequest::ChangeBrushFaceAttributesRequest() = default;

void ChangeBrushFaceAttributesRequest::clear()
{
  *this = ChangeBrushFaceAttributesRequest{};
}

const std::string ChangeBrushFaceAttributesRequest::name() const
{
  return "Change Face Attributes";
}

bool ChangeBrushFaceAttributesRequest::evaluate(BrushFace& brushFace) const
{
  auto result = false;

  BrushFaceAttributes attributes = brushFace.attributes();

  switch (m_materialOp)
  {
  case MaterialOp::Set:
    result |= attributes.setMaterialName(m_materialName);
    break;
  case MaterialOp::None:
    break;
    switchDefault();
  }

  result |=
    attributes.setXOffset(evaluateValueOp(attributes.xOffset(), m_xOffset, m_xOffsetOp));
  result |=
    attributes.setYOffset(evaluateValueOp(attributes.yOffset(), m_yOffset, m_yOffsetOp));
  result |= attributes.setRotation(
    evaluateValueOp(attributes.rotation(), m_rotation, m_rotationOp));
  result |=
    attributes.setXScale(evaluateValueOp(attributes.xScale(), m_xScale, m_xScaleOp));
  result |=
    attributes.setYScale(evaluateValueOp(attributes.yScale(), m_yScale, m_yScaleOp));
  result |= attributes.setSurfaceFlags(evaluateFlagOp(
    attributes.surfaceFlags(),
    brushFace.resolvedSurfaceFlags(),
    m_surfaceFlags,
    m_surfaceFlagsOp));
  result |= attributes.setSurfaceContents(evaluateFlagOp(
    attributes.surfaceContents(),
    brushFace.resolvedSurfaceContents(),
    m_contentFlags,
    m_contentFlagsOp));
  result |= attributes.setSurfaceValue(evaluateValueOp(
    attributes.surfaceValue(),
    brushFace.resolvedSurfaceValue(),
    m_surfaceValue,
    m_surfaceValueOp));
  result |= attributes.setColor(evaluateValueOp(
    attributes.color(), brushFace.resolvedColor(), m_colorValue, m_colorValueOp));

  brushFace.setAttributes(attributes);

  switch (m_axisOp)
  {
  case AxisOp::Reset:
    brushFace.resetUVAxes();
    result |= true;
    break;
  case AxisOp::None:
    break;
  case AxisOp::ToParaxial:
    brushFace.resetUVAxesToParaxial();
    result |= true;
    break;
  case AxisOp::ToParallel:
    break;
    switchDefault();
  }

  return result;
}

void ChangeBrushFaceAttributesRequest::resetAll(
  const BrushFaceAttributes& defaultFaceAttributes)
{
  resetUVAxes();
  setOffset(vm::vec2f{0, 0});
  setRotation(0.0f);
  setScale(defaultFaceAttributes.scale());
}

void ChangeBrushFaceAttributesRequest::resetAllToParaxial(
  const BrushFaceAttributes& defaultFaceAttributes)
{
  resetUVAxesToParaxial();
  setOffset(vm::vec2f{0, 0});
  setRotation(0.0f);
  setScale(defaultFaceAttributes.scale());
}

void ChangeBrushFaceAttributesRequest::setMaterialName(const std::string& materialName)
{
  m_materialName = materialName;
  m_materialOp = MaterialOp::Set;
}

void ChangeBrushFaceAttributesRequest::resetUVAxes()
{
  m_axisOp = AxisOp::Reset;
}

void ChangeBrushFaceAttributesRequest::resetUVAxesToParaxial()
{
  m_axisOp = AxisOp::ToParaxial;
}

void ChangeBrushFaceAttributesRequest::resetUVAxesToParallel()
{
  m_axisOp = AxisOp::ToParallel;
}

void ChangeBrushFaceAttributesRequest::setOffset(const vm::vec2f& offset)
{
  setXOffset(offset.x());
  setYOffset(offset.y());
}

void ChangeBrushFaceAttributesRequest::addOffset(const vm::vec2f& offset)
{
  addXOffset(offset.x());
  addYOffset(offset.y());
}

void ChangeBrushFaceAttributesRequest::mulOffset(const vm::vec2f& offset)
{
  mulXOffset(offset.x());
  mulYOffset(offset.y());
}

void ChangeBrushFaceAttributesRequest::setXOffset(const float xOffset)
{
  m_xOffset = xOffset;
  m_xOffsetOp = ValueOp::Set;
}

void ChangeBrushFaceAttributesRequest::addXOffset(const float xOffset)
{
  m_xOffset = xOffset;
  m_xOffsetOp = ValueOp::Add;
}

void ChangeBrushFaceAttributesRequest::mulXOffset(const float xOffset)
{
  m_xOffset = xOffset;
  m_xOffsetOp = ValueOp::Mul;
}

void ChangeBrushFaceAttributesRequest::setYOffset(const float yOffset)
{
  m_yOffset = yOffset;
  m_yOffsetOp = ValueOp::Set;
}

void ChangeBrushFaceAttributesRequest::addYOffset(const float yOffset)
{
  m_yOffset = yOffset;
  m_yOffsetOp = ValueOp::Add;
}

void ChangeBrushFaceAttributesRequest::mulYOffset(const float yOffset)
{
  m_yOffset = yOffset;
  m_yOffsetOp = ValueOp::Mul;
}

void ChangeBrushFaceAttributesRequest::setRotation(const float rotation)
{
  m_rotation = rotation;
  m_rotationOp = ValueOp::Set;
}

void ChangeBrushFaceAttributesRequest::addRotation(const float rotation)
{
  m_rotation = rotation;
  m_rotationOp = ValueOp::Add;
}

void ChangeBrushFaceAttributesRequest::mulRotation(const float rotation)
{
  m_rotation = rotation;
  m_rotationOp = ValueOp::Mul;
}

void ChangeBrushFaceAttributesRequest::setScale(const vm::vec2f& scale)
{
  setXScale(scale.x());
  setYScale(scale.y());
}

void ChangeBrushFaceAttributesRequest::addScale(const vm::vec2f& scale)
{
  addXScale(scale.x());
  addYScale(scale.y());
}

void ChangeBrushFaceAttributesRequest::mulScale(const vm::vec2f& scale)
{
  mulXScale(scale.x());
  mulYScale(scale.y());
}

void ChangeBrushFaceAttributesRequest::setXScale(const float xScale)
{
  m_xScale = xScale;
  m_xScaleOp = ValueOp::Set;
}

void ChangeBrushFaceAttributesRequest::addXScale(const float xScale)
{
  m_xScale = xScale;
  m_xScaleOp = ValueOp::Add;
}

void ChangeBrushFaceAttributesRequest::mulXScale(const float xScale)
{
  m_xScale = xScale;
  m_xScaleOp = ValueOp::Mul;
}

void ChangeBrushFaceAttributesRequest::setYScale(const float yScale)
{
  m_yScale = yScale;
  m_yScaleOp = ValueOp::Set;
}

void ChangeBrushFaceAttributesRequest::addYScale(const float yScale)
{
  m_yScale = yScale;
  m_yScaleOp = ValueOp::Add;
}

void ChangeBrushFaceAttributesRequest::mulYScale(const float yScale)
{
  m_yScale = yScale;
  m_yScaleOp = ValueOp::Mul;
}

void ChangeBrushFaceAttributesRequest::setSurfaceFlags(const int surfaceFlags)
{
  m_surfaceFlags = surfaceFlags;
  m_surfaceFlagsOp = FlagOp::Set;
}

void ChangeBrushFaceAttributesRequest::unsetSurfaceFlags(const int surfaceFlags)
{
  m_surfaceFlags = surfaceFlags;
  m_surfaceFlagsOp = FlagOp::Unset;
}

void ChangeBrushFaceAttributesRequest::replaceSurfaceFlags(
  const std::optional<int>& surfaceFlags)
{
  m_surfaceFlags = surfaceFlags;
  m_surfaceFlagsOp = FlagOp::Replace;
}

void ChangeBrushFaceAttributesRequest::setContentFlags(const int contentFlags)
{
  m_contentFlags = contentFlags;
  m_contentFlagsOp = FlagOp::Set;
}

void ChangeBrushFaceAttributesRequest::unsetContentFlags(const int contentFlags)
{
  m_contentFlags = contentFlags;
  m_contentFlagsOp = FlagOp::Unset;
}

void ChangeBrushFaceAttributesRequest::replaceContentFlags(
  const std::optional<int>& contentFlags)
{
  m_contentFlags = contentFlags;
  m_contentFlagsOp = FlagOp::Replace;
}

void ChangeBrushFaceAttributesRequest::setSurfaceValue(
  const std::optional<float>& surfaceValue)
{
  m_surfaceValue = surfaceValue;
  m_surfaceValueOp = ValueOp::Set;
}

void ChangeBrushFaceAttributesRequest::addSurfaceValue(const float surfaceValue)
{
  m_surfaceValue = surfaceValue;
  m_surfaceValueOp = ValueOp::Add;
}

void ChangeBrushFaceAttributesRequest::mulSurfaceValue(const float surfaceValue)
{
  m_surfaceValue = surfaceValue;
  m_surfaceValueOp = ValueOp::Mul;
}

void ChangeBrushFaceAttributesRequest::setColor(const std::optional<Color>& colorValue)
{
  m_colorValue = colorValue;
  m_colorValueOp = ValueOp::Set;
}

void ChangeBrushFaceAttributesRequest::setAll(const Model::BrushFace& face)
{
  setAll(face.attributes());
}

void ChangeBrushFaceAttributesRequest::setAllExceptContentFlags(
  const Model::BrushFace& face)
{
  setAllExceptContentFlags(face.attributes());
}

void ChangeBrushFaceAttributesRequest::setAll(
  const Model::BrushFaceAttributes& attributes)
{
  setAllExceptContentFlags(attributes);
  replaceContentFlags(attributes.surfaceContents());
}

void ChangeBrushFaceAttributesRequest::setAllExceptContentFlags(
  const Model::BrushFaceAttributes& attributes)
{
  setMaterialName(attributes.materialName());
  setXOffset(attributes.xOffset());
  setYOffset(attributes.yOffset());
  setRotation(attributes.rotation());
  setXScale(attributes.xScale());
  setYScale(attributes.yScale());
  replaceSurfaceFlags(attributes.surfaceFlags());
  setSurfaceValue(attributes.surfaceValue());
  setColor(attributes.color());
}

} // namespace tb::Model
