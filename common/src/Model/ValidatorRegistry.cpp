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

#include "ValidatorRegistry.h"

#include "Ensure.h"
#include "Model/Validator.h"

#include "kdl/vector_utils.h"

#include <cassert>

namespace TrenchBroom
{
namespace Model
{
ValidatorRegistry::~ValidatorRegistry() = default;

std::vector<const Validator*> ValidatorRegistry::registeredValidators() const
{
  return kdl::vec_transform(m_validators, [](const auto& validator) {
    return const_cast<const Validator*>(validator.get());
  });
}

std::vector<const IssueQuickFix*> ValidatorRegistry::quickFixes(
  const IssueType issueTypes) const
{
  auto result = std::vector<const IssueQuickFix*>{};
  for (const auto& validator : m_validators)
  {
    if ((validator->type() & issueTypes) != 0)
    {
      result = kdl::vec_concat(std::move(result), validator->quickFixes());
    }
  }
  return result;
}

void ValidatorRegistry::registerValidator(std::unique_ptr<Validator> validator)
{
  ensure(validator != nullptr, "validator is null");
  assert(!kdl::vec_contains(m_validators, validator));
  m_validators.push_back(std::move(validator));
}

void ValidatorRegistry::unregisterAllValidators()
{
  m_validators.clear();
}
} // namespace Model
} // namespace TrenchBroom
