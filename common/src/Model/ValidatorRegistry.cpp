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

#include <kdl/vector_utils.h>

#include <cassert>

namespace TrenchBroom {
namespace Model {
ValidatorRegistry::~ValidatorRegistry() {
  clearValidators();
}

const std::vector<Validator*>& ValidatorRegistry::registeredValidators() const {
  return m_validators;
}

std::vector<IssueQuickFix*> ValidatorRegistry::quickFixes(const IssueType issueTypes) const {
  std::vector<IssueQuickFix*> result;
  for (const Validator* validator : m_validators) {
    if ((validator->type() & issueTypes) != 0)
      result = kdl::vec_concat(std::move(result), validator->quickFixes());
  }
  return result;
}

void ValidatorRegistry::registerValidator(Validator* validator) {
  ensure(validator != nullptr, "validator is null");
  assert(!kdl::vec_contains(m_validators, validator));
  m_validators.push_back(validator);
}

void ValidatorRegistry::unregisterAllValidators() {
  clearValidators();
}

void ValidatorRegistry::clearValidators() {
  kdl::vec_clear_and_delete(m_validators);
}
} // namespace Model
} // namespace TrenchBroom
