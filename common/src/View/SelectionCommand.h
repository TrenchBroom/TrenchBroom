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

#ifndef TrenchBroom_SelectionCommand
#define TrenchBroom_SelectionCommand

#include "StringUtils.h"
#include "Model/ModelTypes.h"
#include "View/UndoableCommand.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace View {
        class SelectionCommand : public UndoableCommand {
        public:
            static const CommandType Type;
        private:
            typedef enum {
                Action_SelectNodes,
                Action_SelectFaces,
                Action_SelectAllNodes,
                Action_SelectAllFaces,
                Action_ConvertToFaces,
                Action_DeselectNodes,
                Action_DeselectFaces,
                Action_DeselectAll
            } Action;
            
            Action m_action;
            
            Model::NodeList m_nodes;
            Model::BrushFaceList m_faces;
            
            Model::NodeList m_previouslySelectedNodes;
            Model::BrushFaceList m_previouslySelectedFaces;
        public:
            static SelectionCommand* select(const Model::NodeList& nodes);
            static SelectionCommand* select(const Model::BrushFaceList& faces);
            
            static SelectionCommand* convertToFaces();
            static SelectionCommand* selectAllNodes();
            static SelectionCommand* selectAllFaces();
            
            static SelectionCommand* deselect(const Model::NodeList& nodes);
            static SelectionCommand* deselect(const Model::BrushFaceList& faces);
            static SelectionCommand* deselectAll();
        private:
            SelectionCommand(Action action, const Model::NodeList& nodes, const Model::BrushFaceList& faces);
            static String makeName(Action action, const Model::NodeList& nodes, const Model::BrushFaceList& faces);
        private:
            bool doPerformDo(MapDocumentCommandFacade* document);
            bool doPerformUndo(MapDocumentCommandFacade* document);
            
            bool doIsRepeatDelimiter() const;
            bool doIsRepeatable(MapDocumentCommandFacade* document) const;
            
            bool doCollateWith(UndoableCommand::Ptr command);
        };
    }
}

#endif /* defined(TrenchBroom_SelectionCommand) */
