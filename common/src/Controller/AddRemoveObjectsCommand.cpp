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
            VectorUtils::deleteAll(Model::makeChildList(m_objectsToAdd));
        }

        AddRemoveObjectsCommand::Ptr AddRemoveObjectsCommand::addObjects(View::MapDocumentWPtr document, const Model::ObjectParentList& objects, Model::Layer* layer) {
            Model::ObjectLayerMap layers;
            Model::ObjectParentList::const_iterator it, end;
            for (it = objects.begin(), end = objects.end(); it != end; ++it) {
                Model::Object* object = it->object;
                layers[object] = layer;
            }
            return addObjects(document, objects, layers);
        }

        AddRemoveObjectsCommand::Ptr AddRemoveObjectsCommand::addObjects(View::MapDocumentWPtr document, const Model::ObjectParentList& objects, const Model::ObjectLayerMap& layers) {
            return Ptr(new AddRemoveObjectsCommand(document, Action_Add, objects, layers));
        }
        
        AddRemoveObjectsCommand::Ptr AddRemoveObjectsCommand::removeObjects(View::MapDocumentWPtr document, const Model::ObjectParentList& objects) {
            return Ptr(new AddRemoveObjectsCommand(document, Action_Remove, objects, Model::ObjectLayerMap()));
        }

        AddRemoveObjectsCommand::AddRemoveObjectsCommand(View::MapDocumentWPtr document, const Action action, const Model::ObjectParentList& objects, const Model::ObjectLayerMap& layers) :
        DocumentCommand(Type, makeName(action, objects), true, document),
        m_newLayers(layers),
        m_action(action) {
            if (action == Action_Add) {
                m_objectsToAdd = objects;
                m_oldLayers = collectLayers(m_objectsToAdd);
            } else {
                m_objectsToRemove = addEmptyBrushEntities(objects);
                m_oldLayers = collectLayers(m_objectsToRemove);
            }
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

        Model::ObjectLayerMap AddRemoveObjectsCommand::collectLayers(const Model::ObjectParentList& objects) const {
            Model::ObjectLayerMap result;
            Model::ObjectParentList::const_iterator it, end;
            for (it = objects.begin(), end = objects.end(); it != end; ++it)
                result[it->object] = it->object->layer();
            return result;
        }

        String AddRemoveObjectsCommand::makeName(const Action action, const Model::ObjectParentList& objects) {
            StringStream name;
            name << (action == Action_Add ? "Add " : "Remove ");
            name << (objects.size() == 1 ? "object" : "objects");
            return name.str();
        }

        bool AddRemoveObjectsCommand::doPerformDo() {
            if (m_action == Action_Add)
                addObjects(m_objectsToAdd, m_newLayers);
            else
                removeObjects(m_objectsToRemove, m_newLayers);
            
            using std::swap;
            swap(m_objectsToAdd, m_objectsToRemove);
            return true;
        }
        
        bool AddRemoveObjectsCommand::doPerformUndo() {
            if (m_action == Action_Add)
                removeObjects(m_objectsToRemove, m_oldLayers);
            else
                addObjects(m_objectsToAdd, m_oldLayers);

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

        void AddRemoveObjectsCommand::addObjects(const Model::ObjectParentList& objects, const Model::ObjectLayerMap& layers) {
            View::MapDocumentSPtr document = lockDocument();

            Model::ObjectList parents, children;
            makeParentChildLists(objects, parents, children);

            document->objectsWillChangeNotifier(parents);
            document->addObjects(objects);
            setLayers(layers);
            document->objectsWereAddedNotifier(children);
            document->objectsDidChangeNotifier(parents);
        }
        
        void AddRemoveObjectsCommand::removeObjects(const Model::ObjectParentList& objects, const Model::ObjectLayerMap& layers) {
            View::MapDocumentSPtr document = lockDocument();

            Model::ObjectList parents, children;
            makeParentChildLists(objects, parents, children);
            document->objectsWillChangeNotifier(parents);
            document->objectsWillBeRemovedNotifier(children);
            setLayers(layers);
            document->removeObjects(children);
            document->objectsWereRemovedNotifier(objects);
            document->objectsDidChangeNotifier(parents);
        }

        void AddRemoveObjectsCommand::setLayers(const Model::ObjectLayerMap& layers) {
            Model::ObjectLayerMap::const_iterator it, end;
            for (it = layers.begin(), end = layers.end(); it != end; ++it) {
                Model::Object* object = it->first;
                Model::Layer* layer = it->second;
                object->setLayer(layer);
            }
        }
    }
}
