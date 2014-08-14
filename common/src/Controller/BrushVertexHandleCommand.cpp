/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "BrushVertexHandleCommand.h"

namespace TrenchBroom {
    namespace Controller {
        BrushVertexHandleCommand::BrushVertexHandleCommand(const CommandType type, const String& name, const bool undoable, View::MapDocumentWPtr document) :
        DocumentCommand(type, name, undoable, document) {}

        BrushVertexHandleCommand::~BrushVertexHandleCommand() {}

        Command* BrushVertexHandleCommand::doClone(View::MapDocumentSPtr document) const {
            return NULL;
        }

        void BrushVertexHandleCommand::removeBrushes(View::VertexHandleManager& manager) {
            doRemoveBrushes(manager);
        }
        
        void BrushVertexHandleCommand::addBrushes(View::VertexHandleManager& manager) {
            doAddBrushes(manager);
        }
        
        void BrushVertexHandleCommand::selectNewHandlePositions(View::VertexHandleManager& manager) {
            doSelectNewHandlePositions(manager);
        }
        
        void BrushVertexHandleCommand::selectOldHandlePositions(View::VertexHandleManager& manager) {
            doSelectOldHandlePositions(manager);
        }
    }
}
