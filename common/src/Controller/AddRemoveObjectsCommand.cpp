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
            m_addQuery.clearAndDelete();
        }

        AddRemoveObjectsCommand::Ptr AddRemoveObjectsCommand::addObjects(View::MapDocumentWPtr document, const Model::AddObjectsQuery& addQuery) {
            return Ptr(new AddRemoveObjectsCommand(document, addQuery));
        }
        
        AddRemoveObjectsCommand::Ptr AddRemoveObjectsCommand::removeObjects(View::MapDocumentWPtr document, const Model::RemoveObjectsQuery& removeQuery) {
            return Ptr(new AddRemoveObjectsCommand(document, removeQuery));
        }

        AddRemoveObjectsCommand::AddRemoveObjectsCommand(View::MapDocumentWPtr document, const Model::AddObjectsQuery& addQuery) :
        DocumentCommand(Type, StringUtils::safePlural(addQuery.objectCount(), "Add object", "Add objects"), true, document),
        m_action(Action_Add),
        m_addQuery(addQuery) {}
        
        AddRemoveObjectsCommand::AddRemoveObjectsCommand(View::MapDocumentWPtr document, const Model::RemoveObjectsQuery& removeQuery) :
        DocumentCommand(Type, StringUtils::safePlural(removeQuery.objectCount(), "Remove object", "Remove objects"), true, document),
        m_action(Action_Remove),
        m_removeQuery(removeQuery) {}

        bool AddRemoveObjectsCommand::doPerformDo() {
            if (m_action == Action_Add)
                addObjects();
            else
                removeObjects();
            return true;
        }
        
        bool AddRemoveObjectsCommand::doPerformUndo() {
            if (m_action == Action_Add)
                removeObjects();
            else
                addObjects();
            return true;
        }

        bool AddRemoveObjectsCommand::doIsRepeatable(View::MapDocumentSPtr document) const {
            return false;
        }

        bool AddRemoveObjectsCommand::doCollateWith(Command::Ptr command) {
            return false;
        }

        void AddRemoveObjectsCommand::addObjects() {
            View::MapDocumentSPtr document = lockDocument();
            if (!m_addQuery.parents().empty())
                document->objectsWillChangeNotifier(m_addQuery.parents());
            document->addEntities(m_addQuery.entities(), m_addQuery.layers());
            document->addBrushes(m_addQuery.brushes(), m_addQuery.layers());
            document->objectsWereAddedNotifier(m_addQuery.objects());
            if (!m_addQuery.parents().empty())
                document->objectsDidChangeNotifier(m_addQuery.parents());
            
            m_removeQuery = Model::RemoveObjectsQuery(m_addQuery);
            m_addQuery.clear();
        }
        
        void AddRemoveObjectsCommand::removeObjects() {
            // build the add query when the objects still have all required information, e.g. their layers
            m_addQuery = Model::AddObjectsQuery(m_removeQuery);

            View::MapDocumentSPtr document = lockDocument();
            if (!m_removeQuery.parents().empty())
                document->objectsWillChangeNotifier(m_removeQuery.parents());
            document->objectsWillBeRemovedNotifier(m_removeQuery.objects());
            document->removeBrushes(m_removeQuery.brushes());
            document->removeEntities(m_removeQuery.entities());
            document->objectsWereRemovedNotifier(m_removeQuery.objects());
            if (!m_removeQuery.parents().empty())
                document->objectsDidChangeNotifier(m_removeQuery.parents());
            
            m_removeQuery.clear();
        }
    }
}
