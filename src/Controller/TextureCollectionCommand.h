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
                AAdd,
                ARemove,
                AMoveUp,
                AMoveDown
            } Action;
            
            
            View::MapDocumentPtr m_document;
            Action m_action;
            IO::Path::List m_paths;
            IO::Path::List m_previousCollections;
        public:
            static Ptr add(View::MapDocumentPtr document, const IO::Path& path);
            static Ptr remove(View::MapDocumentPtr document, const IO::Path::List& paths);
            static Ptr moveUp(View::MapDocumentPtr document, const IO::Path& path);
            static Ptr moveDown(View::MapDocumentPtr document, const IO::Path& path);
        private:
            TextureCollectionCommand(View::MapDocumentPtr document, const String& name, Action action, const IO::Path::List& paths);
            
            bool doPerformDo();
            bool doPerformUndo();
            
            bool addTextureCollections(const IO::Path::List& paths);
            bool removeTextureCollections(const IO::Path::List& paths);
            bool moveUp(const IO::Path& path);
            bool moveDown(const IO::Path& path);
        };
    }
}

#endif /* defined(__TrenchBroom__TextureCollectionCommand__) */
