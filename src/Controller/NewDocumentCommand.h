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

#ifndef __TrenchBroom__NewDocumentCommand__
#define __TrenchBroom__NewDocumentCommand__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "SharedPointer.h"
#include "Controller/Command.h"
#include "Model/Game.h"
#include "Model/Map.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        class NewDocumentCommand : public Command {
        public:
            static const CommandType Type;
            typedef std::tr1::shared_ptr<NewDocumentCommand> Ptr;
        private:
            View::MapDocument::Ptr m_document;
            BBox3 m_worldBounds;
            Model::Game::Ptr m_game;
        public:
            NewDocumentCommand(View::MapDocument::Ptr document, const BBox3& worldBounds, Model::Game::Ptr game);
            
            Model::Map::Ptr map() const;
        private:
            bool doPerformDo();
        };
    }
}

#endif /* defined(__TrenchBroom__NewDocumentCommand__) */
