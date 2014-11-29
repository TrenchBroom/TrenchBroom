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

#include "View/DocumentCommand.h"
#include "StringUtils.h"

namespace TrenchBroom {
    namespace View {
        class TextureCollectionCommand : public DocumentCommand {
        public:
            static const CommandType Type;
        private:
            typedef enum {
                Action_Add,
                Action_Remove,
                Action_MoveUp,
                Action_MoveDown
            } Action;
            
            Action m_action;
            StringList m_collectionNames;
        public:
            static TextureCollectionCommand* add(const String& collectionName);
            static TextureCollectionCommand* remove(const StringList& collectionNames);
            static TextureCollectionCommand* moveUp(const String& collectionName);
            static TextureCollectionCommand* moveDown(const String& collectionName);
        private:
            TextureCollectionCommand(const String& name, Action action, const StringList& collectionNames);

            bool doPerformDo(MapDocumentCommandFacade* document);
            bool doPerformUndo(MapDocumentCommandFacade* document);
            
            bool doIsRepeatable(MapDocumentCommandFacade* document) const;
            bool doCollateWith(UndoableCommand* command);
        };
    }
}

#endif /* defined(__TrenchBroom__TextureCollectionCommand__) */
