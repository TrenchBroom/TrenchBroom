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

#include "mdl/IssueType.h"

#include <functional>
#include <string>
#include <vector>

namespace tb::mdl
{
class Issue;
class Map;

class IssueQuickFix
{
private:
  using SingleIssueFix = std::function<void(Map&, const Issue&)>;
  using MultiIssueFix = std::function<void(Map&, const std::vector<const Issue*>&)>;

  std::string m_description;
  MultiIssueFix m_fix;

public:
  IssueQuickFix(std::string description, MultiIssueFix fix);
  IssueQuickFix(IssueType issueType, std::string description, SingleIssueFix fix);
  virtual ~IssueQuickFix();

  const std::string& description() const;

  void apply(Map& map, const std::vector<const Issue*>& issues) const;
};

IssueQuickFix makeDeleteNodesQuickFix();

IssueQuickFix makeRemoveEntityPropertiesQuickFix(IssueType type);

IssueQuickFix makeTransformEntityPropertiesQuickFix(
  IssueType type,
  std::string description,
  std::function<std::string(const std::string&)> keyTransform,
  std::function<std::string(const std::string&)> valueTransform);

} // namespace tb::mdl
