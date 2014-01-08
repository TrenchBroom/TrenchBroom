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

#include "Notifier.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        const Command::CommandType SelectionCommand::Type = Command::freeType();

        SelectionCommand::SelectionCommand(View::MapDocumentWPtr document, const SelectCommand command, const Model::ObjectList& objects, const Model::BrushFaceList& faces) :
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

        SelectionCommand::Ptr SelectionCommand::select(View::MapDocumentWPtr document, const Model::ObjectList& objects) {
            return Ptr(new SelectionCommand(document, SCSelectObjects, objects, Model::EmptyBrushFaceList));
        }
        
        SelectionCommand::Ptr SelectionCommand::select(View::MapDocumentWPtr document, const Model::BrushFaceList& faces) {
            return Ptr(new SelectionCommand(document, SCSelectFaces, Model::EmptyObjectList, faces));
        }
        
        SelectionCommand::Ptr SelectionCommand::selectAllObjects(View::MapDocumentWPtr document) {
            return Ptr(new SelectionCommand(document, SCSelectAllObjects, Model::EmptyObjectList, Model::EmptyBrushFaceList));
        }
        
        SelectionCommand::Ptr SelectionCommand::selectAllFaces(View::MapDocumentWPtr document) {
            return Ptr(new SelectionCommand(document, SCSelectAllFaces, Model::EmptyObjectList, Model::EmptyBrushFaceList));
        }
        
        SelectionCommand::Ptr SelectionCommand::deselect(View::MapDocumentWPtr document, const Model::ObjectList& objects) {
            return Ptr(new SelectionCommand(document, SCDeselectObjects, objects, Model::EmptyBrushFaceList));
        }
        
        SelectionCommand::Ptr SelectionCommand::deselect(View::MapDocumentWPtr document, const Model::BrushFaceList& faces) {
            return Ptr(new SelectionCommand(document, SCDeselectFaces, Model::EmptyObjectList, faces));
        }
        
        SelectionCommand::Ptr SelectionCommand::deselectAll(View::MapDocumentWPtr document) {
            return Ptr(new SelectionCommand(document, SCDeselectAll, Model::EmptyObjectList, Model::EmptyBrushFaceList));
        }

        const Model::SelectionResult& SelectionCommand::lastResult() const {
            return m_lastResult;
        }

        bool SelectionCommand::doPerformDo() {
            View::MapDocumentSPtr document = lock(m_document);
            m_previouslySelectedObjects = document->selectedObjects();
            m_previouslySelectedFaces = document->selectedFaces();
            
            switch (m_command) {
                case SCSelectObjects:
                    m_lastResult = document->selectObjects(m_objects);
                    break;
                case SCSelectFaces:
                    m_lastResult = document->selectFaces(m_faces);
                    break;
                case SCSelectAllObjects:
                    m_lastResult = document->selectAllObjects();
                    break;
                case SCSelectAllFaces:
                    m_lastResult = document->selectAllFaces();
                    break;
                case SCDeselectObjects:
                    m_lastResult = document->deselectObjects(m_objects);
                    break;
                case SCDeselectFaces:
                    m_lastResult = document->deselectFaces(m_faces);
                    break;
                case SCDeselectAll:
                    m_lastResult = document->deselectAll();
                    break;
            }
            document->selectionDidChangeNotifier(m_lastResult);
            return true;
        }
        
        bool SelectionCommand::doPerformUndo() {
            View::MapDocumentSPtr document = lock(m_document);
            m_lastResult = document->deselectAll();
            m_lastResult += document->selectObjects(m_previouslySelectedObjects);
            m_lastResult += document->selectFaces(m_previouslySelectedFaces);
            document->selectionDidChangeNotifier(m_lastResult);
            return true;
        }
    }
}
