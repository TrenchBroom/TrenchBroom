/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "SetGameEngineParameterSpecsCommand.h"

#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType SetGameEngineParameterSpecsCommand::Type = Command::freeType();

        SetGameEngineParameterSpecsCommand::Ptr SetGameEngineParameterSpecsCommand::set(const ::StringMap& specs) {
            return Ptr(new SetGameEngineParameterSpecsCommand("Set engine parameters", specs));
        }

        SetGameEngineParameterSpecsCommand::SetGameEngineParameterSpecsCommand(const String& name, const ::StringMap& specs) :
        Command(Type, name),
        m_specs(specs) {}
        
        bool SetGameEngineParameterSpecsCommand::doPerformDo(MapDocumentCommandFacade* document) {
            document->performSetGameEngineParameterSpecs(m_specs);
            m_specs.clear();
            document->incModificationCount();
            return true;
        }
    }
}
