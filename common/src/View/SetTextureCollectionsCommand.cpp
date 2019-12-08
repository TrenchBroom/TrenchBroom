/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "SetTextureCollectionsCommand.h"
#include "View/MapDocumentCommandFacade.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        const Command::CommandType SetTextureCollectionsCommand::Type = Command::freeType();

        SetTextureCollectionsCommand::Ptr SetTextureCollectionsCommand::set(const std::vector<IO::Path>& paths) {
            return Ptr(new SetTextureCollectionsCommand(paths));
        }

        SetTextureCollectionsCommand::SetTextureCollectionsCommand(const std::vector<IO::Path>& paths) :
        DocumentCommand(Type, "Set Texture Collections"),
        m_paths(paths) {}

        bool SetTextureCollectionsCommand::doPerformDo(MapDocumentCommandFacade* document) {
            m_oldPaths = document->enabledTextureCollections();
            document->performSetTextureCollections(m_paths);
            return true;
        }

        bool SetTextureCollectionsCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->performSetTextureCollections(m_oldPaths);
            m_oldPaths.clear();
            return true;
        }

        bool SetTextureCollectionsCommand::doIsRepeatable(MapDocumentCommandFacade*) const {
            return false;
        }

        bool SetTextureCollectionsCommand::doCollateWith(UndoableCommand::Ptr) {
            return false;
        }
    }
}
