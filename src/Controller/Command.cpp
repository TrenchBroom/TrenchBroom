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

#include "Command.h"

namespace TrenchBroom {
    namespace Controller {
        Command::Command(const String& name, const bool canRollback) :
        m_name(name),
        m_canRollback(canRollback) {}
        
        bool Command::canRollback() const {
            return m_canRollback;
        }

        const String& Command::name() const {
            return m_name;
        }
        
        bool Command::execute() {
            return doExecute();
        }
        
        bool Command::rollback() {
            return doRollback();
        }
    }
}
