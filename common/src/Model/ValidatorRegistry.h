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

#include "Model/IssueType.h"

#include <memory>
#include <vector>

namespace TrenchBroom {
namespace Model {
class IssueQuickFix;
class Validator;

class ValidatorRegistry {
private:
  std::vector<std::unique_ptr<Validator>> m_validators;

public:
  ~ValidatorRegistry();

  std::vector<const Validator*> registeredValidators() const;
  std::vector<const IssueQuickFix*> quickFixes(IssueType issueTypes) const;

  void registerValidator(const std::unique_ptr<Validator> validator);
  void unregisterAllValidators();
};
} // namespace Model
} // namespace TrenchBroom
