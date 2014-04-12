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

        SelectionCommand::SelectionCommand(View::MapDocumentWPtr document, const Action action, const Model::ObjectList& objects, const Model::BrushFaceList& faces, const bool keepBrushSelection) :
        Command(Type, makeName(action, objects, faces), true, false),
        m_document(document),
        m_action(action),
        m_objects(objects),
        m_faces(faces),
        m_keepBrushSelection(keepBrushSelection) {}

        String SelectionCommand::makeName(const Action action, const Model::ObjectList& objects, const Model::BrushFaceList& faces) {
            StringStream result;
            switch (action) {
                case Action_SelectObjects:
                    result << "Select " << objects.size() << (objects.size() == 1 ? " object" : " objects");
                    break;
                case Action_SelectFaces:
                    result << "Select " << faces.size() << (faces.size() == 1 ? " face" : " faces");
                    break;
                case Action_SelectAllObjects:
                    result << "Select all objects";
                    break;
                case Action_SelectAllFaces:
                    result << "Select all faces";
                    break;
                case Action_DeselectObjects:
                    result << "Deselect " << objects.size() << (objects.size() == 1 ? " object" : " objects");
                    break;
                case Action_DeselectFaces:
                    result << "Deselect " << faces.size() << (faces.size() == 1 ? " face" : " faces");
                    break;
                case Action_DeselectAll:
                    result << "Deselect all";
                    break;
            }
            return result.str();
        }

        SelectionCommand::Ptr SelectionCommand::select(View::MapDocumentWPtr document, const Model::ObjectList& objects) {
            return Ptr(new SelectionCommand(document, Action_SelectObjects, objects, Model::EmptyBrushFaceList, false));
        }
        
        SelectionCommand::Ptr SelectionCommand::select(View::MapDocumentWPtr document, const Model::BrushFaceList& faces) {
            return Ptr(new SelectionCommand(document, Action_SelectFaces, Model::EmptyObjectList, faces, false));
        }
        
        SelectionCommand::Ptr SelectionCommand::selectAndKeepBrushes(View::MapDocumentWPtr document, const Model::BrushFaceList& faces) {
            return Ptr(new SelectionCommand(document, Action_SelectFaces, Model::EmptyObjectList, faces, true));
        }
        
        SelectionCommand::Ptr SelectionCommand::selectAllObjects(View::MapDocumentWPtr document) {
            return Ptr(new SelectionCommand(document, Action_SelectAllObjects, Model::EmptyObjectList, Model::EmptyBrushFaceList, false));
        }
        
        SelectionCommand::Ptr SelectionCommand::selectAllFaces(View::MapDocumentWPtr document) {
            return Ptr(new SelectionCommand(document, Action_SelectAllFaces, Model::EmptyObjectList, Model::EmptyBrushFaceList, false));
        }
        
        SelectionCommand::Ptr SelectionCommand::deselect(View::MapDocumentWPtr document, const Model::ObjectList& objects) {
            return Ptr(new SelectionCommand(document, Action_DeselectObjects, objects, Model::EmptyBrushFaceList, false));
        }
        
        SelectionCommand::Ptr SelectionCommand::deselect(View::MapDocumentWPtr document, const Model::BrushFaceList& faces) {
            return Ptr(new SelectionCommand(document, Action_DeselectFaces, Model::EmptyObjectList, faces, false));
        }
        
        SelectionCommand::Ptr SelectionCommand::deselectAll(View::MapDocumentWPtr document) {
            return Ptr(new SelectionCommand(document, Action_DeselectAll, Model::EmptyObjectList, Model::EmptyBrushFaceList, false));
        }

        const Model::SelectionResult& SelectionCommand::lastResult() const {
            return m_lastResult;
        }

        bool SelectionCommand::doPerformDo() {
            View::MapDocumentSPtr document = lock(m_document);
            m_previouslySelectedObjects = document->selectedObjects();
            m_previouslySelectedFaces = document->selectedFaces();
            
            switch (m_action) {
                case Action_SelectObjects:
                    m_lastResult = document->selectObjects(m_objects);
                    break;
                case Action_SelectFaces:
                    m_lastResult = document->selectFaces(m_faces, m_keepBrushSelection);
                    break;
                case Action_SelectAllObjects:
                    m_lastResult = document->selectAllObjects();
                    break;
                case Action_SelectAllFaces:
                    m_lastResult = document->selectAllFaces();
                    break;
                case Action_DeselectObjects:
                    m_lastResult = document->deselectObjects(m_objects);
                    break;
                case Action_DeselectFaces:
                    m_lastResult = document->deselectFaces(m_faces);
                    break;
                case Action_DeselectAll:
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
            m_lastResult += document->selectFaces(m_previouslySelectedFaces, false);
            document->selectionDidChangeNotifier(m_lastResult);
            return true;
        }
    }
}
