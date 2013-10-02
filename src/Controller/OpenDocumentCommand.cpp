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

#include "OpenDocumentCommand.h"

#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        const Command::CommandType OpenDocumentCommand::Type = Command::freeType();
        
        OpenDocumentCommand::OpenDocumentCommand(View::MapDocumentPtr document, const BBox3& worldBounds, Model::GamePtr game, const IO::Path& path) :
        Command(Type, "Open Document", false, false),
        m_document(document),
        m_worldBounds(worldBounds),
        m_game(game),
        m_path(path) {}
        
        Model::Map* OpenDocumentCommand::map() const {
            return m_document->map();
        }

        bool OpenDocumentCommand::doPerformDo() {
            m_document->openDocument(m_worldBounds, m_game, m_path);
            m_document->documentWasLoadedNotifier();
            return true;
        }
    }
}
