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

#include "Notifier.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        const Command::CommandType SelectionCommand::Type = Command::freeType();

        SelectionCommand::SelectionCommand(View::MapDocumentPtr document, const SelectCommand command, const Model::ObjectList& objects, const Model::BrushFaceList& faces) :
        Command(Type, makeName(command, objects, faces), true, false),
        m_document(document),
        m_command(command),
        m_objects(objects),
        m_faces(faces) {}

        String SelectionCommand::makeName(const SelectCommand command, const Model::ObjectList& objects, const Model::BrushFaceList& faces) {
            StringStream result;
            switch (command) {
                case SCSelectObjects:
                    result << "Select " << objects.size() << (objects.size() == 1 ? " object" : " objects");
                    break;
                case SCSelectFaces:
                    result << "Select " << faces.size() << (faces.size() == 1 ? " face" : " faces");
                    break;
                case SCSelectAllObjects:
                    result << "Select all objects";
                    break;
                case SCSelectAllFaces:
                    result << "Select all faces";
                    break;
                case SCDeselectObjects:
                    result << "Deselect " << objects.size() << (objects.size() == 1 ? " object" : " objects");
                    break;
                case SCDeselectFaces:
                    result << "Deselect " << faces.size() << (faces.size() == 1 ? " face" : " faces");
                    break;
                case SCDeselectAll:
                    result << "Deselect all";
                    break;
            }
            return result.str();
        }

        SelectionCommand::Ptr SelectionCommand::select(View::MapDocumentPtr document, const Model::ObjectList& objects) {
            return Ptr(new SelectionCommand(document, SCSelectObjects, objects, Model::EmptyBrushFaceList));
        }
        
        SelectionCommand::Ptr SelectionCommand::select(View::MapDocumentPtr document, const Model::BrushFaceList& faces) {
            return Ptr(new SelectionCommand(document, SCSelectFaces, Model::EmptyObjectList, faces));
        }
        
        SelectionCommand::Ptr SelectionCommand::selectAllObjects(View::MapDocumentPtr document) {
            return Ptr(new SelectionCommand(document, SCSelectAllObjects, Model::EmptyObjectList, Model::EmptyBrushFaceList));
        }
        
        SelectionCommand::Ptr SelectionCommand::selectAllFaces(View::MapDocumentPtr document) {
            return Ptr(new SelectionCommand(document, SCSelectAllFaces, Model::EmptyObjectList, Model::EmptyBrushFaceList));
        }
        
        SelectionCommand::Ptr SelectionCommand::deselect(View::MapDocumentPtr document, const Model::ObjectList& objects) {
            return Ptr(new SelectionCommand(document, SCDeselectObjects, objects, Model::EmptyBrushFaceList));
        }
        
        SelectionCommand::Ptr SelectionCommand::deselect(View::MapDocumentPtr document, const Model::BrushFaceList& faces) {
            return Ptr(new SelectionCommand(document, SCDeselectFaces, Model::EmptyObjectList, faces));
        }
        
        SelectionCommand::Ptr SelectionCommand::deselectAll(View::MapDocumentPtr document) {
            return Ptr(new SelectionCommand(document, SCDeselectAll, Model::EmptyObjectList, Model::EmptyBrushFaceList));
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
                case SCSelectObjects:
                    m_lastResult = m_document->selectObjects(m_objects);
                    break;
                case SCSelectFaces:
                    m_lastResult = m_document->selectFaces(m_faces);
                    break;
                case SCSelectAllObjects:
                    m_lastResult = m_document->selectAllObjects();
                    break;
                case SCSelectAllFaces:
                    m_lastResult = m_document->selectAllFaces();
                    break;
                case SCDeselectObjects:
                    m_lastResult = m_document->deselectObjects(m_objects);
                    break;
                case SCDeselectFaces:
                    m_lastResult = m_document->deselectFaces(m_faces);
                    break;
                case SCDeselectAll:
                    m_lastResult = m_document->deselectAll();
                    break;
            }
            m_document->selectionDidChangeNotifier(m_lastResult);
            return true;
        }
        
        bool SelectionCommand::doPerformUndo() {
            m_lastResult = m_document->deselectAll();
            m_lastResult += m_document->selectObjects(m_previouslySelectedObjects);
            m_lastResult += m_document->selectFaces(m_previouslySelectedFaces);
            m_document->selectionDidChangeNotifier(m_lastResult);
            return true;
        }
    }
}
