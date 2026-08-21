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

#include "gl/Material.h"

#include "base/Macros.h"
#include "gl/GlInterface.h"
#include "gl/Texture.h"

#include "kd/contracts.h"
#include "kd/reflection_impl.h"

#include <ostream>

namespace tb::gl
{

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

kdl_reflect_impl(MaterialAlphaFunc);

std::ostream& operator<<(std::ostream& lhs, const MaterialAlphaFunc::Compare& rhs)
{
  switch (rhs)
  {
  case MaterialAlphaFunc::Compare::GreaterEqual:
    lhs << "GreaterEqual";
    break;
  case MaterialAlphaFunc::Compare::Less:
    lhs << "Less";
    break;
  case MaterialAlphaFunc::Compare::Greater:
    lhs << "Greater";
    break;
    switchDefault();
  }
  return lhs;
}

kdl_reflect_impl(Material);

Material::Material(std::string name, std::shared_ptr<TextureResource> textureResource)
  : m_name{std::move(name)}
  , m_textureResource{std::move(textureResource)}
{
}

Material::~Material() = default;

Material::Material(Material&& other)
  : m_name{std::move(other.m_name)}
  , m_collectionName{std::move(other.m_collectionName)}
  , m_absolutePath{std::move(other.m_absolutePath)}
  , m_relativePath{std::move(other.m_relativePath)}
  , m_textureResource{std::move(other.m_textureResource)}
  , m_usageCount{static_cast<size_t>(other.m_usageCount)}
  , m_surfaceParms{std::move(other.m_surfaceParms)}
  , m_culling{std::move(other.m_culling)}
  , m_blendFunc{std::move(other.m_blendFunc)}
  , m_alphaFunc{std::move(other.m_alphaFunc)}
{
}

Material& Material::operator=(Material&& other)
{
  m_name = std::move(other.m_name);
  m_collectionName = std::move(other.m_collectionName);
  m_absolutePath = std::move(other.m_absolutePath);
  m_relativePath = std::move(other.m_relativePath);
  m_textureResource = std::move(other.m_textureResource);
  m_usageCount = static_cast<size_t>(other.m_usageCount);
  m_surfaceParms = std::move(other.m_surfaceParms);
  m_culling = std::move(other.m_culling);
  m_blendFunc = std::move(other.m_blendFunc);
  m_alphaFunc = std::move(other.m_alphaFunc);
  return *this;
}

const std::string& Material::name() const
{
  return m_name;
}

const std::string& Material::collectionName() const
{
  return m_collectionName;
}

void Material::setCollectionName(std::string collectionName)
{
  m_collectionName = std::move(collectionName);
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

const Texture* Material::texture() const
{
  return m_textureResource->get();
}

Texture* Material::texture()
{
  return m_textureResource->get();
}

const TextureResource& Material::textureResource() const
{
  return *m_textureResource;
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

void Material::setAlphaFunc(
  const MaterialAlphaFunc::Compare compare, const float threshold)
{
  m_alphaFunc = MaterialAlphaFunc{compare, threshold};
}

std::optional<MaterialAlphaFunc> Material::effectiveAlphaFunc() const
{
  if (m_alphaFunc)
  {
    return m_alphaFunc;
  }

  if (const auto* texture = this->texture();
      texture && texture->alphaDomain() == img::ImageAlphaDomain::Binary)
  {
    return MaterialAlphaFunc{MaterialAlphaFunc::Compare::GreaterEqual, 0.5f};
  }

  return std::nullopt;
}

MaterialBlendFunc Material::effectiveBlendFunc() const
{
  if (m_blendFunc.enable != MaterialBlendFunc::Enable::UseDefault || m_alphaFunc)
  {
    return m_blendFunc;
  }


  if (const auto* texture = this->texture();
      texture && texture->alphaDomain() == img::ImageAlphaDomain::Graduated)
  {
    return MaterialBlendFunc{
      MaterialBlendFunc::Enable::UseFactors, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA};
  }

  return m_blendFunc;
}

size_t Material::usageCount() const
{
  return static_cast<size_t>(m_usageCount);
}

void Material::incUsageCount() const
{
  ++m_usageCount;
}

void Material::decUsageCount() const
{
  const size_t previous = m_usageCount--;
  contract_assert(previous > 0);
}

void Material::activate(Gl& gl, const int minFilter, const int magFilter) const
{
  if (const auto* texture = m_textureResource->get();
      texture && texture->activate(gl, minFilter, magFilter))
  {
    switch (m_culling)
    {
    case MaterialCulling::None:
      gl.disable(GL_CULL_FACE);
      break;
    case MaterialCulling::Front:
      gl.cullFace(GL_FRONT);
      break;
    case MaterialCulling::Both:
      gl.cullFace(GL_FRONT_AND_BACK);
      break;
    case MaterialCulling::Default:
    case MaterialCulling::Back:
      break;
    }

    if (const auto blendFunc = effectiveBlendFunc();
        blendFunc.enable != MaterialBlendFunc::Enable::UseDefault)
    {
      if (blendFunc.enable == MaterialBlendFunc::Enable::UseFactors)
      {
        // A material with real per-pixel blending must not write depth: otherwise its
        // fully or partially transparent fragments would still pass the depth test and
        // occlude geometry drawn behind them.
        gl.pushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        gl.blendFunc(blendFunc.srcFactor, blendFunc.destFactor);
        gl.depthMask(GL_FALSE);
      }
      else
      {
        contract_assert(blendFunc.enable == MaterialBlendFunc::Enable::DisableBlend);
        gl.pushAttrib(GL_COLOR_BUFFER_BIT);
        gl.disable(GL_BLEND);
      }
    }
  }
}

void Material::deactivate(Gl& gl) const
{
  if (const auto* texture = m_textureResource->get(); texture && texture->deactivate(gl))
  {
    if (effectiveBlendFunc().enable != MaterialBlendFunc::Enable::UseDefault)
    {
      gl.popAttrib();
    }

    switch (m_culling)
    {
    case MaterialCulling::None:
      gl.enable(GL_CULL_FACE);
      break;
    case MaterialCulling::Front:
      gl.cullFace(GL_BACK);
      break;
    case MaterialCulling::Both:
      gl.cullFace(GL_BACK);
      break;
    case MaterialCulling::Default:
    case MaterialCulling::Back:
      break;
    }
  }
}

const Texture* getTexture(const Material* material)
{
  return material ? material->texture() : nullptr;
}

Texture* getTexture(Material* material)
{
  return material ? material->texture() : nullptr;
}

} // namespace tb::gl
