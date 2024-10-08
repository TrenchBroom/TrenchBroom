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

#include "Quake3Shader.h"

#include "kdl/reflection_impl.h"

#include <string>

namespace tb::mdl
{
const std::string Quake3ShaderStage::BlendFunc::One = "GL_ONE";
const std::string Quake3ShaderStage::BlendFunc::Zero = "GL_ZERO";
const std::string Quake3ShaderStage::BlendFunc::SrcColor = "GL_SRC_COLOR";
const std::string Quake3ShaderStage::BlendFunc::DestColor = "GL_DST_COLOR";
const std::string Quake3ShaderStage::BlendFunc::OneMinusSrcColor =
  "GL_ONE_MINUS_SRC_COLOR";
const std::string Quake3ShaderStage::BlendFunc::OneMinusDestColor =
  "GL_ONE_MINUS_DST_COLOR";
const std::string Quake3ShaderStage::BlendFunc::SrcAlpha = "GL_SRC_ALPHA";
const std::string Quake3ShaderStage::BlendFunc::DestAlpha = "GL_DST_ALPHA";
const std::string Quake3ShaderStage::BlendFunc::OneMinusSrcAlpha =
  "GL_ONE_MINUS_SRC_ALPHA";
const std::string Quake3ShaderStage::BlendFunc::OneMinusDestAlpha =
  "GL_ONE_MINUS_DST_ALPHA";
const std::string Quake3ShaderStage::BlendFunc::SrcAlphaSaturate =
  "GL_SRC_ALPHA_SATURATE";

bool Quake3ShaderStage::BlendFunc::enable() const
{
  return !srcFactor.empty() && !destFactor.empty();
}

bool Quake3ShaderStage::BlendFunc::validateSrcFactor() const
{
  if (srcFactor == One)
  {
    return true;
  }
  if (srcFactor == Zero)
  {
    return true;
  }
  if (srcFactor == DestColor)
  {
    return true;
  }
  if (srcFactor == OneMinusDestColor)
  {
    return true;
  }
  if (srcFactor == SrcAlpha)
  {
    return true;
  }
  if (srcFactor == DestAlpha)
  {
    return true;
  }
  if (srcFactor == OneMinusSrcAlpha)
  {
    return true;
  }
  if (srcFactor == OneMinusDestAlpha)
  {
    return true;
  }
  if (srcFactor == SrcAlphaSaturate)
  {
    return true;
  }
  return false;
}

bool Quake3ShaderStage::BlendFunc::validateDestFactor() const
{
  if (destFactor == One)
  {
    return true;
  }
  if (destFactor == Zero)
  {
    return true;
  }
  if (destFactor == SrcColor)
  {
    return true;
  }
  if (destFactor == OneMinusSrcColor)
  {
    return true;
  }
  if (destFactor == SrcAlpha)
  {
    return true;
  }
  if (destFactor == DestAlpha)
  {
    return true;
  }
  if (destFactor == OneMinusSrcAlpha)
  {
    return true;
  }
  if (destFactor == OneMinusDestAlpha)
  {
    return true;
  }
  return false;
}

void Quake3ShaderStage::BlendFunc::reset()
{
  srcFactor = destFactor = "";
}

kdl_reflect_impl(Quake3ShaderStage::BlendFunc);
kdl_reflect_impl(Quake3ShaderStage);

Quake3ShaderStage& Quake3Shader::addStage()
{
  stages.emplace_back();
  return stages.back();
}

kdl_reflect_impl(Quake3Shader);

std::ostream& operator<<(std::ostream& lhs, const Quake3Shader::Culling rhs)
{
  switch (rhs)
  {
  case Quake3Shader::Culling::Front:
    lhs << "Front";
    break;
  case Quake3Shader::Culling::Back:
    lhs << "Back";
    break;
  case Quake3Shader::Culling::None:
    lhs << "None";
    break;
  }
  return lhs;
}

} // namespace tb::mdl
