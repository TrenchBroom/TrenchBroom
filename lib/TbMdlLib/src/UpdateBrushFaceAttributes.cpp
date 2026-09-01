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

#include "mdl/BrushFace.h"
#include "mdl/SurfaceAttributes.h"
#include "mdl/UvAttributes.h"

#include "kd/optional_utils.h"
#include "kd/reflection_impl.h"
#include "kd/result.h"

#include "vm/scalar.h"
#include "vm/vec_io.h" // IWYU pragma: keep

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

void evaluate(const std::optional<AxisOp>& axisOp, BrushFace& brushFace)
{
  if (axisOp)
  {
    std::visit(
      kdl::overload(
        [&](const ResetAxis&) { brushFace.resetUvAxes(); },
        [&](const ToParaxial&) { brushFace.resetUvAxesToParaxial(); },
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

UpdateBrushFaceAttributes copyAll(const BrushFace& brushFace)
{
  auto result = copyAllExceptContentFlags(brushFace);
  result.surfaceContents = replaceFlagsIfSet(brushFace.surfaceAttributes().contents);
  return result;
}

UpdateBrushFaceAttributes copyAllExceptContentFlags(const BrushFace& brushFace)
{
  const auto& uvAttributes = brushFace.uvAttributes();
  const auto& surfaceAttributes = brushFace.surfaceAttributes();
  return UpdateBrushFaceAttributes{
    .materialName = brushFace.materialName(),
    .xOffset = SetValue{uvAttributes.offset.x()},
    .yOffset = SetValue{uvAttributes.offset.y()},
    .rotation = SetValue{uvAttributes.rotation},
    .xScale = SetValue{uvAttributes.scale.x()},
    .yScale = SetValue{uvAttributes.scale.y()},
    .surfaceFlags = replaceFlagsIfSet(surfaceAttributes.flags),
    .surfaceValue = setValueIfSet(surfaceAttributes.value),
    .color = surfaceAttributes.color,
  };
}

UpdateBrushFaceAttributes resetAll(const UvAttributes& defaultUvAttributes)
{
  return UpdateBrushFaceAttributes{
    .xOffset = SetValue{0.0f},
    .yOffset = SetValue{0.0f},
    .rotation = SetValue{0.0f},
    .xScale = SetValue{defaultUvAttributes.scale.x()},
    .yScale = SetValue{defaultUvAttributes.scale.y()},
    .axis = ResetAxis{},
  };
}

UpdateBrushFaceAttributes resetAllToParaxial(const UvAttributes& defaultUvAttributes)
{
  return UpdateBrushFaceAttributes{
    .xOffset = SetValue{0.0f},
    .yOffset = SetValue{0.0f},
    .rotation = SetValue{0.0f},
    .xScale = SetValue{defaultUvAttributes.scale.x()},
    .yScale = SetValue{defaultUvAttributes.scale.y()},
    .axis = ToParaxial{},
  };
}

Result<void> evaluate(const UpdateBrushFaceAttributes& update, BrushFace& brushFace)
{
  // validate the (possibly invalid) UV update before touching anything else, so that a
  // rejected update leaves the whole face -- not just its UV attributes -- unchanged
  const auto& uvAttributes = brushFace.uvAttributes();
  return brushFace.setUvAttributes(UvAttributes{
           .offset =
             {*evaluate(update.xOffset, uvAttributes.offset.x()),
              *evaluate(update.yOffset, uvAttributes.offset.y())},
           .scale =
             {*evaluate(update.xScale, uvAttributes.scale.x()),
              *evaluate(update.yScale, uvAttributes.scale.y())},
           .rotation =
             vm::normalize_degrees(*evaluate(update.rotation, uvAttributes.rotation)),
         })
         | kdl::transform([&]() {
             if (update.materialName)
             {
               brushFace.setMaterialName(*update.materialName);
             }

             auto surfaceAttributes = brushFace.surfaceAttributes();

             if (update.surfaceFlags)
             {
               surfaceAttributes.flags =
                 evaluate(update.surfaceFlags, brushFace.resolvedSurfaceFlags());
             }

             if (update.surfaceContents)
             {
               surfaceAttributes.contents =
                 evaluate(update.surfaceContents, brushFace.resolvedSurfaceContents());
             }

             if (update.surfaceValue)
             {
               surfaceAttributes.value =
                 evaluate(update.surfaceValue, brushFace.resolvedSurfaceValue());
             }

             if (update.color)
             {
               surfaceAttributes.color = *update.color;
             }

             brushFace.setSurfaceAttributes(surfaceAttributes);

             evaluate(update.axis, brushFace);
           });
}

} // namespace tb::mdl
