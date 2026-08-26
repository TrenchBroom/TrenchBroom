/*
 Copyright (C) 2026 Kristian Duske

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
#include "gl/MockGl.h"
#include "gl/TestUtils.h"
#include "gl/Texture.h"
#include "gl/TextureResource.h"

#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::gl
{
namespace
{

std::shared_ptr<TextureResource> makeReadyTextureResource(Gl& gl)
{
  auto texture = Texture{4, 4};
  texture.upload(gl);
  return createTextureResource(std::move(texture));
}

} // namespace

TEST_CASE("Material")
{
  SECTION("name")
  {
    auto material = Material{"some material", createTextureResource(Texture{4, 4})};
    CHECK(material.name() == "some material");
  }

  SECTION("texture and textureResource")
  {
    auto resource = createTextureResource(Texture{4, 4});
    const auto* textureResourcePtr = resource.get();
    auto material = Material{"some material", std::move(resource)};

    CHECK(&material.textureResource() == textureResourcePtr);
    CHECK(material.texture() == material.textureResource().get());
    CHECK(std::as_const(material).texture() == material.textureResource().get());
  }

  SECTION("usageCount, incUsageCount and decUsageCount")
  {
    auto material = Material{"some material", createTextureResource(Texture{4, 4})};
    CHECK(material.usageCount() == 0u);

    material.incUsageCount();
    material.incUsageCount();
    CHECK(material.usageCount() == 2u);

    material.decUsageCount();
    CHECK(material.usageCount() == 1u);
  }

  SECTION("move construction and move assignment")
  {
    auto resource = createTextureResource(Texture{4, 4});
    const auto* resourcePtr = resource.get();

    auto original = Material{"some material", std::move(resource)};
    original.setCollectionName("some collection");
    original.incUsageCount();

    auto moved = Material{std::move(original)};
    CHECK(moved.name() == "some material");
    CHECK(moved.collectionName() == "some collection");
    CHECK(moved.usageCount() == 1u);
    CHECK(&moved.textureResource() == resourcePtr);

    auto assigned = Material{"other material", createTextureResource(Texture{4, 4})};
    assigned = std::move(moved);
    CHECK(assigned.name() == "some material");
    CHECK(&assigned.textureResource() == resourcePtr);
  }

  SECTION("activate and deactivate")
  {
    auto gl = MockGl{};
    installTextureUploadSupport(gl);

    SECTION("do nothing if the texture is not GL-ready")
    {
      // this must not crash even though the texture never becomes ready
      auto material = Material{"some material", createTextureResource(Texture{4, 4})};

      auto boundTextures = std::vector<GLuint>{};
      gl.onBindTexture = [&](GLenum, const GLuint id) { boundTextures.push_back(id); };

      material.activate(gl, GL_LINEAR, GL_LINEAR);
      material.deactivate(gl);
      CHECK(boundTextures.empty());
    }

    SECTION("with a GL-ready texture")
    {
      auto material = Material{"some material", makeReadyTextureResource(gl)};

      auto boundTextures = std::vector<GLuint>{};
      gl.onBindTexture = [&](GLenum, const GLuint id) { boundTextures.push_back(id); };

      material.activate(gl, GL_LINEAR, GL_LINEAR);
      material.deactivate(gl);

      REQUIRE(boundTextures.size() == 2u);
      CHECK(boundTextures[0] != 0u);
      CHECK(boundTextures[1] == 0u);
    }

    SECTION("apply culling")
    {
      const auto culling = GENERATE(
        MaterialCulling::None,
        MaterialCulling::Front,
        MaterialCulling::Both,
        MaterialCulling::Back,
        MaterialCulling::Default);
      CAPTURE(culling);

      auto material = Material{"some material", makeReadyTextureResource(gl)};
      material.setCulling(culling);

      enum class Call
      {
        Enable,
        Disable,
        CullFace
      };
      using CullingCall = std::pair<Call, GLenum>;

      auto activateCalls = std::vector<CullingCall>{};
      gl.onEnable = [&](const GLenum cap) {
        activateCalls.emplace_back(Call::Enable, cap);
      };
      gl.onDisable = [&](const GLenum cap) {
        activateCalls.emplace_back(Call::Disable, cap);
      };
      gl.onCullFace = [&](const GLenum mode) {
        activateCalls.emplace_back(Call::CullFace, mode);
      };

      material.activate(gl, GL_LINEAR, GL_LINEAR);
      const auto onActivate = activateCalls;
      activateCalls.clear();

      material.deactivate(gl);
      const auto onDeactivate = activateCalls;

      switch (culling)
      {
      case MaterialCulling::None:
        CHECK(onActivate == std::vector<CullingCall>{{Call::Disable, GL_CULL_FACE}});
        CHECK(onDeactivate == std::vector<CullingCall>{{Call::Enable, GL_CULL_FACE}});
        break;
      case MaterialCulling::Front:
        CHECK(onActivate == std::vector<CullingCall>{{Call::CullFace, GL_FRONT}});
        CHECK(onDeactivate == std::vector<CullingCall>{{Call::CullFace, GL_BACK}});
        break;
      case MaterialCulling::Both:
        CHECK(
          onActivate == std::vector<CullingCall>{{Call::CullFace, GL_FRONT_AND_BACK}});
        CHECK(onDeactivate == std::vector<CullingCall>{{Call::CullFace, GL_BACK}});
        break;
      case MaterialCulling::Back:
      case MaterialCulling::Default:
        CHECK(onActivate.empty());
        CHECK(onDeactivate.empty());
        break;
      }
    }

    SECTION("apply the blend func set by setBlendFunc or disableBlend")
    {
      auto material = Material{"some material", makeReadyTextureResource(gl)};

      auto pushedAttribs = std::vector<GLbitfield>{};
      gl.onPushAttrib = [&](const GLbitfield mask) { pushedAttribs.push_back(mask); };

      auto poppedAttribCount = 0;
      gl.onPopAttrib = [&]() { ++poppedAttribCount; };

      auto depthMaskCalls = std::vector<GLboolean>{};
      gl.onDepthMask = [&](const GLboolean flag) { depthMaskCalls.push_back(flag); };

      SECTION("UseFactors")
      {
        material.setBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        auto capturedBlendFunc = std::pair<GLenum, GLenum>{0, 0};
        gl.onBlendFunc = [&](const GLenum src, const GLenum dst) {
          capturedBlendFunc = {src, dst};
        };

        material.activate(gl, GL_LINEAR, GL_LINEAR);
        CHECK(
          capturedBlendFunc
          == std::pair<GLenum, GLenum>{GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA});
        // A material with real per-pixel blending must not write depth.
        CHECK(depthMaskCalls == std::vector<GLboolean>{GL_FALSE});
        REQUIRE(
          pushedAttribs
          == std::vector<GLbitfield>{GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT});
      }

      SECTION("DisableBlend")
      {
        material.disableBlend();

        auto disabledCaps = std::vector<GLenum>{};
        gl.onDisable = [&](const GLenum cap) { disabledCaps.push_back(cap); };

        material.activate(gl, GL_LINEAR, GL_LINEAR);
        CHECK(disabledCaps == std::vector<GLenum>{GL_BLEND});
        CHECK(depthMaskCalls.empty());
        REQUIRE(pushedAttribs == std::vector<GLbitfield>{GL_COLOR_BUFFER_BIT});
      }

      material.deactivate(gl);
      CHECK(poppedAttribCount == 1);
    }

    SECTION(
      "apply the blend func implicitly derived from a graduated texture's alpha domain")
    {
      auto texture = Texture{4, 4};
      texture.setAlphaDomain(img::ImageAlphaDomain::Graduated);
      texture.upload(gl);
      auto material =
        Material{"some material", createTextureResource(std::move(texture))};

      auto pushedAttribs = std::vector<GLbitfield>{};
      gl.onPushAttrib = [&](const GLbitfield mask) { pushedAttribs.push_back(mask); };

      auto poppedAttribCount = 0;
      gl.onPopAttrib = [&]() { ++poppedAttribCount; };

      auto depthMaskCalls = std::vector<GLboolean>{};
      gl.onDepthMask = [&](const GLboolean flag) { depthMaskCalls.push_back(flag); };

      auto capturedBlendFunc = std::pair<GLenum, GLenum>{0, 0};
      gl.onBlendFunc = [&](const GLenum src, const GLenum dst) {
        capturedBlendFunc = {src, dst};
      };

      material.activate(gl, GL_LINEAR, GL_LINEAR);
      CHECK(
        capturedBlendFunc
        == std::pair<GLenum, GLenum>{GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA});
      // A material with real per-pixel blending must not write depth.
      CHECK(depthMaskCalls == std::vector<GLboolean>{GL_FALSE});
      REQUIRE(
        pushedAttribs
        == std::vector<GLbitfield>{GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT});

      material.deactivate(gl);
      CHECK(poppedAttribCount == 1);
    }
  }

  SECTION("effectiveAlphaFunc and effectiveBlendFunc")
  {
    auto makeMaterial = [](const img::ImageAlphaDomain alphaDomain) {
      auto texture = Texture{4, 4};
      texture.setAlphaDomain(alphaDomain);
      return Material{"some material", createTextureResource(std::move(texture))};
    };

    SECTION("no explicit override falls back to the texture's alpha domain")
    {
      // clang-format off
      const auto
      [alphaDomain,                      expectAlphaFunc, expectedBlendEnable] = GENERATE(table<img::ImageAlphaDomain, bool, MaterialBlendFunc::Enable>({
      {img::ImageAlphaDomain::Opaque,    false,            MaterialBlendFunc::Enable::UseDefault},
      {img::ImageAlphaDomain::Binary,    true,             MaterialBlendFunc::Enable::UseDefault},
      {img::ImageAlphaDomain::Graduated, false,            MaterialBlendFunc::Enable::UseFactors},
        }));
      // clang-format on

      CAPTURE(alphaDomain);

      const auto material = makeMaterial(alphaDomain);
      const auto alphaFunc = material.effectiveAlphaFunc();
      CHECK(alphaFunc.has_value() == expectAlphaFunc);
      if (expectAlphaFunc)
      {
        CHECK(alphaFunc->compare == MaterialAlphaFunc::Compare::GreaterEqual);
        CHECK(alphaFunc->threshold == 0.5f);
      }
      CHECK(material.effectiveBlendFunc().enable == expectedBlendEnable);
      if (expectedBlendEnable == MaterialBlendFunc::Enable::UseFactors)
      {
        CHECK(material.effectiveBlendFunc().srcFactor == GL_SRC_ALPHA);
        CHECK(material.effectiveBlendFunc().destFactor == GL_ONE_MINUS_SRC_ALPHA);
      }
    }

    SECTION("an explicit setAlphaFunc override wins over the texture's alpha domain")
    {
      auto material = makeMaterial(img::ImageAlphaDomain::Opaque);
      material.setAlphaFunc(MaterialAlphaFunc::Compare::Less, 0.25f);

      const auto alphaFunc = material.effectiveAlphaFunc();
      REQUIRE(alphaFunc.has_value());
      CHECK(alphaFunc->compare == MaterialAlphaFunc::Compare::Less);
      CHECK(alphaFunc->threshold == 0.25f);
    }

    SECTION(
      "an explicit setAlphaFunc override suppresses the implicit blend derived from a "
      "graduated texture")
    {
      auto material = makeMaterial(img::ImageAlphaDomain::Graduated);
      material.setAlphaFunc(MaterialAlphaFunc::Compare::GreaterEqual, 0.5f);

      CHECK(material.effectiveAlphaFunc().has_value());
      CHECK(
        material.effectiveBlendFunc().enable == MaterialBlendFunc::Enable::UseDefault);
    }

    SECTION(
      "an explicit setBlendFunc/disableBlend override wins over the texture's "
      "alpha domain")
    {
      auto material = makeMaterial(img::ImageAlphaDomain::Opaque);

      material.setBlendFunc(GL_ONE, GL_ONE);
      CHECK(
        material.effectiveBlendFunc().enable == MaterialBlendFunc::Enable::UseFactors);
      CHECK(material.effectiveBlendFunc().srcFactor == GL_ONE);
      CHECK(material.effectiveBlendFunc().destFactor == GL_ONE);

      material.disableBlend();
      CHECK(
        material.effectiveBlendFunc().enable == MaterialBlendFunc::Enable::DisableBlend);
    }
  }

  SECTION("getTexture")
  {
    auto material = Material{"some material", createTextureResource(Texture{4, 4})};

    CHECK(getTexture(static_cast<Material*>(nullptr)) == nullptr);
    CHECK(getTexture(static_cast<const Material*>(nullptr)) == nullptr);
    CHECK(getTexture(&material) == material.texture());
    CHECK(getTexture(const_cast<const Material*>(&material)) == material.texture());
  }
}

} // namespace tb::gl
