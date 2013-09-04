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

#include "ControllerFacade.h"

#include "Controller/AddRemoveObjectsCommand.h"
#include "Controller/EntityPropertyCommand.h"
#include "Controller/FaceAttributeCommand.h"
#include "Controller/NewDocumentCommand.h"
#include "Controller/OpenDocumentCommand.h"
#include "Controller/SelectionCommand.h"
#include "View/ViewTypes.h"
#include "TrenchBroomApp.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        void ControllerFacade::setDocument(MapDocumentPtr document) {
            assert(m_document == NULL);
            assert(document != NULL);
            m_document = document;
        }

        void ControllerFacade::addCommandListener(Controller::CommandListener::Ptr listener) {
            m_commandProcessor.addCommandListener(listener);
        }
        
        void ControllerFacade::removeCommandListener(Controller::CommandListener::Ptr listener) {
            m_commandProcessor.removeCommandListener(listener);
        }

        bool ControllerFacade::newDocument(const BBox3& worldBounds, Model::GamePtr game) {
            using namespace Controller;
            
            Command::Ptr command = Command::Ptr(new NewDocumentCommand(m_document, worldBounds, game));
            return m_commandProcessor.submitCommand(command);
        }
        
        bool ControllerFacade::openDocument(const BBox3& worldBounds, Model::GamePtr game, const IO::Path& path) {
            using namespace Controller;
            
            Command::Ptr command = Command::Ptr(new OpenDocumentCommand(m_document, worldBounds, game, path));
            if (m_commandProcessor.submitCommand(command)) {
                View::TrenchBroomApp* app = static_cast<View::TrenchBroomApp*>(wxTheApp);
                if (app != NULL)
                    app->updateRecentDocument(path);
                return true;
            }
            return false;
        }

        void ControllerFacade::beginUndoableGroup(const String& name) {
            m_commandProcessor.beginUndoableGroup(name);
        }
        
        void ControllerFacade::beginOneShotGroup(const String& name) {
            m_commandProcessor.beginOneShotGroup(name);
        }
        
        void ControllerFacade::closeGroup() {
            m_commandProcessor.closeGroup();
        }

        void ControllerFacade::rollbackGroup() {
            m_commandProcessor.undoGroup();
        }

        bool ControllerFacade::selectObjects(const Model::ObjectList& objects) {
            using namespace Controller;
            
            Command::Ptr command = Command::Ptr(new SelectionCommand(m_document, SelectionCommand::SCSelect, SelectionCommand::STObjects, objects, Model::EmptyBrushFaceList));
            return m_commandProcessor.submitAndStoreCommand(command);
        }

        bool ControllerFacade::selectObject(Model::Object* object) {
            using namespace Controller;
            
            Model::ObjectList objects;
            objects.push_back(object);
            Command::Ptr command = Command::Ptr(new SelectionCommand(m_document, SelectionCommand::SCSelect, SelectionCommand::STObjects, objects, Model::EmptyBrushFaceList));
            return m_commandProcessor.submitAndStoreCommand(command);
        }

        bool ControllerFacade::deselectAllAndSelectObject(Model::Object* object) {
            using namespace Controller;

            Model::ObjectList objects;
            objects.push_back(object);
            Command::Ptr selectCommand = Command::Ptr(new SelectionCommand(m_document, SelectionCommand::SCSelect, SelectionCommand::STObjects, objects, Model::EmptyBrushFaceList));
            Command::Ptr deselectCommand = Command::Ptr(new SelectionCommand(m_document, SelectionCommand::SCDeselect, SelectionCommand::STAll, Model::EmptyObjectList, Model::EmptyBrushFaceList));
            
            m_commandProcessor.beginUndoableGroup(selectCommand->name());
            m_commandProcessor.submitAndStoreCommand(deselectCommand);
            m_commandProcessor.submitAndStoreCommand(selectCommand);
            m_commandProcessor.closeGroup();
            return true;
        }
        
        bool ControllerFacade::deselectObject(Model::Object* object) {
            using namespace Controller;

            Model::ObjectList objects;
            objects.push_back(object);
            Command::Ptr command = Command::Ptr(new SelectionCommand(m_document, SelectionCommand::SCDeselect, SelectionCommand::STObjects, objects, Model::EmptyBrushFaceList));
            return m_commandProcessor.submitAndStoreCommand(command);
        }
        
        bool ControllerFacade::selectFace(Model::BrushFace* face) {
            using namespace Controller;

            Model::BrushFaceList faces;
            faces.push_back(face);
            Command::Ptr command = Command::Ptr(new SelectionCommand(m_document, SelectionCommand::SCSelect, SelectionCommand::STFaces, Model::EmptyObjectList, faces));
            return m_commandProcessor.submitAndStoreCommand(command);
        }
        
        bool ControllerFacade::deselectAllAndSelectFace(Model::BrushFace* face) {
            using namespace Controller;

            Model::BrushFaceList faces;
            faces.push_back(face);
            Command::Ptr selectCommand = Command::Ptr(new SelectionCommand(m_document, SelectionCommand::SCSelect, SelectionCommand::STFaces, Model::EmptyObjectList, faces));
            Command::Ptr deselectCommand = Command::Ptr(new SelectionCommand(m_document, SelectionCommand::SCDeselect, SelectionCommand::STAll, Model::EmptyObjectList, Model::EmptyBrushFaceList));
            
            m_commandProcessor.beginUndoableGroup(selectCommand->name());
            m_commandProcessor.submitAndStoreCommand(deselectCommand);
            m_commandProcessor.submitAndStoreCommand(selectCommand);
            m_commandProcessor.closeGroup();
            return true;
        }
        
        bool ControllerFacade::deselectFace(Model::BrushFace* face) {
            using namespace Controller;

            Model::BrushFaceList faces;
            faces.push_back(face);
            Command::Ptr command = Command::Ptr(new SelectionCommand(m_document, SelectionCommand::SCDeselect, SelectionCommand::STFaces, Model::EmptyObjectList, faces));
            return m_commandProcessor.submitAndStoreCommand(command);
        }
        
        bool ControllerFacade::deselectAll() {
            using namespace Controller;

            Command::Ptr deselectCommand = Command::Ptr(new SelectionCommand(m_document, SelectionCommand::SCDeselect, SelectionCommand::STAll, Model::EmptyObjectList, Model::EmptyBrushFaceList));
            return m_commandProcessor.submitAndStoreCommand(deselectCommand);
        }

        bool ControllerFacade::addObjects(const Model::ObjectList& objects) {
            using namespace Controller;
            
            Command::Ptr command = AddRemoveObjectsCommand::addObjects(m_document, objects);
            return m_commandProcessor.submitAndStoreCommand(command);
        }

        bool ControllerFacade::renameEntityProperty(const Model::EntityList& entities, const Model::PropertyKey& oldKey, const Model::PropertyKey& newKey, const bool force) {
            using namespace Controller;

            Command::Ptr command = EntityPropertyCommand::renameEntityProperty(m_document, entities, oldKey, newKey, force);
            return m_commandProcessor.submitAndStoreCommand(command);
        }
        
        bool ControllerFacade::setEntityProperty(const Model::EntityList& entities, const Model::PropertyKey& key, const Model::PropertyValue& newValue, const bool force) {
            using namespace Controller;
            
            Command::Ptr command = EntityPropertyCommand::setEntityProperty(m_document, entities, key, newValue, force);
            return m_commandProcessor.submitAndStoreCommand(command);
        }

        bool ControllerFacade::removeEntityProperty(const Model::EntityList& entities, const Model::PropertyKey& key, const bool force) {
            using namespace Controller;

            Command::Ptr command = EntityPropertyCommand::removeEntityProperty(m_document, entities, key, force);
            return m_commandProcessor.submitAndStoreCommand(command);
        }

        bool ControllerFacade::setFaceXOffset(const Model::BrushFaceList& faces, const float xOffset) {
            using namespace Controller;

            FaceAttributeCommand::Ptr command(new FaceAttributeCommand(m_document, faces));
            command->setXOffset(xOffset);
            return m_commandProcessor.submitAndStoreCommand(command);
        }
        
        bool ControllerFacade::setFaceYOffset(const Model::BrushFaceList& faces, const float yOffset) {
            using namespace Controller;
            
            FaceAttributeCommand::Ptr command(new FaceAttributeCommand(m_document, faces));
            command->setYOffset(yOffset);
            return m_commandProcessor.submitAndStoreCommand(command);
        }
        
        bool ControllerFacade::setFaceRotation(const Model::BrushFaceList& faces, const float rotation) {
            using namespace Controller;
            
            FaceAttributeCommand::Ptr command(new FaceAttributeCommand(m_document, faces));
            command->setRotation(rotation);
            return m_commandProcessor.submitAndStoreCommand(command);
        }
        
        bool ControllerFacade::setFaceXScale(const Model::BrushFaceList& faces, const float xScale) {
            using namespace Controller;
            
            FaceAttributeCommand::Ptr command(new FaceAttributeCommand(m_document, faces));
            command->setXScale(xScale);
            return m_commandProcessor.submitAndStoreCommand(command);
        }
        
        bool ControllerFacade::setFaceYScale(const Model::BrushFaceList& faces, const float yScale) {
            using namespace Controller;
            
            FaceAttributeCommand::Ptr command(new FaceAttributeCommand(m_document, faces));
            command->setYScale(yScale);
            return m_commandProcessor.submitAndStoreCommand(command);
        }
    }
}
