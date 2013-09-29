/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        const Command::CommandType SelectionCommand::Type = Command::freeType();

        SelectionCommand::SelectionCommand(View::MapDocumentPtr document, const SelectCommand command, const SelectTarget target, const Model::ObjectList& objects, const Model::BrushFaceList& faces) :
        Command(Type, makeName(command, target, objects, faces), true, false),
        m_document(document),
        m_command(command),
        m_target(target),
        m_objects(objects),
        m_faces(faces) {}

        String SelectionCommand::makeName(const SelectCommand command, const SelectTarget target, const Model::ObjectList& objects, const Model::BrushFaceList& faces) {
            StringStream result;
            switch (command) {
                case SCSelect:
                    result << "Select ";
                    break;
                case SCDeselect:
                    result << "Deselect ";
                    break;
            }
            switch (target) {
                case STObjects:
                    result << objects.size() << (objects.size() == 1 ? " Object" : " Objects");
                    break;
                case STFaces:
                    result << faces.size() << (faces.size() == 1 ? " Face" : " Faces");
                    break;
                case STAll:
                    result << "All";
                    break;
            };
            return result.str();
        }

        View::MapDocumentPtr SelectionCommand::document() const {
            return m_document;
        }

        const Model::SelectionResult& SelectionCommand::lastResult() const {
            return m_lastResult;
        }

        bool SelectionCommand::doPerformDo() {
            m_previouslySelectedObjects = m_document->selectedObjects();
            m_previouslySelectedFaces = m_document->selectedFaces();
            
            switch (m_command) {
                case SCSelect:
                    switch (m_target) {
                        case STObjects:
                            m_lastResult = m_document->selectObjects(m_objects);
                            break;
                        case STFaces:
                            m_lastResult = m_document->selectFaces(m_faces);
                            break;
                        case STAll:
                            m_lastResult = m_document->selectAllObjects();
                            break;
                    }
                    break;
                case SCDeselect:
                    switch (m_target) {
                        case STObjects:
                            m_lastResult = m_document->deselectObjects(m_objects);
                            break;
                        case STFaces:
                            m_lastResult = m_document->deselectFaces(m_faces);
                            break;
                        case STAll:
                            m_lastResult = m_document->deselectAll();
                            break;
                    }
                    break;
            }
            return true;
        }
        
        bool SelectionCommand::doPerformUndo() {
            m_lastResult = m_document->deselectAll();
            m_lastResult += m_document->selectObjects(m_previouslySelectedObjects);
            m_lastResult += m_document->selectFaces(m_previouslySelectedFaces);
            return true;
        }
    }
}
