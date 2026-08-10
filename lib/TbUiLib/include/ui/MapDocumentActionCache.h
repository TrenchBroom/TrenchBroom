/*
 Copyright (C) 2026 Kristian Duske

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

#include "base/NotifierConnection.h"
#include "ui/Action.h"

#include <optional>
#include <vector>

namespace tb::ui
{
class ActionManager;
class MapDocument;

/**
 * Caches the tag and entity definition actions for a MapDocument, since building
 * them requires querying the ActionManager, which is expensive to do repeatedly.
 * The cache is invalidated automatically when the document is (re)loaded or its
 * entity definitions change.
 */
class MapDocumentActionCache
{
private:
  MapDocument& m_document;

  std::optional<std::vector<Action>> m_cachedTagActions;
  std::optional<std::vector<Action>> m_cachedEntityDefinitionActions;

  NotifierConnection m_notifierConnection;

public:
  explicit MapDocumentActionCache(MapDocument& document);

  template <typename ActionVisitor>
  void visitTagActions(const ActionManager& actionManager, ActionVisitor&& visitor)
  {
    for (auto& action : cacheTagActions(actionManager))
    {
      visitor(action);
    }
  }

  template <typename ActionVisitor>
  void visitEntityDefinitionActions(
    const ActionManager& actionManager, ActionVisitor&& visitor)
  {
    for (auto& action : cacheEntityDefinitionActions(actionManager))
    {
      visitor(action);
    }
  }

private:
  std::vector<Action>& cacheTagActions(const ActionManager& actionManager);
  void clearTagActions();

  std::vector<Action>& cacheEntityDefinitionActions(const ActionManager& actionManager);
  void clearEntityDefinitionActions();

  void connectObservers();
  void documentWasLoaded();
  void entityDefinitionsDidChange();
};

} // namespace tb::ui
