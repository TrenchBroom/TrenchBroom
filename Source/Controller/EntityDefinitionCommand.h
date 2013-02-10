/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#ifndef __TrenchBroom__EntityDefinitionCommand__
#define __TrenchBroom__EntityDefinitionCommand__

#include "Controller/Command.h"

namespace TrenchBroom {
    namespace Controller {
        class EntityDefinitionCommand : public DocumentCommand {
        protected:
            bool m_createdWorldspawn;
            String m_oldEntityDefinitionFile;
            String m_newEntityDefinitionFile;
            
            bool performDo();
            bool performUndo();

            EntityDefinitionCommand(Model::MapDocument& document, const String& entityDefinitionFile);
        public:
            static EntityDefinitionCommand* setEntityDefinitionFile(Model::MapDocument& document, const String& entityDefinitionFile);
        };
    }
}

#endif /* defined(__TrenchBroom__EntityDefinitionCommand__) */
