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

#include "SetCurrentLayerCommand.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
namespace View {
const Command::CommandType SetCurrentLayerCommand::Type = Command::freeType();

std::unique_ptr<SetCurrentLayerCommand> SetCurrentLayerCommand::set(Model::LayerNode* layer) {
  return std::make_unique<SetCurrentLayerCommand>(layer);
}

SetCurrentLayerCommand::SetCurrentLayerCommand(Model::LayerNode* layer)
  : UndoableCommand(Type, "Set Current Layer", false)
  , m_currentLayer(layer)
  , m_oldCurrentLayer(nullptr) {}

std::unique_ptr<CommandResult> SetCurrentLayerCommand::doPerformDo(
  MapDocumentCommandFacade* document) {
  m_oldCurrentLayer = document->performSetCurrentLayer(m_currentLayer);
  return std::make_unique<CommandResult>(true);
}

std::unique_ptr<CommandResult> SetCurrentLayerCommand::doPerformUndo(
  MapDocumentCommandFacade* document) {
  document->performSetCurrentLayer(m_oldCurrentLayer);
  return std::make_unique<CommandResult>(true);
}

bool SetCurrentLayerCommand::doCollateWith(UndoableCommand& command) {
  auto& other = static_cast<SetCurrentLayerCommand&>(command);
  m_currentLayer = other.m_currentLayer;
  return true;
}
} // namespace View
} // namespace TrenchBroom
