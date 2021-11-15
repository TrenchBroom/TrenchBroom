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

#pragma once

#include "EL/EL_Forward.h"
#include "Macros.h"

#include <memory>
#include <string>

namespace TrenchBroom {
namespace EL {
class EvaluationContext {
private:
  std::unique_ptr<VariableStore> m_store;

public:
  EvaluationContext();
  explicit EvaluationContext(const VariableStore& store);
  virtual ~EvaluationContext();

  virtual Value variableValue(const std::string& name) const;
  virtual void declareVariable(const std::string& name, const Value& value);

  deleteCopyAndMove(EvaluationContext);
};

class EvaluationStack : public EvaluationContext {
private:
  const EvaluationContext& m_next;

public:
  explicit EvaluationStack(const EvaluationContext& next);

  Value variableValue(const std::string& name) const override;

  deleteCopyAndMove(EvaluationStack);
};
} // namespace EL
} // namespace TrenchBroom
