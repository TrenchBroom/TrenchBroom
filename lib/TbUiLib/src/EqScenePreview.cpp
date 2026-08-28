/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "ui/EqScenePreview.h"

#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Map.h"

#include "kd/string_compare.h"
#include "kd/string_utils.h"

#include "vm/vec_io.h"

#include <algorithm>
#include <cmath>
#include <string_view>
#include <utility>
#include <vector>

namespace tb::ui
{
namespace
{

float lightRadius(const mdl::Entity& entity, const float fallback)
{
  if (const auto* value = entity.property("radius"))
  {
    return std::max(kdl::str_to_float(*value).value_or(fallback), 1.0f);
  }
  return fallback;
}

vm::vec3f lightColor(const mdl::Entity& entity, const vm::vec3f& fallback)
{
  if (const auto* value = entity.property("_color"))
  {
    if (const auto parsed = vm::parse<float, 3>(*value))
    {
      return vm::vec3f{
        std::clamp(parsed->x() / 255.0f, 0.0f, 1.0f),
        std::clamp(parsed->y() / 255.0f, 0.0f, 1.0f),
        std::clamp(parsed->z() / 255.0f, 0.0f, 1.0f)};
    }
  }
  return fallback;
}

render::PointLight pointLight(const mdl::Entity& entity)
{
  const auto isFixture = entity.classname() == "eq_prop_light";
  auto fallbackRadius = 640.0f;
  if (const auto* model = entity.property("model"))
  {
    if (model->find("candelabra") != std::string::npos)
    {
      fallbackRadius = 480.0f;
    }
    else if (model->find("brazier") != std::string::npos)
    {
      fallbackRadius = 900.0f;
    }
  }

  return {
    vm::vec3f{entity.origin()},
    lightColor(entity, isFixture ? vm::vec3f{1.0f, 0.48f, 0.16f} : vm::vec3f{1.0f}),
    lightRadius(entity, isFixture ? fallbackRadius : 3200.0f),
  };
}

} // namespace

render::PlayerVision playerVisionFromName(const std::string& name)
{
  if (kdl::ci::str_is_equal(name, "infravision"))
  {
    return render::PlayerVision::Infravision;
  }
  if (kdl::ci::str_is_equal(name, "ultravision"))
  {
    return render::PlayerVision::Ultravision;
  }
  return render::PlayerVision::Human;
}

const char* playerVisionName(const render::PlayerVision vision)
{
  switch (vision)
  {
  case render::PlayerVision::Human:
    return "human";
  case render::PlayerVision::Infravision:
    return "infravision";
  case render::PlayerVision::Ultravision:
    return "ultravision";
  }
  return "human";
}

render::SceneLighting buildEqSceneLighting(
  mdl::Map& map,
  const EqScenePreviewSettings& settings,
  const vm::vec3f& referencePosition)
{
  auto result =
    render::SceneLighting{settings.enabled, settings.vision, settings.timeOfDay, {}};
  if (!settings.enabled || !settings.entityLights)
  {
    return result;
  }

  auto lightNodes = map.findNodes<mdl::EntityNode>("eq_light");
  const auto fixtureNodes = map.findNodes<mdl::EntityNode>("eq_prop_light");
  lightNodes.insert(lightNodes.end(), fixtureNodes.begin(), fixtureNodes.end());

  auto scoredLights = std::vector<std::pair<float, render::PointLight>>{};
  scoredLights.reserve(lightNodes.size());
  for (const auto* node : lightNodes)
  {
    const auto& entity = node->entity();
    if (entity.classname() != "eq_light" && entity.classname() != "eq_prop_light")
    {
      continue;
    }
    auto light = pointLight(entity);
    const auto distance = vm::distance(light.position, referencePosition);
    scoredLights.emplace_back(std::max(distance - light.radius, 0.0f), light);
  }
  std::stable_sort(
    scoredLights.begin(), scoredLights.end(), [](const auto& lhs, const auto& rhs) {
      return lhs.first < rhs.first;
    });

  const auto count = std::min(scoredLights.size(), render::MaxScenePointLights);
  result.pointLights.reserve(count);
  for (size_t i = 0u; i < count; ++i)
  {
    result.pointLights.push_back(scoredLights[i].second);
  }
  return result;
}

} // namespace tb::ui
