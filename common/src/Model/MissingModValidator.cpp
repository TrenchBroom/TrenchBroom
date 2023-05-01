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

#include "MissingModValidator.h"

#include "IO/Path.h"
#include "Model/Entity.h"
#include "Model/EntityNodeBase.h"
#include "Model/EntityProperties.h"
#include "Model/Game.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"
#include "Model/PushSelection.h"

#include <kdl/memory_utils.h>
#include <kdl/vector_utils.h>

#include <cassert>
#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom
{
namespace Model
{
namespace
{
static const auto Type = freeIssueType();

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
      mods = kdl::vec_erase(std::move(mods), missingMod);
    }
  }
  return mods;
}

IssueQuickFix makeRemoveModsQuickFix()
{
  return {"Remove Mod", [](MapFacade& facade, const std::vector<const Issue*>& issues) {
            const auto pushSelection = PushSelection{facade};

            // If nothing is selected, property changes will affect only world.
            facade.deselectAll();

            const auto oldMods = facade.mods();
            const auto newMods = removeMissingMods(oldMods, issues);
            facade.setMods(newMods);
          }};
}
} // namespace

MissingModValidator::MissingModValidator(std::weak_ptr<Game> game)
  : Validator{Type, "Missing mod directory"}
  , m_game{std::move(game)}
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

  if (kdl::mem_expired(m_game))
  {
    return;
  }

  auto game = kdl::mem_lock(m_game);
  auto mods = game->extractEnabledMods(entityNode.entity());

  if (mods == m_lastMods)
  {
    return;
  }

  const auto additionalSearchPaths =
    kdl::vec_transform(mods, [](const auto& mod) { return IO::Path{mod}; });
  const auto errors = game->checkAdditionalSearchPaths(additionalSearchPaths);

  for (const auto& [searchPath, message] : errors)
  {
    const auto mod = searchPath.string();
    issues.push_back(std::make_unique<MissingModIssue>(
      entityNode, mod, "Mod '" + mod + "' could not be used: " + message));
  }

  m_lastMods = std::move(mods);
}
} // namespace Model
} // namespace TrenchBroom
