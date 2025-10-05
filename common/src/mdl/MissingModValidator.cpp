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

#include "MissingModValidator.h"

#include "mdl/Entity.h"
#include "mdl/EntityNodeBase.h"
#include "mdl/EntityProperties.h"
#include "mdl/Game.h"
#include "mdl/Issue.h"
#include "mdl/IssueQuickFix.h"
#include "mdl/Map.h"
#include "mdl/Map_Selection.h"
#include "mdl/Map_World.h"
#include "mdl/PushSelection.h"

#include "kdl/ranges/to.h"

#include <fmt/format.h>
#include <fmt/std.h>

#include <cassert>
#include <filesystem>
#include <ranges>
#include <string>
#include <vector>

namespace tb::mdl
{
namespace
{
const auto Type = freeIssueType();

class MissingModIssue : public Issue
{
private:
  std::string m_mod;

public:
  MissingModIssue(EntityNodeBase& entityNode, std::string mod, std::string description)
    : Issue{Type, entityNode, std::move(description)}
    , m_mod{std::move(mod)}
  {
  }

  const std::string& mod() const { return m_mod; }
};

std::vector<std::string> removeMissingMods(
  std::vector<std::string> mods, const std::vector<const Issue*>& issues)
{
  for (const auto* issue : issues)
  {
    if (issue->type() == Type)
    {
      const auto* modIssue = static_cast<const MissingModIssue*>(issue);
      const auto& missingMod = modIssue->mod();
      std::erase(mods, missingMod);
    }
  }
  return mods;
}

IssueQuickFix makeRemoveModsQuickFix()
{
  return {"Remove Mod", [](Map& map, const std::vector<const Issue*>& issues) {
            const auto pushSelection = PushSelection{map};

            // If nothing is selected, property changes will affect only world.
            deselectAll(map);

            const auto oldMods = enabledMods(map);
            const auto newMods = removeMissingMods(oldMods, issues);
            setEnabledMods(map, newMods);
          }};
}
} // namespace

MissingModValidator::MissingModValidator(const Game& game)
  : Validator{Type, "Missing mod directory"}
  , m_game{game}
{
  addQuickFix(makeRemoveModsQuickFix());
}

void MissingModValidator::doValidate(
  EntityNodeBase& entityNode, std::vector<std::unique_ptr<Issue>>& issues) const
{
  if (entityNode.entity().classname() != EntityPropertyValues::WorldspawnClassname)
  {
    return;
  }

  auto mods = mdl::enabledMods(entityNode.entity());
  if (mods == m_lastMods)
  {
    return;
  }

  const auto additionalSearchPaths =
    mods
    | std::views::transform([](const auto& mod) { return std::filesystem::path{mod}; })
    | kdl::ranges::to<std::vector>();
  const auto errors = m_game.checkAdditionalSearchPaths(additionalSearchPaths);

  for (const auto& [searchPath, message] : errors)
  {
    const auto mod = searchPath.string();
    issues.push_back(std::make_unique<MissingModIssue>(
      entityNode, mod, fmt::format("Mod '{}' could not be used: {}", mod, message)));
  }

  m_lastMods = std::move(mods);
}

} // namespace tb::mdl
