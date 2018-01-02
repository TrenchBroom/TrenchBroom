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

#include "Command.h"
#include "Exceptions.h"

namespace TrenchBroom {
    namespace View {
        Command::CommandType Command::freeType() {
            static CommandType type = 1;
            return type++;
        }

        Command::Command(const CommandType type, const String& name) :
        m_type(type),
        m_state(CommandState_Default),
        m_name(name) {}
        
        Command::~Command() {}
        
        Command::CommandType Command::type() const {
            return m_type;
        }
        
        bool Command::isType(CommandType type) const {
            return m_type == type;
        }
        
        Command::CommandState Command::state() const {
            return m_state;
        }

        const String& Command::name() const {
            return m_name;
        }
        
        bool Command::performDo(MapDocumentCommandFacade* document) {
            m_state = CommandState_Doing;
            if (doPerformDo(document)) {
                m_state = CommandState_Done;
                return true;
            } else {
                m_state = CommandState_Default;
                return false;
            }
        }
    }
}
