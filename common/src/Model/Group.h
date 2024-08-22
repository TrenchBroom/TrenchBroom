/*
 Copyright (C) 2020 Kristian Duske

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

#include "FloatType.h"

#include "kdl/reflection_decl.h"

#include "vm/mat.h"

#include <string>

namespace TrenchBroom::Model
{

class Group
{
private:
  std::string m_name;

  vm::mat4x4 m_transformation;

  kdl_reflect_decl(Group, m_name, m_transformation);

public:
  explicit Group(std::string name);

  const std::string& name() const;
  void setName(std::string name);

  const vm::mat4x4& transformation() const;
  void setTransformation(const vm::mat4x4& transformation);
  void transform(const vm::mat4x4& transformation);
};

} // namespace TrenchBroom::Model
