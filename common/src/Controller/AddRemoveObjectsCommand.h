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

#ifndef __TrenchBroom__AddRemoveObjectsCommand__
#define __TrenchBroom__AddRemoveObjectsCommand__

#include "SharedPointer.h"
#include "Controller/DocumentCommand.h"
#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

#include <map>

namespace TrenchBroom {
    namespace Controller {
        class AddRemoveObjectsCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            typedef std::tr1::shared_ptr<AddRemoveObjectsCommand> Ptr;
        private:
            typedef enum {
                Action_Add,
                Action_Remove
            } Action;
            
            Model::ObjectLayerMap m_newLayers;
            Model::ObjectLayerMap m_oldLayers;
            
            Action m_action;
            Model::ObjectParentList m_objectsToAdd;
            Model::ObjectParentList m_objectsToRemove;
        public:
            ~AddRemoveObjectsCommand();
            
            static AddRemoveObjectsCommand::Ptr addObjects(View::MapDocumentWPtr document, const Model::ObjectParentList& objects, Model::Layer* layer);
            static AddRemoveObjectsCommand::Ptr addObjects(View::MapDocumentWPtr document, const Model::ObjectParentList& objects, const Model::ObjectLayerMap& layers);
            static AddRemoveObjectsCommand::Ptr removeObjects(View::MapDocumentWPtr document, const Model::ObjectParentList& objects);
        private:
            AddRemoveObjectsCommand(View::MapDocumentWPtr document, const Action action, const Model::ObjectParentList& objects, const Model::ObjectLayerMap& layers);
            Model::ObjectParentList addEmptyBrushEntities(const Model::ObjectParentList& objects) const;
            static String makeName(const Action action, const Model::ObjectParentList& objects);

            bool doPerformDo();
            bool doPerformUndo();

            bool doIsRepeatable(View::MapDocumentSPtr document) const;

            bool doCollateWith(Command::Ptr command);
            
            void addObjects(const Model::ObjectParentList& objects);
            void removeObjects(const Model::ObjectParentList& objects);
            void setLayers(const Model::ObjectParentList& objects);
            void restoreLayers(const Model::ObjectParentList& objects);
        };
    }
}

#endif /* defined(__TrenchBroom__AddRemoveObjectsCommand__) */
