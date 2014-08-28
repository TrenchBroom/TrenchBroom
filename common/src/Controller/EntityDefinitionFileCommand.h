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

#ifndef __TrenchBroom__EntityDefinitionFileCommand__
#define __TrenchBroom__EntityDefinitionFileCommand__

#include "SharedPointer.h"
#include "StringUtils.h"
#include "Controller/DocumentCommand.h"
#include "Model/EntityDefinitionFileSpec.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Controller {
        class EntityDefinitionFileCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            typedef std::tr1::shared_ptr<EntityDefinitionFileCommand> Ptr;
        private:
            Model::EntityDefinitionFileSpec m_newSpec;
            Model::EntityDefinitionFileSpec m_oldSpec;
        public:
            static Ptr setEntityDefinitionFileSpec(View::MapDocumentWPtr document, const Model::EntityDefinitionFileSpec& spec);
            static Ptr reloadEntityDefinitionFile(View::MapDocumentWPtr document);
        private:
            EntityDefinitionFileCommand(View::MapDocumentWPtr document, const Model::EntityDefinitionFileSpec& spec);
            
            bool doPerformDo();
            bool doPerformUndo();

            bool doIsRepeatable(View::MapDocumentSPtr document) const;

            bool doCollateWith(Command::Ptr command);
        };
    }
}

#endif /* defined(__TrenchBroom__EntityDefinitionFileCommand__) */
