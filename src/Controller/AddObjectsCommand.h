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

#ifndef __TrenchBroom__AddObjectsCommand__
#define __TrenchBroom__AddObjectsCommand__

#include "SharedPointer.h"
#include "Controller/Command.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Controller {
        class AddObjectsCommand : public Command {
        public:
            static const CommandType Type;
            typedef std::tr1::shared_ptr<AddObjectsCommand> Ptr;
        private:
            View::MapDocumentPtr m_document;
            
            Model::EntityList m_entitiesToAdd;
            Model::BrushList m_brushesToAdd;
            
            Model::EntityList m_addedEntities;
            Model::BrushList m_addedBrushes;
            bool m_hasAddedBrushes;
        public:
            static AddObjectsCommand::Ptr addObjects(View::MapDocumentPtr document, const Model::EntityList& entities, const Model::BrushList& brushes);
            static AddObjectsCommand::Ptr addEntities(View::MapDocumentPtr document, const Model::EntityList& entities);
            static AddObjectsCommand::Ptr addBrushes(View::MapDocumentPtr document, const Model::BrushList& brushes);
        private:
            AddObjectsCommand(View::MapDocumentPtr document, const Model::EntityList& entities, const Model::BrushList& brushes);

            bool doPerformDo();
            bool doPerformUndo();
        };
    }
}

#endif /* defined(__TrenchBroom__AddObjectsCommand__) */
