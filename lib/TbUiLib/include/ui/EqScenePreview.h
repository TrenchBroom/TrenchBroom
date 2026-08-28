/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#pragma once

#include "render/SceneLighting.h"

#include "vm/vec.h"

#include <string>

namespace tb::mdl
{
class Map;
}

namespace tb::ui
{

struct EqScenePreviewSettings
{
  bool enabled = false;
  render::PlayerVision vision = render::PlayerVision::Human;
  float timeOfDay = 12.0f;
  bool entityLights = true;
};

render::SceneLighting buildEqSceneLighting(
  mdl::Map& map,
  const EqScenePreviewSettings& settings,
  const vm::vec3f& referencePosition);

render::PlayerVision playerVisionFromName(const std::string& name);
const char* playerVisionName(render::PlayerVision vision);

} // namespace tb::ui
