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

        SetVisibilityCommand::Ptr SetVisibilityCommand::show(const Model::NodeList& nodes) {
            return Ptr(new SetVisibilityCommand(nodes, Action_Show));
        }
        
        SetVisibilityCommand::Ptr SetVisibilityCommand::hide(const Model::NodeList& nodes) {
            return Ptr(new SetVisibilityCommand(nodes, Action_Hide));
        }
        
        SetVisibilityCommand::Ptr SetVisibilityCommand::ensureVisible(const Model::NodeList& nodes) {
            return Ptr(new SetVisibilityCommand(nodes, Action_Ensure));
        }

        SetVisibilityCommand::Ptr SetVisibilityCommand::reset(const Model::NodeList& nodes) {
            return Ptr(new SetVisibilityCommand(nodes, Action_Reset));
        }

        SetVisibilityCommand::SetVisibilityCommand(const Model::NodeList& nodes, const Action action) :
        UndoableCommand(Type, makeName(action)),
        m_nodes(nodes),
        m_action(action) {}

        String SetVisibilityCommand::makeName(const Action action) {
            switch (action) {
                case Action_Reset:
                    return "Reset Visibility";
                case Action_Hide:
                    return "Hide Objects";
                case Action_Show:
                    return "Show Objects";
                case Action_Ensure:
                    return "Ensure Objects Visible";
                switchDefault()
            }
        }
        
        bool SetVisibilityCommand::doPerformDo(MapDocumentCommandFacade* document) {
            switch (m_action) {
                case Action_Reset:
                    m_oldState = document->setVisibilityState(m_nodes, Model::Visibility_Inherited);
                    break;
                case Action_Hide:
                    m_oldState = document->setVisibilityState(m_nodes, Model::Visibility_Hidden);
                    break;
                case Action_Show:
                    m_oldState = document->setVisibilityState(m_nodes, Model::Visibility_Shown);
                    break;
                case Action_Ensure:
                    m_oldState = document->setVisibilityEnsured(m_nodes);
                    break;
                switchDefault()
            }
            return true;
        }

        bool SetVisibilityCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->restoreVisibilityState(m_oldState);
            return true;
        }

        bool SetVisibilityCommand::doCollateWith(UndoableCommand::Ptr command) {
            return false;
        }

        bool SetVisibilityCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return false;
        }
    }
}
