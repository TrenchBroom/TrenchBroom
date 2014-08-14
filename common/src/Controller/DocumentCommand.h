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

#ifndef __TrenchBroom__DocumentCommand__
#define __TrenchBroom__DocumentCommand__

#include "SharedPointer.h"
#include "Controller/Command.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Controller {
        class DocumentCommand : public Command {
        public:
            typedef std::tr1::shared_ptr<DocumentCommand> Ptr;
        private:
            View::MapDocumentWPtr m_document;
        public:
            virtual ~DocumentCommand();
        protected:
            DocumentCommand(CommandType type, const String& name, bool undoable, View::MapDocumentWPtr document);
            
            View::MapDocumentSPtr lockDocument();
        };
    }
}

#endif /* defined(__TrenchBroom__DocumentCommand__) */
