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

#include "ui/MapDocumentActionCache.h"

#include "mdl/EntityDefinitionManager.h"
#include "mdl/Map.h"
#include "mdl/TagManager.h"
#include "ui/ActionManager.h"
#include "ui/MapDocument.h"

namespace tb::ui
{

MapDocumentActionCache::MapDocumentActionCache(MapDocument& document)
  : m_document{document}
{
  connectObservers();
}

void MapDocumentActionCache::connectObservers()
{
  m_notifierConnection += m_document.documentWasLoadedNotifier.connect(
    this, &MapDocumentActionCache::documentWasLoaded);
  m_notifierConnection += m_document.entityDefinitionsDidChangeNotifier.connect(
    this, &MapDocumentActionCache::entityDefinitionsDidChange);
}

std::vector<Action>& MapDocumentActionCache::cacheTagActions(
  const ActionManager& actionManager)
{
  if (!m_cachedTagActions)
  {
    m_cachedTagActions =
      actionManager.createTagActions(m_document.map().tagManager().smartTags());
  }
  return *m_cachedTagActions;
}

void MapDocumentActionCache::clearTagActions()
{
  m_cachedTagActions = std::nullopt;
}

std::vector<Action>& MapDocumentActionCache::cacheEntityDefinitionActions(
  const ActionManager& actionManager)
{
  if (!m_cachedEntityDefinitionActions)
  {
    m_cachedEntityDefinitionActions = actionManager.createEntityDefinitionActions(
      m_document.map().entityDefinitionManager().definitions());
  }
  return *m_cachedEntityDefinitionActions;
}

void MapDocumentActionCache::clearEntityDefinitionActions()
{
  m_cachedEntityDefinitionActions = std::nullopt;
}

void MapDocumentActionCache::documentWasLoaded()
{
  clearTagActions();
  clearEntityDefinitionActions();
}

void MapDocumentActionCache::entityDefinitionsDidChange()
{
  clearEntityDefinitionActions();
}

} // namespace tb::ui
