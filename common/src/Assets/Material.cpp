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

#include "Material.h"

#include "Assets/MaterialCollection.h"
#include "Assets/TextureBuffer.h"
#include "Macros.h"
#include "Renderer/GL.h"

#include "kdl/overload.h"
#include "kdl/reflection_impl.h"

#include "vm/vec_io.h"

#include <algorithm> // for std::max
#include <cassert>
#include <ostream>

namespace TrenchBroom::Assets
{

std::ostream& operator<<(std::ostream& lhs, const TextureType& rhs)
{
  switch (rhs)
  {
  case TextureType::Opaque:
    lhs << "Opaque";
    break;
  case TextureType::Masked:
    lhs << "Masked";
    break;
    switchDefault();
  }
  return lhs;
}

std::ostream& operator<<(std::ostream& lhs, const MaterialCulling& rhs)
{
  switch (rhs)
  {
  case MaterialCulling::Default:
    lhs << "Default";
    break;
  case MaterialCulling::None:
    lhs << "None";
    break;
  case MaterialCulling::Front:
    lhs << "Front";
    break;
  case MaterialCulling::Back:
    lhs << "Back";
    break;
  case MaterialCulling::Both:
    lhs << "Both";
    break;
    switchDefault();
  }
  return lhs;
}

kdl_reflect_impl(MaterialBlendFunc);

std::ostream& operator<<(std::ostream& lhs, const MaterialBlendFunc::Enable& rhs)
{
  switch (rhs)
  {
  case MaterialBlendFunc::Enable::UseDefault:
    lhs << "UseDefault";
    break;
  case MaterialBlendFunc::Enable::UseFactors:
    lhs << "UseFactors";
    break;
  case MaterialBlendFunc::Enable::DisableBlend:
    lhs << "DisableBlend";
    break;
    switchDefault();
  }
  return lhs;
}

std::ostream& operator<<(std::ostream& lhs, const GameData& rhs)
{
  std::visit(
    kdl::overload(
      [&](const std::monostate&) { lhs << "std::monostate"; },
      [&](const auto& x) { lhs << x; }),
    rhs);
  return lhs;
}

kdl_reflect_impl(Q2Data);

kdl_reflect_impl(Material);

namespace
{

auto textureMask(const TextureType type)
{
  return type == TextureType::Masked ? TextureMask::On : TextureMask::Off;
}

auto embeddedDefaults(const GameData& gameData)
{
  return std::visit(
    kdl::overload(
      [](const std::monostate&) { return EmbeddedDefaults{NoEmbeddedDefaults{}}; },
      [](const Q2Data& x) {
        return EmbeddedDefaults{Q2EmbeddedDefaults{x.flags, x.contents, x.value}};
      }),
    gameData);
}

} // namespace

Material::Material(
  std::string name,
  const size_t width,
  const size_t height,
  const Color& averageColor,
  Buffer&& buffer,
  const GLenum format,
  const TextureType type,
  GameData gameData)
  : Material{
    std::move(name),
    Texture{
      width,
      height,
      averageColor,
      format,
      textureMask(type),
      embeddedDefaults(gameData),
      {std::move(buffer)}}}
{
}

Material::Material(
  std::string name,
  const size_t width,
  const size_t height,
  const Color& averageColor,
  BufferList buffers,
  const GLenum format,
  const TextureType type,
  GameData gameData)
  : Material{
    std::move(name),
    Texture{
      width,
      height,
      averageColor,
      format,
      textureMask(type),
      embeddedDefaults(gameData),
      std::move(buffers)}}
{
}

Material::Material(
  std::string name,
  const size_t width,
  const size_t height,
  const GLenum format,
  const TextureType type,
  GameData gameData)
  : Material{
    std::move(name),
    Texture{
      width,
      height,
      Color(0.0f, 0.0f, 0.0f, 1.0f),
      format,
      textureMask(type),
      embeddedDefaults(gameData),
      std::vector<TextureBuffer>{}}}
{
}

Material::Material(std::string name, Texture texture)
  : m_name{std::move(name)}
  , m_texture{std::move(texture)}
{
}

Material::~Material() = default;

Material::Material(Material&& other)
  : m_name{std::move(other.m_name)}
  , m_absolutePath{std::move(other.m_absolutePath)}
  , m_relativePath{std::move(other.m_relativePath)}
  , m_texture{std::move(other.m_texture)}
  , m_usageCount{static_cast<size_t>(other.m_usageCount)}
  , m_surfaceParms{std::move(other.m_surfaceParms)}
  , m_culling{std::move(other.m_culling)}
  , m_blendFunc{std::move(other.m_blendFunc)}
{
}

Material& Material::operator=(Material&& other)
{
  m_name = std::move(other.m_name);
  m_absolutePath = std::move(other.m_absolutePath);
  m_relativePath = std::move(other.m_relativePath);
  m_texture = std::move(other.m_texture);
  m_usageCount = static_cast<size_t>(other.m_usageCount);
  m_surfaceParms = std::move(other.m_surfaceParms);
  m_culling = std::move(other.m_culling);
  m_blendFunc = std::move(other.m_blendFunc);
  return *this;
}

TextureType Material::selectTextureType(const bool masked)
{
  return masked ? TextureType::Masked : TextureType::Opaque;
}

const std::string& Material::name() const
{
  return m_name;
}

const std::filesystem::path& Material::absolutePath() const
{
  return m_absolutePath;
}

void Material::setAbsolutePath(std::filesystem::path absolutePath)
{
  m_absolutePath = std::move(absolutePath);
}

const std::filesystem::path& Material::relativePath() const
{
  return m_relativePath;
}

void Material::setRelativePath(std::filesystem::path relativePath)
{
  m_relativePath = std::move(relativePath);
}

size_t Material::width() const
{
  return m_texture.width();
}

size_t Material::height() const
{
  return m_texture.height();
}

const Color& Material::averageColor() const
{
  return m_texture.averageColor();
}

bool Material::masked() const
{
  return m_texture.mask() == TextureMask::On;
}

void Material::setOpaque()
{
  m_texture.setMask(TextureMask::Off);
}

const std::set<std::string>& Material::surfaceParms() const
{
  return m_surfaceParms;
}

void Material::setSurfaceParms(std::set<std::string> surfaceParms)
{
  m_surfaceParms = std::move(surfaceParms);
}

MaterialCulling Material::culling() const
{
  return m_culling;
}

void Material::setCulling(const MaterialCulling culling)
{
  m_culling = culling;
}

void Material::setBlendFunc(const GLenum srcFactor, const GLenum destFactor)
{
  m_blendFunc.enable = MaterialBlendFunc::Enable::UseFactors;
  m_blendFunc.srcFactor = srcFactor;
  m_blendFunc.destFactor = destFactor;
}

void Material::disableBlend()
{
  m_blendFunc.enable = MaterialBlendFunc::Enable::DisableBlend;
}

GameData Material::gameData() const
{
  return std::visit(
    kdl::overload(
      [](const NoEmbeddedDefaults&) -> GameData { return std::monostate{}; },
      [](const Q2EmbeddedDefaults& x) -> GameData {
        return Q2Data{x.flags, x.contents, x.value};
      }),
    m_texture.embeddedDefaults());
}

size_t Material::usageCount() const
{
  return static_cast<size_t>(m_usageCount);
}

void Material::incUsageCount()
{
  ++m_usageCount;
}

void Material::decUsageCount()
{
  const size_t previous = m_usageCount--;
  assert(previous > 0);
  unused(previous);
}

bool Material::isPrepared() const
{
  return m_texture.isReady();
}

void Material::prepare(const GLuint, const int minFilter, const int magFilter)
{
  m_texture.upload();
  m_texture.setFilterMode(minFilter, magFilter);
}

void Material::setFilterMode(const int minFilter, const int magFilter)
{
  m_texture.setFilterMode(minFilter, magFilter);
}

void Material::activate() const
{
  if (m_texture.activate())
  {
    switch (m_culling)
    {
    case Assets::MaterialCulling::None:
      glAssert(glDisable(GL_CULL_FACE));
      break;
    case Assets::MaterialCulling::Front:
      glAssert(glCullFace(GL_FRONT));
      break;
    case Assets::MaterialCulling::Both:
      glAssert(glCullFace(GL_FRONT_AND_BACK));
      break;
    case Assets::MaterialCulling::Default:
    case Assets::MaterialCulling::Back:
      break;
    }

    if (m_blendFunc.enable != MaterialBlendFunc::Enable::UseDefault)
    {
      glAssert(glPushAttrib(GL_COLOR_BUFFER_BIT));
      if (m_blendFunc.enable == MaterialBlendFunc::Enable::UseFactors)
      {
        glAssert(glBlendFunc(m_blendFunc.srcFactor, m_blendFunc.destFactor));
      }
      else
      {
        assert(m_blendFunc.enable == MaterialBlendFunc::Enable::DisableBlend);
        glAssert(glDisable(GL_BLEND));
      }
    }
  }
}

void Material::deactivate() const
{
  if (m_texture.deactivate())
  {
    if (m_blendFunc.enable != MaterialBlendFunc::Enable::UseDefault)
    {
      glAssert(glPopAttrib());
    }

    switch (m_culling)
    {
    case Assets::MaterialCulling::None:
      glAssert(glEnable(GL_CULL_FACE));
      break;
    case Assets::MaterialCulling::Front:
      glAssert(glCullFace(GL_BACK));
      break;
    case Assets::MaterialCulling::Both:
      glAssert(glCullFace(GL_BACK));
      break;
    case Assets::MaterialCulling::Default:
    case Assets::MaterialCulling::Back:
      break;
    }

    glAssert(glBindTexture(GL_TEXTURE_2D, 0));
  }
}

const Material::BufferList& Material::buffersIfUnprepared() const
{
  return m_texture.buffersIfLoaded();
}

GLenum Material::format() const
{
  return m_texture.format();
}

TextureType Material::type() const
{
  return m_texture.mask() == TextureMask::On ? TextureType::Masked : TextureType::Opaque;
}

} // namespace TrenchBroom::Assets
