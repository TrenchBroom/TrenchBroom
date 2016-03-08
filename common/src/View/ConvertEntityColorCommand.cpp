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

#include "ConvertEntityColorCommand.h"

#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType ConvertEntityColorCommand::Type = Command::freeType();
        
        ConvertEntityColorCommand::Ptr ConvertEntityColorCommand::convert(const Model::AttributeName& attributeName, const Assets::ColorRange::Type colorRange) {
            return Ptr(new ConvertEntityColorCommand(attributeName, colorRange));
        }

        ConvertEntityColorCommand::ConvertEntityColorCommand(const Model::AttributeName& attributeName, Assets::ColorRange::Type colorRange) :
        DocumentCommand(Type, "Convert Color"),
        m_attributeName(attributeName),
        m_colorRange(colorRange) {}
        
        bool ConvertEntityColorCommand::doPerformDo(MapDocumentCommandFacade* document) {
            m_snapshots = document->performConvertColorRange(m_attributeName, m_colorRange);
            return true;
        }
        
        bool ConvertEntityColorCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->restoreAttributes(m_snapshots);
            return true;
        }
        
        bool ConvertEntityColorCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return false;
        }
        
        bool ConvertEntityColorCommand::doCollateWith(UndoableCommand::Ptr command) {
            return false;
        }
    }
}
