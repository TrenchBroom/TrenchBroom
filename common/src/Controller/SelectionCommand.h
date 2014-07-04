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

#ifndef __TrenchBroom__SelectionCommand__
#define __TrenchBroom__SelectionCommand__

#include "SharedPointer.h"
#include "StringUtils.h"
#include "Controller/Command.h"
#include "Model/ModelTypes.h"
#include "Model/SelectionResult.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Model {
        class Map;
    }
    
    namespace Controller {
        class SelectionCommand : public Command {
        public:
            static const CommandType Type;
            typedef TrenchBroom::shared_ptr<SelectionCommand> Ptr;

            typedef enum {
                Action_SelectObjects,
                Action_SelectFaces,
                Action_SelectAllObjects,
                Action_SelectAllFaces,
                Action_DeselectObjects,
                Action_DeselectFaces,
                Action_DeselectAll
            } Action;
        private:
            View::MapDocumentWPtr m_document;
            Action m_action;
            
            Model::ObjectList m_objects;
            Model::BrushFaceList m_faces;
            bool m_keepBrushSelection;

            Model::ObjectList m_previouslySelectedObjects;
            Model::BrushFaceList m_previouslySelectedFaces;
            
            Model::SelectionResult m_lastResult;
        public:
            static Ptr select(View::MapDocumentWPtr document, const Model::ObjectList& objects);
            static Ptr select(View::MapDocumentWPtr document, const Model::BrushFaceList& faces);
            static Ptr selectAndKeepBrushes(View::MapDocumentWPtr document, const Model::BrushFaceList& faces);
            static Ptr selectAllObjects(View::MapDocumentWPtr document);
            static Ptr selectAllFaces(View::MapDocumentWPtr document);
            
            static Ptr deselect(View::MapDocumentWPtr document, const Model::ObjectList& objects);
            static Ptr deselect(View::MapDocumentWPtr document, const Model::BrushFaceList& faces);
            static Ptr deselectAll(View::MapDocumentWPtr document);

            const Model::SelectionResult& lastResult() const;
        private:
            SelectionCommand(View::MapDocumentWPtr document, Action command, const Model::ObjectList& objects, const Model::BrushFaceList& faces, bool keepBrushSelection);

            static String makeName(Action command, const Model::ObjectList& objects, const Model::BrushFaceList& faces);
            
            bool doPerformDo();
            bool doPerformUndo();
            bool doCollateWith(Command::Ptr command);
        };
    }
}

#endif /* defined(__TrenchBroom__SelectionCommand__) */
