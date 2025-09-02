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

#include "IssueQuickFix.h"

#include "mdl/Issue.h"
#include "mdl/Map.h"
#include "mdl/Map_Entities.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/PushSelection.h"

#include <cassert>
#include <string>
#include <vector>

namespace tb::mdl
{

IssueQuickFix::IssueQuickFix(std::string description, MultiIssueFix fix)
  : m_description{std::move(description)}
  , m_fix{std::move(fix)}
{
}

IssueQuickFix::IssueQuickFix(
  IssueType issueType, std::string description, SingleIssueFix fix)
  : IssueQuickFix{
      std::move(description), [=](Map& map, const std::vector<const Issue*>& issues) {
        for (const auto* issue : issues)
        {
          if (issue->type() == issueType)
          {
            fix(map, *issue);
          }
        }
      }}
{
}

IssueQuickFix::~IssueQuickFix() = default;

const std::string& IssueQuickFix::description() const
{
  return m_description;
}

void IssueQuickFix::apply(Map& map, const std::vector<const Issue*>& issues) const
{
  m_fix(map, issues);
}

IssueQuickFix makeDeleteNodesQuickFix()
{
  return {"Delete Objects", [](Map& map, const std::vector<const Issue*>&) {
            removeSelectedNodes(map);
          }};
}

IssueQuickFix makeRemoveEntityPropertiesQuickFix(const IssueType type)
{
  return {type, "Delete Property", [](Map& map, const Issue& issue) {
            const auto pushSelection = PushSelection{map};

            const auto& entityPropertyIssue =
              static_cast<const EntityPropertyIssue&>(issue);

            // If world node is affected, the selection will fail, but if nothing is
            // selected, the removeProperty call will correctly affect worldspawn
            // either way.

            deselectAll(map);
            selectNodes(map, {&issue.node()});
            removeEntityProperty(map, entityPropertyIssue.propertyKey());
          }};
}

IssueQuickFix makeTransformEntityPropertiesQuickFix(
  const IssueType type,
  std::string description,
  std::function<std::string(const std::string&)> keyTransform,
  std::function<std::string(const std::string&)> valueTransform)
{
  return {type, std::move(description), [=](Map& map, const Issue& issue) {
            const auto pushSelection = PushSelection{map};

            const auto& propIssue = static_cast<const EntityPropertyIssue&>(issue);
            const auto& oldkey = propIssue.propertyKey();
            const auto& oldValue = propIssue.propertyValue();
            const auto newKey = keyTransform(oldkey);
            const auto newValue = valueTransform(oldValue);

            // If world node is affected, the selection will fail, but if nothing is
            // selected, the removeProperty call will correctly affect worldspawn
            // either way.

            deselectAll(map);
            selectNodes(map, {&issue.node()});

            if (newKey.empty())
            {
              removeEntityProperty(map, propIssue.propertyKey());
            }
            else
            {
              if (newKey != oldkey)
              {
                renameEntityProperty(map, oldkey, newKey);
              }
              if (newValue != oldValue)
              {
                setEntityProperty(map, newKey, newValue);
              }
            }
          }};
}

} // namespace tb::mdl
