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

#ifndef __TrenchBroom__TextureCollectionCommand__
#define __TrenchBroom__TextureCollectionCommand__

#include "SharedPointer.h"
#include "StringUtils.h"
#include "Controller/Command.h"
#include "IO/Path.h"
#include "View/ViewTypes.h"

#include <vector>

namespace TrenchBroom {
    namespace Controller {
        class TextureCollectionCommand : public Command {
        public:
            static const CommandType Type;
            typedef std::tr1::shared_ptr<TextureCollectionCommand> Ptr;
        private:
            typedef enum {
                Action_Add,
                Action_Remove,
                Action_MoveUp,
                Action_MoveDown
            } Action;
            
            
            View::MapDocumentWPtr m_document;
            Action m_action;
            StringList m_names;
        public:
            static Ptr add(View::MapDocumentWPtr document, const String& name);
            static Ptr remove(View::MapDocumentWPtr document, const StringList& names);
            static Ptr moveUp(View::MapDocumentWPtr document, const String& name);
            static Ptr moveDown(View::MapDocumentWPtr document, const String& name);
        private:
            TextureCollectionCommand(View::MapDocumentWPtr document, const String& name, Action action, const StringList& names);
            
            bool doPerformDo();
            bool doPerformUndo();
        };
    }
}

#endif /* defined(__TrenchBroom__TextureCollectionCommand__) */
