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

#include "mdl/GameEngineConfig.h"

#include "kd/ranges/to.h"
#include "kd/reflection_impl.h"

namespace tb::mdl
{
namespace
{

el::Value toValue(const GameEngineProfile& profile)
{
  return el::Value{el::MapType{
    {"name", el::Value{profile.name}},
    {"path", el::Value{profile.path.string()}},
    {"parameters", el::Value{profile.parameterSpec}},
  }};
}

el::Value toValue(const std::vector<GameEngineProfile>& profiles)
{
  return el::Value{
    profiles
    | std::views::transform([&](const auto& profile) { return toValue(profile); })
    | kdl::ranges::to<std::vector>()};
}

} // namespace

kdl_reflect_impl(GameEngineConfig);

el::Value toValue(const GameEngineConfig& config)
{
  return el::Value{el::MapType{
    {"version", el::Value{1.0}},
    {"profiles", toValue(config.profiles)},
  }};
}

} // namespace tb::mdl
