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

#ifndef __TrenchBroom__OpenDocumentCommand__
#define __TrenchBroom__OpenDocumentCommand__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "SharedPointer.h"
#include "Controller/Command.h"
#include "IO/Path.h"
#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Controller {
        class OpenDocumentCommand : public Command {
        public:
            static const CommandType Type;
            typedef std::tr1::shared_ptr<OpenDocumentCommand> Ptr;
        private:
            View::MapDocumentPtr m_document;
            BBox3 m_worldBounds;
            Model::GamePtr m_game;
            IO::Path m_path;
        public:
            OpenDocumentCommand(View::MapDocumentPtr document, const BBox3& worldBounds, Model::GamePtr game, const IO::Path& path);
            
            Model::MapPtr map() const;
        private:
            bool doPerformDo();
        };
    }
}

#endif /* defined(__TrenchBroom__OpenDocumentCommand__) */
