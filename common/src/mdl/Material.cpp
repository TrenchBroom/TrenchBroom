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

#include "Material.h"

#include "Macros.h"
#include "mdl/Texture.h"

#include "kdl/reflection_impl.h"

#include <cassert>
#include <ostream>

namespace tb::mdl
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

void Material::activate(const int minFilter, const int magFilter) const
{
  if (const auto* texture = m_textureResource->get();
      texture && texture->activate(minFilter, magFilter))
  {
    switch (m_culling)
    {
    case MaterialCulling::None:
      glAssert(glDisable(GL_CULL_FACE));
      break;
    case MaterialCulling::Front:
      glAssert(glCullFace(GL_FRONT));
      break;
    case MaterialCulling::Both:
      glAssert(glCullFace(GL_FRONT_AND_BACK));
      break;
    case MaterialCulling::Default:
    case MaterialCulling::Back:
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
  if (const auto* texture = m_textureResource->get(); texture && texture->deactivate())
  {
    if (m_blendFunc.enable != MaterialBlendFunc::Enable::UseDefault)
    {
      glAssert(glPopAttrib());
    }

    switch (m_culling)
    {
    case MaterialCulling::None:
      glAssert(glEnable(GL_CULL_FACE));
      break;
    case MaterialCulling::Front:
      glAssert(glCullFace(GL_BACK));
      break;
    case MaterialCulling::Both:
      glAssert(glCullFace(GL_BACK));
      break;
    case MaterialCulling::Default:
    case MaterialCulling::Back:
      break;
    }

    glAssert(glBindTexture(GL_TEXTURE_2D, 0));
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

} // namespace tb::mdl
