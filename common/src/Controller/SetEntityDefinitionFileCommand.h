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

#ifndef __TrenchBroom__SetEntityDefinitionFileCommand__
#define __TrenchBroom__SetEntityDefinitionFileCommand__

#include "SharedPointer.h"
#include "StringUtils.h"
#include "Controller/Command.h"
#include "Model/EntityDefinitionFileSpec.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Controller {
        class SetEntityDefinitionFileCommand : public Command {
        public:
            static const CommandType Type;
            typedef std::tr1::shared_ptr<SetEntityDefinitionFileCommand> Ptr;
        private:
            View::MapDocumentWPtr m_document;
            Model::EntityDefinitionFileSpec m_newSpec;
            Model::EntityDefinitionFileSpec m_oldSpec;
        public:
            static Ptr setEntityDefinitionFileSpec(View::MapDocumentWPtr document, const Model::EntityDefinitionFileSpec& spec);
        private:
            SetEntityDefinitionFileCommand(View::MapDocumentWPtr document, const Model::EntityDefinitionFileSpec& spec);
            
            bool doPerformDo();
            bool doPerformUndo();
            bool doCollateWith(Command::Ptr command);
        };
    }
}

#endif /* defined(__TrenchBroom__SetEntityDefinitionFileCommand__) */
