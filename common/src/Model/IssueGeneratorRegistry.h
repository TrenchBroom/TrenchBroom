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

#include <vector>

namespace TrenchBroom {
namespace Model {
class IssueGenerator;
class IssueQuickFix;

class IssueGeneratorRegistry {
private:
  std::vector<IssueGenerator*> m_generators;

public:
  ~IssueGeneratorRegistry();

  const std::vector<IssueGenerator*>& registeredGenerators() const;
  std::vector<IssueQuickFix*> quickFixes(IssueType issueTypes) const;

  void registerGenerator(IssueGenerator* generator);
  void unregisterAllGenerators();

private:
  void clearGenerators();
};
} // namespace Model
} // namespace TrenchBroom
