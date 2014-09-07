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
#include "Model/AddObjectsQuery.h"
#include "Model/ModelTypes.h"
#include "Model/RemoveObjectsQuery.h"
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
            
            Action m_action;
            Model::AddObjectsQuery m_addQuery;
            Model::RemoveObjectsQuery m_removeQuery;
        public:
            ~AddRemoveObjectsCommand();
            
            static Ptr addObjects(View::MapDocumentWPtr document, const Model::AddObjectsQuery& addQuery);
            static Ptr removeObjects(View::MapDocumentWPtr document, const Model::RemoveObjectsQuery& removeQuery);
        private:
            AddRemoveObjectsCommand(View::MapDocumentWPtr document, const Model::AddObjectsQuery& addQuery);
            AddRemoveObjectsCommand(View::MapDocumentWPtr document, const Model::RemoveObjectsQuery& removeQuery);
            
            static String makeName(const Action action, const Model::ObjectParentList& objects);

            bool doPerformDo();
            bool doPerformUndo();

            bool doIsRepeatable(View::MapDocumentSPtr document) const;

            bool doCollateWith(Command::Ptr command);

            void addObjects();
            void removeObjects();
        };
    }
}

#endif /* defined(__TrenchBroom__AddRemoveObjectsCommand__) */
