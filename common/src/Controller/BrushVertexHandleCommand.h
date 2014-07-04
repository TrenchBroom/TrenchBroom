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

#ifndef __TrenchBroom__BrushVertexHandleCommand__
#define __TrenchBroom__BrushVertexHandleCommand__

#include "SharedPointer.h"
#include "Controller/Command.h"

namespace TrenchBroom {
    namespace View {
        class VertexHandleManager;
    }
    
    namespace Controller {
        class BrushVertexHandleCommand : public Command {
        public:
            typedef TrenchBroom::shared_ptr<BrushVertexHandleCommand> Ptr;
        public:
            BrushVertexHandleCommand(CommandType type, const String& name, bool undoable, bool modifiesDocument);
            ~BrushVertexHandleCommand();

            void removeBrushes(View::VertexHandleManager& manager);
            void addBrushes(View::VertexHandleManager& manager);
            void selectNewHandlePositions(View::VertexHandleManager& manager);
            void selectOldHandlePositions(View::VertexHandleManager& manager);
        private:
            virtual void doRemoveBrushes(View::VertexHandleManager& manager) = 0;
            virtual void doAddBrushes(View::VertexHandleManager& manager) = 0;
            virtual void doSelectNewHandlePositions(View::VertexHandleManager& manager) = 0;
            virtual void doSelectOldHandlePositions(View::VertexHandleManager& manager) = 0;
        };
    }
}

#endif /* defined(__TrenchBroom__BrushVertexHandleCommand__) */
