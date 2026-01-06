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

#include "mdl/UpdateBrushFaceAttributes.h"

#include "Macros.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h"

#include "kd/optional_utils.h"
#include "kd/range_utils.h"
#include "kd/ranges/to.h"
#include "kd/reflection_impl.h"

#include "vm/vec_io.h" // IWYU pragma: keep

#include <cassert>

namespace tb::mdl
{
namespace
{
auto replaceFlagsIfSet(const auto& maybeFlags)
{
  return maybeFlags
         | kdl::optional_transform([](const auto& flags) { return SetFlags{flags}; });
};

auto setValueIfSet(const auto& maybeValue)
{
  return maybeValue
         | kdl::optional_transform([](const auto& value) { return SetValue{value}; });
};

float normalizeRotation(const float rotation)
{
  return vm::mod(rotation, 360.0f);
}

void evaluate(const std::optional<AxisOp>& axisOp, BrushFace& brushFace)
{
  if (axisOp)
  {
    std::visit(
      kdl::overload(
        [&](const ResetAxis&) { brushFace.resetUVAxes(); },
        [&](const ToParaxial&) { brushFace.resetUVAxesToParaxial(); },
        [](const ToParallel&) {}),
      *axisOp);
  }
}

auto evaluate(const std::optional<ValueOp>& valueOp, const std::optional<float>& value)
{
  return valueOp ? std::visit(
                     kdl::overload(
                       [](const SetValue& setValue) { return setValue.value; },
                       [&](const AddValue& addValue) {
                         return value | kdl::optional_transform([&](const auto x) {
                                  return x + addValue.delta;
                                });
                       },
                       [&](const MultiplyValue& multiplyValue) {
                         return value | kdl::optional_transform([&](const auto x) {
                                  return x * multiplyValue.factor;
                                });
                       }),
                     *valueOp)
                 : value;
}

auto evaluate(const std::optional<FlagOp>& flagOp, const std::optional<int>& value)
{
  return flagOp ? std::visit(
                    kdl::overload(
                      [](const SetFlags& replaceFlags) { return replaceFlags.value; },
                      [&](const SetFlagBits& setFlagBits) {
                        return value | kdl::optional_transform([&](const auto x) {
                                 return x | setFlagBits.value;
                               });
                      },
                      [&](const ClearFlagBits& clearFlagBits) {
                        return value | kdl::optional_transform([&](const auto x) {
                                 return x & ~clearFlagBits.value;
                               });
                      }),
                    *flagOp)
                : value;
}

float normalizeAngle(const float angleInDegrees)
{
  return vm::correct(vm::mod(angleInDegrees, 360.0f));
}

} // namespace

kdl_reflect_impl(ResetAxis);
kdl_reflect_impl(ToParaxial);
kdl_reflect_impl(ToParallel);

std::ostream& operator<<(std::ostream& lhs, const AxisOp& rhs)
{
  std::visit([&](const auto& x) { lhs << x; }, rhs);
  return lhs;
}

kdl_reflect_impl(SetValue);
kdl_reflect_impl(AddValue);
kdl_reflect_impl(MultiplyValue);

std::ostream& operator<<(std::ostream& lhs, const ValueOp& rhs)
{
  std::visit([&](const auto& x) { lhs << x; }, rhs);
  return lhs;
}

kdl_reflect_impl(SetFlags);
kdl_reflect_impl(SetFlagBits);
kdl_reflect_impl(ClearFlagBits);

std::ostream& operator<<(std::ostream& lhs, const FlagOp& rhs)
{
  std::visit([&](const auto& x) { lhs << x; }, rhs);
  return lhs;
}

kdl_reflect_impl(UpdateBrushFaceAttributes);

UpdateBrushFaceAttributes copyAll(const BrushFaceAttributes& attributes)
{
  auto result = copyAllExceptContentFlags(attributes);
  result.surfaceContents = replaceFlagsIfSet(attributes.surfaceContents());
  return result;
}

UpdateBrushFaceAttributes copyAllExceptContentFlags(const BrushFaceAttributes& attributes)
{
  return UpdateBrushFaceAttributes{
    .materialName = attributes.materialName(),
    .xOffset = SetValue{attributes.xOffset()},
    .yOffset = SetValue{attributes.yOffset()},
    .rotation = SetValue{attributes.rotation()},
    .xScale = SetValue{attributes.xScale()},
    .yScale = SetValue{attributes.yScale()},
    .surfaceFlags = replaceFlagsIfSet(attributes.surfaceFlags()),
    .surfaceValue = setValueIfSet(attributes.surfaceValue()),
    .color = attributes.color(),
  };
}

UpdateBrushFaceAttributes resetAll(const BrushFaceAttributes& defaultFaceAttributes)
{
  return UpdateBrushFaceAttributes{
    .xOffset = SetValue{0.0f},
    .yOffset = SetValue{0.0f},
    .rotation = SetValue{0.0f},
    .xScale = SetValue{defaultFaceAttributes.scale().x()},
    .yScale = SetValue{defaultFaceAttributes.scale().y()},
    .axis = ResetAxis{},
  };
}

UpdateBrushFaceAttributes resetAllToParaxial(
  const BrushFaceAttributes& defaultFaceAttributes)
{
  return UpdateBrushFaceAttributes{
    .xOffset = SetValue{0.0f},
    .yOffset = SetValue{0.0f},
    .rotation = SetValue{0.0f},
    .xScale = SetValue{defaultFaceAttributes.scale().x()},
    .yScale = SetValue{defaultFaceAttributes.scale().y()},
    .axis = ToParaxial{},
  };
}

std::ostream& operator<<(std::ostream& lhs, const UvPolicy rhs)
{
  switch (rhs)
  {
  case UvPolicy::best:
    lhs << "best";
    break;
  case UvPolicy::next:
    lhs << "next";
    break;
  case UvPolicy::prev:
    lhs << "previous";
    break;
  }
  return lhs;
}

UpdateBrushFaceAttributes align(const BrushFace& brushFace, const UvPolicy uvPolicy)
{
  constexpr auto uAxis = vm::vec2d{1, 0};

  const auto dot = [&](const auto& v) { return vm::dot(v, uAxis); };

  const auto edgeVecs =
    brushFace.geometry()->boundary()
    | std::views::transform(
      [toUV = brushFace.toUVCoordSystemMatrix(
         brushFace.attributes().offset(), vm::vec2f{1, 1})](const auto* halfEdge) {
        const auto start = vm::vec2d{toUV * halfEdge->origin()->position()};
        const auto end = vm::vec2d{toUV * halfEdge->next()->origin()->position()};
        return vm::normalize(end - start);
      })
    | kdl::ranges::to<std::vector>();

  // find the edge vec that is closest to the U axis
  const auto iBestMatch = std::ranges::max_element(edgeVecs, std::less<double>{}, dot);
  contract_assert(iBestMatch != std::ranges::end(edgeVecs));

  const auto isExactMatch = vm::is_equal(dot(*iBestMatch), 1.0, vm::Cd::almost_zero());
  const auto iEdgeToAlignTo = [&] {
    switch (uvPolicy)
    {
    case UvPolicy::best:
      return iBestMatch;
    case UvPolicy::next:
      return isExactMatch ? kdl::succ(edgeVecs, iBestMatch) : iBestMatch;
    case UvPolicy::prev:
      return isExactMatch ? kdl::pred(edgeVecs, iBestMatch) : iBestMatch;
      switchDefault();
    }
  }();

  const auto angleInDegrees =
    brushFace.measureUVAngle(vm::vec2f{0, 0}, vm::vec2f{*iEdgeToAlignTo});

  return {
    .rotation = SetValue{normalizeAngle(angleInDegrees)},
  };
}

void evaluate(const UpdateBrushFaceAttributes& update, BrushFace& brushFace)
{
  auto attributes = brushFace.attributes();

  attributes.setMaterialName(update.materialName.value_or(attributes.materialName()));
  attributes.setXOffset(*evaluate(update.xOffset, attributes.xOffset()));
  attributes.setYOffset(*evaluate(update.yOffset, attributes.yOffset()));
  attributes.setRotation(
    normalizeRotation(*evaluate(update.rotation, attributes.rotation())));
  attributes.setXScale(*evaluate(update.xScale, attributes.xScale()));
  attributes.setYScale(*evaluate(update.yScale, attributes.yScale()));

  if (update.surfaceFlags)
  {
    attributes.setSurfaceFlags(
      evaluate(update.surfaceFlags, brushFace.resolvedSurfaceFlags()));
  }

  if (update.surfaceContents)
  {
    attributes.setSurfaceContents(
      evaluate(update.surfaceContents, brushFace.resolvedSurfaceContents()));
  }

  if (update.surfaceValue)
  {
    attributes.setSurfaceValue(
      evaluate(update.surfaceValue, brushFace.resolvedSurfaceValue()));
  }

  if (update.color)
  {
    attributes.setColor(
      *update.color | kdl::optional_or_else([&]() { return brushFace.resolvedColor(); }));
  }

  brushFace.setAttributes(attributes);
  evaluate(update.axis, brushFace);
}

} // namespace tb::mdl
