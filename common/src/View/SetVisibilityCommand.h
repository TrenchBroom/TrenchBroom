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

#ifndef __TrenchBroom__SetVisibilityCommand__
#define __TrenchBroom__SetVisibilityCommand__

#include "Model/ModelTypes.h"
#include "View/UndoableCommand.h"

#include <map>

namespace TrenchBroom {
    namespace View {
        class SetVisibilityCommand : public UndoableCommand {
        public:
            static const CommandType Type;
        private:
            Model::NodeList m_nodes;
            Model::VisibilityState m_state;
            Model::VisibilityMap m_oldState;
        public:
            static SetVisibilityCommand* show(const Model::NodeList& nodes);
            static SetVisibilityCommand* hide(const Model::NodeList& nodes);
            static SetVisibilityCommand* reset(const Model::NodeList& nodes);
        private:
            SetVisibilityCommand(const Model::NodeList& nodes, Model::VisibilityState state);
            static String makeName(Model::VisibilityState state);
        private:
            bool doPerformDo(MapDocumentCommandFacade* document);
            bool doPerformUndo(MapDocumentCommandFacade* document);
            
            bool doCollateWith(UndoableCommand* command);
            bool doIsRepeatable(MapDocumentCommandFacade* document) const;
        };
    }
}

#endif /* defined(__TrenchBroom__SetVisibilityCommand__) */
