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

#include "AddRemoveObjectsCommand.h"

#include "CollectionUtils.h"
#include "Notifier.h"
#include "Model/Entity.h"
#include "Model/Object.h"
#include "Model/ModelUtils.h"
#include "View/MapDocument.h"

#include <map>

namespace TrenchBroom {
    namespace Controller {
        const Command::CommandType AddRemoveObjectsCommand::Type = Command::freeType();

        AddRemoveObjectsCommand::~AddRemoveObjectsCommand() {
            VectorUtils::clearAndDelete(m_removedObjects);
        }

        AddRemoveObjectsCommand::Ptr AddRemoveObjectsCommand::addObjects(View::MapDocumentWPtr document, const Model::ObjectParentList& objects) {
            return AddRemoveObjectsCommand::Ptr(new AddRemoveObjectsCommand(document, Action_Add, objects));
        }

        AddRemoveObjectsCommand::Ptr AddRemoveObjectsCommand::removeObjects(View::MapDocumentWPtr document, const Model::ObjectParentList& objects) {
            return AddRemoveObjectsCommand::Ptr(new AddRemoveObjectsCommand(document, Action_Remove, objects));
        }

        const Model::ObjectList& AddRemoveObjectsCommand::addedObjects() const {
            return m_addedObjects;
        }
        
        const Model::ObjectList& AddRemoveObjectsCommand::removedObjects() const {
            return m_removedObjects;
        }

        AddRemoveObjectsCommand::AddRemoveObjectsCommand(View::MapDocumentWPtr document, const Action action, const Model::ObjectParentList& objects) :
        DocumentCommand(Type, makeName(action, objects), true, document),
        m_action(action) {
            if (action == Action_Add)
                m_objectsToAdd = objects;
            else
                m_objectsToRemove = addEmptyBrushEntities(objects);
        }
        
        Model::ObjectParentList AddRemoveObjectsCommand::addEmptyBrushEntities(const Model::ObjectParentList& objects) const {

            /*
             This method makes sure that brush entities which will have all their brushes removed get removed themselves instead of remaining in the map, but empty.
             */
            
            // First we build a map of each parent object to its children.
            Model::ObjectParentList result;
            Model::ObjectChildrenMap map;
            Model::ObjectParentList::const_iterator it, end;
            for (it = objects.begin(), end = objects.end(); it != end; ++it) {
                Model::Object* parent = it->parent;
                Model::Object* object = it->object;
                map[parent].push_back(object);
            }
            
            // now we iterate the map, checking for every non-null parent whether all its children are removed
            // if that is the case, we add that parent to the list of objects to be removed instead
            // of all its children, which then get removed automatically
            Model::ObjectChildrenMap::const_iterator mIt, mEnd;
            for (mIt = map.begin(), mEnd = map.end(); mIt != mEnd; ++mIt) {
                Model::Object* parent = mIt->first;
                const Model::ObjectList& children = mIt->second;
                
                if (parent != NULL) {
                    if (parent->type() == Model::Object::Type_Entity) {
                        Model::Entity* entity = static_cast<Model::Entity*>(parent);
                        if (entity->brushes().size() == children.size() && !entity->worldspawn()) {
                            result.push_back(Model::ObjectParentPair(parent, NULL));
                        } else {
                            Model::ObjectList::const_iterator cIt, cEnd;
                            for (cIt = children.begin(), cEnd = children.end(); cIt != cEnd; ++cIt) {
                                Model::Object* child = *cIt;
                                result.push_back(Model::ObjectParentPair(child, parent));
                            }
                        }
                    }
                } else {
                    Model::ObjectList::const_iterator cIt, cEnd;
                    for (cIt = children.begin(), cEnd = children.end(); cIt != cEnd; ++cIt) {
                        Model::Object* child = *cIt;
                        result.push_back(Model::ObjectParentPair(child, parent));
                    }
                }
            }

            return result;
        }

        String AddRemoveObjectsCommand::makeName(const Action action, const Model::ObjectParentList& objects) {
            StringStream name;
            name << (action == Action_Add ? "Add " : "Remove ");
            name << (objects.size() == 1 ? "object" : "objects");
            return name.str();
        }

        bool AddRemoveObjectsCommand::doPerformDo() {
            m_addedObjects.clear();
            m_removedObjects.clear();
            
            if (m_action == Action_Add)
                addObjects(m_objectsToAdd);
            else
                removeObjects(m_objectsToRemove);
            
            using std::swap;
            swap(m_objectsToAdd, m_objectsToRemove);
            return true;
        }
        
        bool AddRemoveObjectsCommand::doPerformUndo() {
            m_addedObjects.clear();
            m_removedObjects.clear();
            
            if (m_action == Action_Add)
                removeObjects(m_objectsToRemove);
            else
                addObjects(m_objectsToAdd);

            using std::swap;
            swap(m_objectsToAdd, m_objectsToRemove);
            return true;
        }

        bool AddRemoveObjectsCommand::doIsRepeatable(View::MapDocumentSPtr document) const {
            return false;
        }

        bool AddRemoveObjectsCommand::doCollateWith(Command::Ptr command) {
            return false;
        }

        void AddRemoveObjectsCommand::addObjects(const Model::ObjectParentList& objects) {
            View::MapDocumentSPtr document = lockDocument();

            Model::ObjectList parents, children;
            makeParentChildLists(objects, parents, children);

            document->objectsWillChangeNotifier(parents);
            document->addObjects(objects);
            document->objectsWereAddedNotifier(children);
            document->objectsDidChangeNotifier(parents);
            m_addedObjects = children;
        }
        
        void AddRemoveObjectsCommand::removeObjects(const Model::ObjectParentList& objects) {
            View::MapDocumentSPtr document = lockDocument();

            Model::ObjectList parents, children;
            makeParentChildLists(objects, parents, children);
            document->objectsWillChangeNotifier(parents);
            document->objectsWillBeRemovedNotifier(children);
            document->removeObjects(children);
            document->objectsWereRemovedNotifier(objects);
            document->objectsDidChangeNotifier(parents);
            m_removedObjects = children;
        }
    }
}
