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

#include "Exceptions.h"
#include "Expression.h"
#include "FileLocation.h"
#include "Macros.h"
#include "Result.h"
#include "Value.h"

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace tb::el
{

class EvaluationContext
{
private:
  std::unique_ptr<VariableStore> m_variables;
  std::unordered_map<Value, ExpressionNode> m_trace;

  EvaluationContext();
  explicit EvaluationContext(const VariableStore& variables);

public:
  ~EvaluationContext();

  Value variableValue(const std::string& name) const;

  std::optional<ExpressionNode> expression(const Value& value) const;
  std::optional<FileLocation> location(const Value& value) const;

  Value trace(Value value, const ExpressionNode& expression);
  Value trace(Value value, const Value& original);

  template <typename F, typename... Args>
  friend auto withEvaluationContext(const F& f, Args&&... args);

  deleteCopyAndMove(EvaluationContext);
};

template <typename F, typename... Args>
auto withEvaluationContext(const F& f, Args&&... args)
{
  using ReturnType = decltype(f(std::declval<EvaluationContext&>()));
  if constexpr (kdl::is_result_v<ReturnType>)
  {
    try
    {
      auto context = EvaluationContext{std::forward<Args>(args)...};
      return f(context);
    }
    catch (const el::Exception& e)
    {
      return ReturnType{Error{e.what()}};
    }
  }
  else if constexpr (std::is_same_v<ReturnType, void>)
  {
    try
    {
      auto context = EvaluationContext{std::forward<Args>(args)...};
      f(context);
      return Result<void>{};
    }
    catch (const el::Exception& e)
    {
      return Result<void>{Error{e.what()}};
    }
  }
  else
  {
    using ResultType = Result<ReturnType>;

    try
    {
      auto context = EvaluationContext{std::forward<Args>(args)...};
      return ResultType{f(context)};
    }
    catch (const el::Exception& e)
    {
      return ResultType{Error{e.what()}};
    }
  }
}

} // namespace tb::el
