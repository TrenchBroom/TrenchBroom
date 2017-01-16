/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

        SelectionCommand::Ptr SelectionCommand::select(const Model::NodeArray& nodes) {
            return Ptr(new SelectionCommand(Action_SelectNodes, nodes, Model::EmptyBrushFaceArray));
        }
        
        SelectionCommand::Ptr SelectionCommand::select(const Model::BrushFaceArray& faces) {
            return Ptr(new SelectionCommand(Action_SelectFaces, Model::EmptyNodeArray, faces));
        }
        
        SelectionCommand::Ptr SelectionCommand::convertToFaces() {
            return Ptr(new SelectionCommand(Action_ConvertToFaces, Model::EmptyNodeArray, Model::EmptyBrushFaceArray));
        }
        
        SelectionCommand::Ptr SelectionCommand::selectAllNodes() {
            return Ptr(new SelectionCommand(Action_SelectAllNodes, Model::EmptyNodeArray, Model::EmptyBrushFaceArray));
        }
        
        SelectionCommand::Ptr SelectionCommand::selectAllFaces() {
            return Ptr(new SelectionCommand(Action_SelectAllFaces, Model::EmptyNodeArray, Model::EmptyBrushFaceArray));
        }
        
        SelectionCommand::Ptr SelectionCommand::deselect(const Model::NodeArray& nodes) {
            return Ptr(new SelectionCommand(Action_DeselectNodes, nodes, Model::EmptyBrushFaceArray));
        }
        
        SelectionCommand::Ptr SelectionCommand::deselect(const Model::BrushFaceArray& faces) {
            return Ptr(new SelectionCommand(Action_DeselectFaces, Model::EmptyNodeArray, faces));
        }
        
        SelectionCommand::Ptr SelectionCommand::deselectAll() {
            return Ptr(new SelectionCommand(Action_DeselectAll, Model::EmptyNodeArray, Model::EmptyBrushFaceArray));
        }

        SelectionCommand::SelectionCommand(const Action action, const Model::NodeArray& nodes, const Model::BrushFaceArray& faces) :
        UndoableCommand(Type, makeName(action, nodes, faces)),
        m_action(action),
        m_nodes(nodes),
        m_faces(faces) {}

        String SelectionCommand::makeName(const Action action, const Model::NodeArray& nodes, const Model::BrushFaceArray& faces) {
            StringStream result;
            switch (action) {
                case Action_SelectNodes:
                    result << "Select " << nodes.size() << " " << StringUtils::safePlural(nodes.size(), "Object", "Objects");
                    break;
                case Action_SelectFaces:
                    result << "Select " << faces.size() << " " << StringUtils::safePlural(nodes.size(), "Brush Face", "Brush Faces");
                    break;
                case Action_SelectAllNodes:
                    result << "Select All Objects";
                    break;
                case Action_SelectAllFaces:
                    result << "Select All Brush Faces";
                    break;
                case Action_ConvertToFaces:
                    result << "Convert to Brush Face Selection";
                    break;
                case Action_DeselectNodes:
                    result << "Deselect " << nodes.size() << " " << StringUtils::safePlural(nodes.size(), "Object", "Objects");
                    break;
                case Action_DeselectFaces:
                    result << "Deselect " << faces.size() << " " << StringUtils::safePlural(nodes.size(), "Brush Face", "Brush Faces");
                    break;
                case Action_DeselectAll:
                    return "Select None";
                switchDefault()
            }
            return result.str();
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
