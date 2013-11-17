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

#ifndef __TrenchBroom__SetModsCommand__
#define __TrenchBroom__SetModsCommand__

#include "SharedPointer.h"
#include "StringUtils.h"
#include "Controller/Command.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Controller {
        class SetModsCommand : public Command {
        public:
            static const CommandType Type;
            typedef std::tr1::shared_ptr<SetModsCommand> Ptr;
        private:
            View::MapDocumentPtr m_document;
            StringList m_newMods;
            StringList m_oldMods;
        public:
            static Ptr setMods(View::MapDocumentPtr document, const StringList& mods);
        private:
            SetModsCommand(View::MapDocumentPtr document, const StringList& mods);
            
            bool doPerformDo();
            bool doPerformUndo();
        };
    }
}

#endif /* defined(__TrenchBroom__SetModsCommand__) */
