/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#pragma once

#include "vm/vec.h"

#include <vector>

namespace tb::gl
{
class ActiveShader;
}

namespace tb::render
{

enum class PlayerVision
{
  Human,
  Infravision,
  Ultravision,
};

struct PointLight
{
  vm::vec3f position;
  vm::vec3f color;
  float radius = 0.0f;
};

struct SceneLighting
{
  bool enabled = false;
  PlayerVision vision = PlayerVision::Human;
  float timeOfDay = 12.0f;
  std::vector<PointLight> pointLights;
};

struct SceneLightingProfile
{
  vm::vec3f ambientColor;
  vm::vec3f sunDirection;
  vm::vec3f sunColor;
  vm::vec3f visionTint;
  vm::vec3f skyColor;
};

constexpr size_t MaxScenePointLights = 16u;

SceneLightingProfile sceneLightingProfile(PlayerVision vision, float timeOfDay);
void setSceneLightingUniforms(gl::ActiveShader& shader, const SceneLighting& lighting);

} // namespace tb::render
