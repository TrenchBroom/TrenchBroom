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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ControllerFacade.h"

#include "Controller/NewDocumentCommand.h"
#include "Controller/OpenDocumentCommand.h"
#include "Controller/SelectionCommand.h"
#include "TrenchBroomApp.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        void ControllerFacade::setDocument(View::MapDocument::Ptr document) {
            assert(m_document == NULL);
            assert(document != NULL);
            m_document = document;
        }

        void ControllerFacade::addCommandListener(CommandListener::Ptr listener) {
            m_commandProcessor.addCommandListener(listener);
        }
        
        void ControllerFacade::removeCommandListener(CommandListener::Ptr listener) {
            m_commandProcessor.removeCommandListener(listener);
        }

        bool ControllerFacade::newDocument(const BBox3& worldBounds, Model::Game::Ptr game) {
            Command::Ptr command = Command::Ptr(new NewDocumentCommand(m_document, worldBounds, game));
            return m_commandProcessor.submitCommand(command);
        }
        
        bool ControllerFacade::openDocument(const BBox3& worldBounds, Model::Game::Ptr game, const IO::Path& path) {
            Command::Ptr command = Command::Ptr(new OpenDocumentCommand(m_document, worldBounds, game, path));
            if (m_commandProcessor.submitCommand(command)) {
                View::TrenchBroomApp* app = static_cast<View::TrenchBroomApp*>(wxTheApp);
                if (app != NULL)
                    app->updateRecentDocument(path);
                return true;
            }
            return false;
        }

        bool ControllerFacade::selectObject(Model::Object::Ptr object) {
            Model::Object::List objects;
            objects.push_back(object);
            Command::Ptr command = Command::Ptr(new SelectionCommand(m_document, SelectionCommand::SCSelect, SelectionCommand::STObjects, objects, Model::BrushFace::EmptyList));
            return m_commandProcessor.submitAndStoreCommand(command);
        }

        bool ControllerFacade::deselectAllAndSelectObject(Model::Object::Ptr object) {
            Model::Object::List objects;
            objects.push_back(object);
            Command::Ptr selectCommand = Command::Ptr(new SelectionCommand(m_document, SelectionCommand::SCSelect, SelectionCommand::STObjects, objects, Model::BrushFace::EmptyList));
            Command::Ptr deselectCommand = Command::Ptr(new SelectionCommand(m_document, SelectionCommand::SCDeselect, SelectionCommand::STAll, Model::Object::EmptyList, Model::BrushFace::EmptyList));
            
            m_commandProcessor.beginUndoableGroup(selectCommand->name());
            m_commandProcessor.submitAndStoreCommand(deselectCommand);
            m_commandProcessor.submitAndStoreCommand(selectCommand);
            m_commandProcessor.closeGroup();
            return true;
        }
        
        bool ControllerFacade::deselectObject(Model::Object::Ptr object) {
            Model::Object::List objects;
            objects.push_back(object);
            Command::Ptr command = Command::Ptr(new SelectionCommand(m_document, SelectionCommand::SCDeselect, SelectionCommand::STObjects, objects, Model::BrushFace::EmptyList));
            return m_commandProcessor.submitAndStoreCommand(command);
        }
        
        bool ControllerFacade::selectFace(Model::BrushFace::Ptr face) {
            Model::BrushFace::List faces;
            faces.push_back(face);
            Command::Ptr command = Command::Ptr(new SelectionCommand(m_document, SelectionCommand::SCSelect, SelectionCommand::STFaces, Model::Object::EmptyList, faces));
            return m_commandProcessor.submitAndStoreCommand(command);
        }
        
        bool ControllerFacade::deselectAllAndSelectFace(Model::BrushFace::Ptr face) {
            Model::BrushFace::List faces;
            faces.push_back(face);
            Command::Ptr selectCommand = Command::Ptr(new SelectionCommand(m_document, SelectionCommand::SCSelect, SelectionCommand::STFaces, Model::Object::EmptyList, faces));
            Command::Ptr deselectCommand = Command::Ptr(new SelectionCommand(m_document, SelectionCommand::SCDeselect, SelectionCommand::STAll, Model::Object::EmptyList, Model::BrushFace::EmptyList));
            
            m_commandProcessor.beginUndoableGroup(selectCommand->name());
            m_commandProcessor.submitAndStoreCommand(deselectCommand);
            m_commandProcessor.submitAndStoreCommand(selectCommand);
            m_commandProcessor.closeGroup();
            return true;
        }
        
        bool ControllerFacade::deselectFace(Model::BrushFace::Ptr face) {
            Model::BrushFace::List faces;
            faces.push_back(face);
            Command::Ptr command = Command::Ptr(new SelectionCommand(m_document, SelectionCommand::SCDeselect, SelectionCommand::STFaces, Model::Object::EmptyList, faces));
            return m_commandProcessor.submitAndStoreCommand(command);
        }
        
        bool ControllerFacade::deselectAll() {
            Command::Ptr deselectCommand = Command::Ptr(new SelectionCommand(m_document, SelectionCommand::SCDeselect, SelectionCommand::STAll, Model::Object::EmptyList, Model::BrushFace::EmptyList));
            return m_commandProcessor.submitAndStoreCommand(deselectCommand);
        }
    }
}
