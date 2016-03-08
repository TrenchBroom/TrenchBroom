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

#include "SetVisibilityCommand.h"
#include "Macros.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType SetVisibilityCommand::Type = Command::freeType();

        SetVisibilityCommand* SetVisibilityCommand::show(const Model::NodeList& nodes) {
            return new SetVisibilityCommand(nodes, Model::Visibility_Shown);
        }
        
        SetVisibilityCommand* SetVisibilityCommand::hide(const Model::NodeList& nodes) {
            return new SetVisibilityCommand(nodes, Model::Visibility_Hidden);
        }
        
        SetVisibilityCommand* SetVisibilityCommand::reset(const Model::NodeList& nodes) {
            return new SetVisibilityCommand(nodes, Model::Visibility_Inherited);
        }

        SetVisibilityCommand::SetVisibilityCommand(const Model::NodeList& nodes, const Model::VisibilityState state) :
        UndoableCommand(Type, makeName(state)),
        m_nodes(nodes),
        m_state(state) {}

        String SetVisibilityCommand::makeName(const Model::VisibilityState state) {
            switch (state) {
                case Model::Visibility_Inherited:
                    return "Reset Visibility";
                case Model::Visibility_Hidden:
                    return "Hide Objects";
                case Model::Visibility_Shown:
                    return "Show Objects";
		DEFAULT_SWITCH()
            }
        }
        
        bool SetVisibilityCommand::doPerformDo(MapDocumentCommandFacade* document) {
            m_oldState = document->setVisibilityState(m_nodes, m_state);
            return true;
        }

        bool SetVisibilityCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->restoreVisibilityState(m_oldState);
            return true;
        }

        bool SetVisibilityCommand::doCollateWith(UndoableCommand* command) {
            return false;
        }

        bool SetVisibilityCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return false;
        }
    }
}
