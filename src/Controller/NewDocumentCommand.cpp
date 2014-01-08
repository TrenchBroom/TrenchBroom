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

#include "NewDocumentCommand.h"

#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        const Command::CommandType NewDocumentCommand::Type = Command::freeType();

        NewDocumentCommand::NewDocumentCommand(View::MapDocumentWPtr document, const BBox3& worldBounds, Model::GamePtr game) :
        Command(Type, "New Document", false, false),
        m_document(document),
        m_worldBounds(worldBounds),
        m_game(game) {}

        Model::Map* NewDocumentCommand::map() const {
            View::MapDocumentSPtr document = lock(m_document);
            return document->map();
        }

        bool NewDocumentCommand::doPerformDo() {
            View::MapDocumentSPtr document = lock(m_document);
            document->newDocument(m_worldBounds, m_game);
            document->documentWasNewedNotifier();
            return true;
        }
    }
}
