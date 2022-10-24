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

#include "GameEngineProfile.h"

#include <kdl/reflection_impl.h>

#include <memory>
#include <ostream>
#include <string>

namespace TrenchBroom
{
namespace Model
{
GameEngineProfile::GameEngineProfile(
  const std::string& name, const IO::Path& path, const std::string& parameterSpec)
  : m_name(name)
  , m_path(path)
  , m_parameterSpec(parameterSpec)
{
}

std::unique_ptr<GameEngineProfile> GameEngineProfile::clone() const
{
  return std::make_unique<GameEngineProfile>(m_name, m_path, m_parameterSpec);
}

const std::string& GameEngineProfile::name() const
{
  return m_name;
}

const IO::Path& GameEngineProfile::path() const
{
  return m_path;
}

const std::string& GameEngineProfile::parameterSpec() const
{
  return m_parameterSpec;
}

void GameEngineProfile::setName(const std::string& name)
{
  m_name = name;
}

void GameEngineProfile::setPath(const IO::Path& path)
{
  m_path = path;
}

void GameEngineProfile::setParameterSpec(const std::string& parameterSpec)
{
  m_parameterSpec = parameterSpec;
}

kdl_reflect_impl(GameEngineProfile);

} // namespace Model
} // namespace TrenchBroom
