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

#include "AddRemoveObjectsCommand.h"

#include "CollectionUtils.h"
#include "Model/Entity.h"
#include "Model/Object.h"
#include "Model/ModelUtils.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        const Command::CommandType AddRemoveObjectsCommand::Type = Command::freeType();

        class AddObjectToDocument {
        private:
            View::MapDocumentPtr m_document;
        public:
            AddObjectToDocument(View::MapDocumentPtr document) :
            m_document(document) {}
            
            inline void operator()(Model::Object* object) {
                switch (object->type()) {
                    case Model::Object::OTEntity:
                        m_document->addEntity(static_cast<Model::Entity*>(object));
                        break;
                    case Model::Object::OTBrush:
                        m_document->addWorldBrush(static_cast<Model::Brush*>(object));
                        break;
                }
            }
        };
        
        class RemoveObjectFromDocument {
        private:
            View::MapDocumentPtr m_document;
        public:
            RemoveObjectFromDocument(View::MapDocumentPtr document) :
            m_document(document) {}
            
            inline void operator()(Model::Object* object) {
                switch (object->type()) {
                    case Model::Object::OTEntity:
                        m_document->removeEntity(static_cast<Model::Entity*>(object));
                        break;
                    case Model::Object::OTBrush:
                        m_document->removeWorldBrush(static_cast<Model::Brush*>(object));
                        break;
                }
            }
        };

        struct AddObject {
            Model::ObjectList& objects;
            
            AddObject(Model::ObjectList& i_objects) :
            objects(i_objects) {}
            
            inline void operator()(Model::Object* object) {
                switch (object->type()) {
                    case Model::Object::OTEntity: {
                        Model::Entity* entity = static_cast<Model::Entity*>(object);
                        if (entity->worldspawn()) {
                            const Model::BrushList& worldBrushes = entity->brushes();
                            objects.insert(objects.end(), worldBrushes.begin(), worldBrushes.end());
                            delete entity; // we take ownership of all objects, and since we discard worldspawn, we must delete it
                        } else {
                            objects.push_back(object);
                        }
                        break;
                    }
                    case Model::Object::OTBrush:
                        objects.push_back(object);
                        break;
                }
            }
        };
        
        AddRemoveObjectsCommand::~AddRemoveObjectsCommand() {
            if (m_action == AAdd)
                VectorUtils::clearAndDelete(m_objectsToAdd);
            else if (m_action == ARemove)
                VectorUtils::clearAndDelete(m_objectsToRemove);
        }

        AddRemoveObjectsCommand::Ptr AddRemoveObjectsCommand::addObjects(View::MapDocumentPtr document, const Model::ObjectList& objects) {
            return AddRemoveObjectsCommand::Ptr(new AddRemoveObjectsCommand(document, AAdd, objects));
        }

        const Model::ObjectList& AddRemoveObjectsCommand::addedObjects() const {
            if (m_action == AAdd)
                return m_objectsToRemove;
            return m_objectsToAdd;
        }
        
        const Model::ObjectList& AddRemoveObjectsCommand::removedObjects() const {
            if (m_action == AAdd)
                return m_objectsToAdd;
            return m_objectsToRemove;
        }

        AddRemoveObjectsCommand::AddRemoveObjectsCommand(View::MapDocumentPtr document, const Action action, const Model::ObjectList& objects) :
        Command(Type, makeName(action, objects), true),
        m_document(document),
        m_action(action) {
            AddObject addObject(m_action == AAdd ? m_objectsToAdd : m_objectsToRemove);
            Model::each(objects.begin(), objects.end(), addObject, Model::MatchAll());
        }

        String AddRemoveObjectsCommand::makeName(const Action action, const Model::ObjectList& objects) {
            StringStream name;
            name << (action == AAdd ? "Add " : "Remove ");
            name << (objects.size() == 1 ? "object" : "objects");
            return name.str();
        }

        bool AddRemoveObjectsCommand::doPerformDo() {
            if (m_action == AAdd)
                addObjects(m_objectsToAdd);
            else
                removeObjects(m_objectsToRemove);
            std::swap(m_objectsToAdd, m_objectsToRemove);
            return true;
        }
        
        bool AddRemoveObjectsCommand::doPerformUndo() {
            if (m_action == AAdd)
                removeObjects(m_objectsToAdd);
            else
                addObjects(m_objectsToRemove);
            std::swap(m_objectsToAdd, m_objectsToRemove);
            return true;
        }

        void AddRemoveObjectsCommand::addObjects(const Model::ObjectList& objects) {
            AddObjectToDocument addObjectToDocument(m_document);
            Model::each(objects.begin(), objects.end(), addObjectToDocument, Model::MatchAll());
        }
        
        void AddRemoveObjectsCommand::removeObjects(const Model::ObjectList& objects) {
            RemoveObjectFromDocument removeObjectFromDocument(m_document);
            Model::each(objects.begin(), objects.end(), removeObjectFromDocument, Model::MatchAll());
        }
    }
}
