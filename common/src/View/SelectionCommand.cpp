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

#include "SelectionCommand.h"

#include "Macros.h"
#include "Model/Brush.h"
#include "Model/EditorContext.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/NodeVisitor.h"
#include "Model/World.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType SelectionCommand::Type = Command::freeType();

        SelectionCommand* SelectionCommand::select(const Model::NodeList& nodes) {
            return new SelectionCommand(Action_SelectNodes, nodes, Model::EmptyBrushFaceList);
        }
        
        SelectionCommand* SelectionCommand::select(const Model::BrushFaceList& faces) {
            return new SelectionCommand(Action_SelectFaces, Model::EmptyNodeList, faces);
        }
        
        SelectionCommand* SelectionCommand::convertToFaces() {
            return new SelectionCommand(Action_ConvertToFaces, Model::EmptyNodeList, Model::EmptyBrushFaceList);
        }
        
        SelectionCommand* SelectionCommand::selectAllNodes() {
            return new SelectionCommand(Action_SelectAllNodes, Model::EmptyNodeList, Model::EmptyBrushFaceList);
        }
        
        SelectionCommand* SelectionCommand::selectAllFaces() {
            return new SelectionCommand(Action_SelectAllFaces, Model::EmptyNodeList, Model::EmptyBrushFaceList);
        }
        
        SelectionCommand* SelectionCommand::deselect(const Model::NodeList& nodes) {
            return new SelectionCommand(Action_DeselectNodes, nodes, Model::EmptyBrushFaceList);
        }
        
        SelectionCommand* SelectionCommand::deselect(const Model::BrushFaceList& faces) {
            return new SelectionCommand(Action_DeselectFaces, Model::EmptyNodeList, faces);
        }
        
        SelectionCommand* SelectionCommand::deselectAll() {
            return new SelectionCommand(Action_DeselectAll, Model::EmptyNodeList, Model::EmptyBrushFaceList);
        }

        SelectionCommand::SelectionCommand(const Action action, const Model::NodeList& nodes, const Model::BrushFaceList& faces) :
        UndoableCommand(Type, makeName(action, nodes, faces)),
        m_action(action),
        m_nodes(nodes),
        m_faces(faces) {}

        String SelectionCommand::makeName(const Action action, const Model::NodeList& nodes, const Model::BrushFaceList& faces) {
            switch (action) {
                case Action_SelectNodes:
                    return  StringUtils::safePlural(nodes.size(), "Select object", "Select objects");
                case Action_SelectFaces:
                    return  StringUtils::safePlural(faces.size(), "Select face", "Select faces");
                case Action_SelectAllNodes:
                    return "Select all objects";
                case Action_SelectAllFaces:
                    return "Select all faces";
                case Action_ConvertToFaces:
                    return "Convert to face selection";
                case Action_DeselectNodes:
                    return  StringUtils::safePlural(nodes.size(), "Deselect object", "Select objects");
                case Action_DeselectFaces:
                    return  StringUtils::safePlural(faces.size(), "Deselect face", "Select faces");
                case Action_DeselectAll:
                    return "Deselect all";
                switchDefault()
            }
        }

        bool SelectionCommand::doPerformDo(MapDocumentCommandFacade* document) {
            m_previouslySelectedNodes = document->selectedNodes().nodes();
            m_previouslySelectedFaces = document->selectedBrushFaces();
            
            switch (m_action) {
                case Action_SelectNodes:
                    document->performSelect(m_nodes);
                    break;
                case Action_SelectFaces:
                    document->performSelect(m_faces);
                    break;
                case Action_SelectAllNodes:
                    document->performSelectAllNodes();
                    break;
                case Action_SelectAllFaces:
                    document->performSelectAllBrushFaces();
                    break;
                case Action_ConvertToFaces:
                    document->performConvertToBrushFaceSelection();
                    break;
                case Action_DeselectNodes:
                    document->performDeselect(m_nodes);
                    break;
                case Action_DeselectFaces:
                    document->performDeselect(m_faces);
                    break;
                case Action_DeselectAll:
                    document->performDeselectAll();
                    break;
            }
            return true;
        }
        
        bool SelectionCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->performDeselectAll();
            if (!m_previouslySelectedNodes.empty())
                document->performSelect(m_previouslySelectedNodes);
            if (!m_previouslySelectedFaces.empty())
                document->performSelect(m_previouslySelectedFaces);
            return true;
        }
        
        bool SelectionCommand::doIsRepeatDelimiter() const {
            return true;
        }
        
        bool SelectionCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return false;
        }
        
        bool SelectionCommand::doCollateWith(UndoableCommand::Ptr command) {
            return false;
        }
    }
}
