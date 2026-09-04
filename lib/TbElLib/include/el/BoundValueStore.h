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

#pragma once

#include "el/Types.h"
#include "el/VariableStore.h"

#include <string>
#include <vector>

namespace tb::el
{

/**
 * A VariableStore backed by a single BoundValue -- forwards value()/names() straight
 * into the BoundValue's own at()/keys() closures, with a missing key resolving to
 * Value::Undefined, matching every other VariableStore's convention. This is what both
 * `mdl::EntityPropertiesVariableStore` and the node/face schema stores in `mdl` are
 * built from, so no hand-written VariableStore subclass is needed per bound object kind.
 */
class BoundValueStore : public VariableStore
{
private:
  BoundValue m_boundValue;

public:
  explicit BoundValueStore(BoundValue boundValue);

  VariableStore* clone() const override;
  size_t size() const override;
  Value value(const std::string& name) const override;
  std::vector<std::string> names() const override;
  void set(std::string name, Value value) override;
};

} // namespace tb::el
