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

#include "gl/GlUtils.h"
#include "gl/TextureResource.h"

#include "kd/reflection_decl.h"

#include <atomic>
#include <filesystem>
#include <memory>
#include <optional>
#include <set>
#include <string>

namespace tb::gl
{
class Gl;

enum class MaterialCulling
{
  Default,
  None,
  Front,
  Back,
  Both
};

std::ostream& operator<<(std::ostream& lhs, const MaterialCulling& rhs);

struct MaterialBlendFunc
{
  enum class Enable
  {
    /**
     * Don't change GL_BLEND and don't change the blend function.
     */
    UseDefault,
    /**
     * Don't change GL_BLEND, but set the blend function.
     */
    UseFactors,
    /**
     * Set GL_BLEND to off.
     */
    DisableBlend
  };

  Enable enable;
  GLenum srcFactor;
  GLenum destFactor;

  kdl_reflect_decl(MaterialBlendFunc, enable, srcFactor, destFactor);
};

std::ostream& operator<<(std::ostream& lhs, const MaterialBlendFunc::Enable& rhs);

/**
 * A hard alpha-test cutout: keep the fragment when its alpha compares to `threshold`
 * according to `compare`, discard it otherwise. Mirrors Quake 3's alphaFunc directive
 * (GE128/LT128/GT0), which is independent of (and combinable with) blendFunc.
 */
struct MaterialAlphaFunc
{
  enum class Compare
  {
    GreaterEqual,
    Less,
    Greater,
  };

  Compare compare;
  float threshold;

  kdl_reflect_decl(MaterialAlphaFunc, compare, threshold);
};

std::ostream& operator<<(std::ostream& lhs, const MaterialAlphaFunc::Compare& rhs);

class Material
{
private:
  std::string m_name;
  std::string m_collectionName;
  std::filesystem::path m_absolutePath;
  std::filesystem::path m_relativePath;

  std::shared_ptr<TextureResource> m_textureResource;

  mutable std::atomic<size_t> m_usageCount = 0;

  // Quake 3 surface parameters; move these to materials when we add proper support for
  // those.
  std::set<std::string> m_surfaceParms;

  // Quake 3 surface culling; move to materials
  MaterialCulling m_culling = MaterialCulling::Default;

  // Quake 3 blend function, move to materials
  MaterialBlendFunc m_blendFunc = {
    MaterialBlendFunc::Enable::UseDefault, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA};

  // Explicit alpha-test (hard cutout) override. When unset, effectiveAlphaFunc() and
  // effectiveBlendFunc() fall back to the texture's ImageAlphaDomain, since some
  // formats can only classify their alpha channel once the pixel data is decoded, which
  // may happen after this material is constructed.
  std::optional<MaterialAlphaFunc> m_alphaFunc;

  kdl_reflect_decl(
    Material,
    m_name,
    m_collectionName,
    m_absolutePath,
    m_relativePath,
    m_textureResource,
    m_usageCount,
    m_surfaceParms,
    m_culling,
    m_blendFunc,
    m_alphaFunc);

public:
  Material(std::string name, std::shared_ptr<TextureResource> textureResource);

  Material(const Material&) = delete;
  Material& operator=(const Material&) = delete;

  Material(Material&& other);
  Material& operator=(Material&& other);

  ~Material();

  const std::string& name() const;

  const std::string& collectionName() const;
  void setCollectionName(std::string collectionName);

  /**
   * Absolute path of the material
   */
  const std::filesystem::path& absolutePath() const;
  void setAbsolutePath(std::filesystem::path absolutePath);

  /**
   * Relative path of the material in the game filesystem
   */
  const std::filesystem::path& relativePath() const;
  void setRelativePath(std::filesystem::path relativePath);

  const Texture* texture() const;
  Texture* texture();

  const TextureResource& textureResource() const;

  const std::set<std::string>& surfaceParms() const;
  void setSurfaceParms(std::set<std::string> surfaceParms);

  MaterialCulling culling() const;
  void setCulling(MaterialCulling culling);

  void setBlendFunc(GLenum srcFactor, GLenum destFactor);
  void disableBlend();

  void setAlphaFunc(MaterialAlphaFunc::Compare compare, float threshold);

  std::optional<MaterialAlphaFunc> effectiveAlphaFunc() const;
  MaterialBlendFunc effectiveBlendFunc() const;

  size_t usageCount() const;
  void incUsageCount() const;
  void decUsageCount() const;

  void activate(Gl& gl, int minFilter, int magFilter) const;
  void deactivate(Gl& gl) const;
};

const Texture* getTexture(const Material* material);
Texture* getTexture(Material* material);

} // namespace tb::gl
