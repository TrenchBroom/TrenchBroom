/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "render/SceneLighting.h"

#include "gl/ActiveShader.h"

#include "vm/scalar.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <string>

namespace tb::render
{
namespace
{

vm::vec3f mix(const vm::vec3f& a, const vm::vec3f& b, const float amount)
{
  return a + amount * (b - a);
}

float normalizedHour(const float timeOfDay)
{
  auto hour = std::fmod(timeOfDay, 24.0f);
  if (hour < 0.0f)
  {
    hour += 24.0f;
  }
  return hour;
}

} // namespace

SceneLightingProfile sceneLightingProfile(
  const PlayerVision vision, const float timeOfDay)
{
  const auto hour = normalizedHour(timeOfDay);
  const auto solarAngle = (hour - 6.0f) / 12.0f * std::numbers::pi_v<float>;
  const auto sunHeight = std::max(std::sin(solarAngle), 0.0f);
  const auto daylight = vm::smoothstep(0.0f, 0.35f, sunHeight);

  const auto nightAmbient = [&]() {
    switch (vision)
    {
    case PlayerVision::Human:
      return vm::vec3f{0.025f, 0.035f, 0.065f};
    case PlayerVision::Infravision:
      return vm::vec3f{0.24f, 0.105f, 0.055f};
    case PlayerVision::Ultravision:
      return vm::vec3f{0.15f, 0.20f, 0.36f};
    }
    return vm::vec3f{};
  }();
  const auto nightTint = [&]() {
    switch (vision)
    {
    case PlayerVision::Human:
      return vm::vec3f{0.78f, 0.86f, 1.0f};
    case PlayerVision::Infravision:
      return vm::vec3f{1.16f, 0.78f, 0.62f};
    case PlayerVision::Ultravision:
      return vm::vec3f{0.74f, 0.9f, 1.2f};
    }
    return vm::vec3f{1.0f, 1.0f, 1.0f};
  }();

  const auto azimuth = (hour / 24.0f) * 2.0f * std::numbers::pi_v<float>;
  const auto sunDirection = vm::normalize(vm::vec3f{
    -std::cos(azimuth) * std::sqrt(std::max(1.0f - sunHeight * sunHeight, 0.0f)),
    -std::sin(azimuth) * std::sqrt(std::max(1.0f - sunHeight * sunHeight, 0.0f)),
    sunHeight});

  return {
    mix(nightAmbient, vm::vec3f{0.46f, 0.48f, 0.52f}, daylight),
    sunDirection,
    mix(vm::vec3f{}, vm::vec3f{0.62f, 0.57f, 0.48f}, daylight),
    mix(nightTint, vm::vec3f{1.0f, 1.0f, 1.0f}, daylight),
    mix(vm::vec3f{0.025f, 0.012f, 0.055f}, vm::vec3f{0.28f, 0.43f, 0.65f}, daylight),
  };
}

void setSceneLightingUniforms(gl::ActiveShader& shader, const SceneLighting& lighting)
{
  shader.set("ApplySceneLighting", lighting.enabled);
  const auto profile = sceneLightingProfile(lighting.vision, lighting.timeOfDay);
  shader.set("SceneAmbientColor", profile.ambientColor);
  shader.set("SceneSunDirection", profile.sunDirection);
  shader.set("SceneSunColor", profile.sunColor);
  shader.set("SceneVisionTint", profile.visionTint);

  const auto count = std::min(lighting.pointLights.size(), MaxScenePointLights);
  shader.set("ScenePointLightCount", count);
  for (size_t i = 0u; i < count; ++i)
  {
    const auto prefix = "ScenePointLights[" + std::to_string(i) + "]";
    shader.set(prefix + ".position", lighting.pointLights[i].position);
    shader.set(prefix + ".color", lighting.pointLights[i].color);
    shader.set(prefix + ".radius", lighting.pointLights[i].radius);
  }
}

} // namespace tb::render
