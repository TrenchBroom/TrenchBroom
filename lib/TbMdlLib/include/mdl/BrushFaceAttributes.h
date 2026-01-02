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

#include "kd/reflection_decl.h"

#include "vm/vec.h"

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace tb::mdl
{

class BrushFaceAttributes
{
public:
  static const std::string NoMaterialName;

  // SiN
  static constexpr float SiNDefaultNonLitValue = 0.5f;
  static constexpr float SiNDefaultFriction = 1.0f;
  static constexpr float SiNDefaultAnimTime = 0.2f;

  // SiN extended
  static constexpr float SiNDefaultExtDirectScale = 1.0f;
  static constexpr float SiNDefaultExtPatchScale = 1.0f;
  static constexpr float SiNDefaultExtMaxLight = 1.0f;
  static constexpr float SiNDefaultExtLuxelScale = 1.0f;
  static constexpr float SiNDefaultExtMottle = 1.0f;

private:
  std::string m_materialName;

  vm::vec2f m_offset = vm::vec2f{0, 0};
  vm::vec2f m_scale = vm::vec2f{1, 1};
  float m_rotation = 0.0f;

  std::optional<int> m_surfaceContents;
  std::optional<int> m_surfaceFlags;
  std::optional<float> m_surfaceValue;

  std::optional<Color> m_color;

  // SiN stuff
  std::optional<float> m_nonlitvalue;// = 0.5f;
  std::optional<int> m_trans_angle;
  std::optional<float> m_trans_mag;
  std::optional<float> m_translucence;
  std::optional<float> m_restitution;
  std::optional<float> m_friction;// = 1.0f;
  std::optional<float> m_animtime;// = 0.2f;
  
  std::optional<std::string> m_directstyle;
  std::optional<float> m_direct;
  std::optional<float> m_directangle;

  // SiN extended
  std::optional<float> m_ext_directscale;// = 1.0f;
  std::optional<float> m_ext_patchscale;// = 1.0f;
  std::optional<float> m_ext_minlight;
  std::optional<float> m_ext_maxlight;// = 1.0f;
  std::optional<float> m_ext_luxel_scale;// = 1.0f;
  std::optional<float> m_ext_mottle;// = 1.0f;
  std::optional<int>  m_ext_flags;

public:
  explicit BrushFaceAttributes(std::string_view materialName);
  BrushFaceAttributes(std::string_view materialName, const BrushFaceAttributes& other);

  kdl_reflect_decl(
    BrushFaceAttributes,
    m_materialName,
    m_offset,
    m_scale,
    m_rotation,
    m_surfaceContents,
    m_surfaceFlags,
    m_surfaceValue,
    m_color,
    // SiN
    m_nonlitvalue,
    m_trans_angle,
    m_trans_mag,
    m_translucence,
    m_restitution,
    m_friction,
    m_animtime,
    m_directstyle,
    m_direct,
    m_directangle,
    m_ext_directscale,
    m_ext_patchscale,
    m_ext_minlight,
    m_ext_maxlight,
    m_ext_luxel_scale,
    m_ext_mottle,
    m_ext_flags
  );

  const std::string& materialName() const;

  const vm::vec2f& offset() const;
  float xOffset() const;
  float yOffset() const;
  vm::vec2f modOffset(const vm::vec2f& offset, const vm::vec2f& size) const;

  const vm::vec2f& scale() const;
  float xScale() const;
  float yScale() const;

  float rotation() const;

  bool hasSurfaceAttributes() const;
  const std::optional<int>& surfaceContents() const;
  const std::optional<int>& surfaceFlags() const;
  const std::optional<float>& surfaceValue() const;

  bool hasColor() const;
  const std::optional<Color>& color() const;

  bool valid() const;

  bool setMaterialName(const std::string& materialName);
  bool setOffset(const vm::vec2f& offset);
  bool setXOffset(float xOffset);
  bool setYOffset(float yOffset);
  bool setScale(const vm::vec2f& scale);
  bool setXScale(float xScale);
  bool setYScale(float yScale);
  bool setRotation(float rotation);
  bool setSurfaceContents(const std::optional<int>& surfaceContents);
  bool setSurfaceFlags(const std::optional<int>& surfaceFlags);
  bool setSurfaceValue(const std::optional<float>& surfaceValue);
  bool setColor(const std::optional<Color>& color);

  // SiN
  bool hasSiNNonlitValue() const;
  const std::optional<float>& sinNonlitValue() const;

  bool hasSiNTransAngle() const;
  const std::optional<int>& sinTransAngle() const;

  bool hasSiNTransMag() const;
  const std::optional<float>& sinTransMag() const;

  bool hasSiNTranslucence() const;
  const std::optional<float>& sinTranslucence() const;

  bool hasSiNRestitution() const;
  const std::optional<float>& sinRestitution() const;

  bool hasSiNFriction() const;
  const std::optional<float>& sinFriction() const;

  bool hasSiNAnimTime() const;
  const std::optional<float>& sinAnimTime() const;

  bool hasSiNDirectStyle() const;
  const std::optional<std::string>& sinDirectStyle() const;

  bool hasSiNDirect() const;
  const std::optional<float>& sinDirect() const;

  bool hasSiNDirectAngle() const;
  const std::optional<float>& sinDirectAngle() const;

  bool setSiNNonlitValue(const std::optional<float>& value);
  bool setSiNTransAngle(const std::optional<int>& value);
  bool setSiNTransMag(const std::optional<float>& value);
  bool setSiNTranslucence(const std::optional<float>& value);
  bool setSiNRestitution(const std::optional<float>& value);
  bool setSiNFriction(const std::optional<float>& value);
  bool setSiNAnimTime(const std::optional<float>& value);
  bool setSiNDirectStyle(const std::optional<std::string>& value);
  bool setSiNDirect(const std::optional<float>& value);
  bool setSiNDirectAngle(const std::optional<float>& value);

  // extended
  bool hasSiNExtDirectScale() const;
  const std::optional<float>& sinExtDirectScale() const;

  bool hasSiNExtPatchScale() const;
  const std::optional<float>& sinExtPatchScale() const;

  bool hasSiNExtMinLight() const;
  const std::optional<float>& sinExtMinLight() const;

  bool hasSiNExtMaxLight() const;
  const std::optional<float>& sinExtMaxLight() const;

  bool hasSiNExtLuxelScale() const;
  const std::optional<float>& sinExtLuxelScale() const;

  bool hasSiNExtMottle() const;
  const std::optional<float>& sinExtMottle() const;
  
  bool setSiNExtDirectScale(const std::optional<float>& value);
  bool setSiNExtPatchScale(const std::optional<float>& value);
  bool setSiNExtMinLight(const std::optional<float>& value);
  bool setSiNExtMaxLight(const std::optional<float>& value);
  bool setSiNExtLuxelScale(const std::optional<float>& value);
  bool setSiNExtMottle(const std::optional<float>& value);

  const std::optional<int>& extendedFlags() const;
  bool setExtendedFlags(const std::optional<int>& extendedFlags);
};

} // namespace tb::mdl
