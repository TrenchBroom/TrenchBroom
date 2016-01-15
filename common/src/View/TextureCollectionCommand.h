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

#ifndef TrenchBroom_TextureCollectionCommand
#define TrenchBroom_TextureCollectionCommand

#include "SharedPointer.h"
#include "View/DocumentCommand.h"
#include "StringUtils.h"

namespace TrenchBroom {
    namespace View {
        class TextureCollectionCommand : public DocumentCommand {
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
            
            Action m_action;
            StringList m_collectionNames;
        public:
            static Ptr add(const String& collectionName);
            static Ptr remove(const StringList& collectionNames);
            static Ptr moveUp(const String& collectionName);
            static Ptr moveDown(const String& collectionName);
        private:
            TextureCollectionCommand(const String& name, Action action, const StringList& collectionNames);

            bool doPerformDo(MapDocumentCommandFacade* document);
            bool doPerformUndo(MapDocumentCommandFacade* document);
            
            bool doIsRepeatable(MapDocumentCommandFacade* document) const;
            bool doCollateWith(UndoableCommand::Ptr command);
        };
    }
}

#endif /* defined(TrenchBroom_TextureCollectionCommand) */
