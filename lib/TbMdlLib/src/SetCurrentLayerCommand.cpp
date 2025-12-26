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

#include "mdl/SetCurrentLayerCommand.h"

#include "mdl/EditorContext.h"
#include "mdl/Map.h"

namespace tb::mdl
{

std::unique_ptr<SetCurrentLayerCommand> SetCurrentLayerCommand::set(LayerNode* layer)
{
  return std::make_unique<SetCurrentLayerCommand>(layer);
}

SetCurrentLayerCommand::SetCurrentLayerCommand(LayerNode* layer)
  : UndoableCommand{"Set Current Layer", false}
  , m_currentLayer{layer}
{
}

bool SetCurrentLayerCommand::doPerformDo(Map& map)
{
  auto& editorContext = map.editorContext();
  m_oldCurrentLayer = editorContext.currentLayer();
  editorContext.setCurrentLayer(m_currentLayer);
  map.currentLayerDidChangeNotifier();
  return true;
}

bool SetCurrentLayerCommand::doPerformUndo(Map& map)
{
  auto& editorContext = map.editorContext();
  editorContext.setCurrentLayer(m_oldCurrentLayer);
  map.currentLayerDidChangeNotifier();
  return true;
}

bool SetCurrentLayerCommand::doCollateWith(UndoableCommand& command)
{
  if (auto* other = dynamic_cast<SetCurrentLayerCommand*>(&command))
  {
    m_currentLayer = other->m_currentLayer;
    return true;
  }
  return false;
}

} // namespace tb::mdl
