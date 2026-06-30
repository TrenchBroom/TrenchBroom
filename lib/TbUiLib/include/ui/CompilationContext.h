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

#include "Result.h"
#include "el/VariableStore.h"
#include "ui/TextOutputAdapter.h"

#include "vm/vec.h"

#include <memory>
#include <optional>
#include <string>

namespace tb
{
namespace mdl
{
class Map;
}

namespace ui
{

struct CompilationCameraSnapshot
{
  vm::vec3d position;
  vm::vec3d direction;
};

class CompilationContext
{
private:
  const mdl::Map& m_map;
  std::unique_ptr<el::VariableStore> m_variables;

  TextOutputAdapter m_output;
  bool m_test;
  std::optional<CompilationCameraSnapshot> m_cameraSnapshot;

public:
  CompilationContext(
    const mdl::Map& map,
    const el::VariableStore& variables,
    TextOutputAdapter output,
    bool test,
    std::optional<CompilationCameraSnapshot> cameraSnapshot = std::nullopt);

  const mdl::Map& map() const;
  bool test() const;
  const std::optional<CompilationCameraSnapshot>& cameraSnapshot() const;

  Result<std::string> interpolate(const std::string& input) const;
  Result<std::string> variableValue(const std::string& variableName) const;

  template <typename T>
  CompilationContext& operator<<(const T& t)
  {
    m_output << t;
    return *this;
  }
};

} // namespace ui
} // namespace tb
