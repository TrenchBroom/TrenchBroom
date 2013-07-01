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
        Command::Command(const String& name, const bool undoable) :
        m_name(name),
        m_undoable(undoable) {}
        
        bool Command::undoable() const {
            return m_undoable;
        }

        const String& Command::name() const {
            return m_name;
        }
        
        bool Command::performDo() {
            return doPerformDo();
        }
        
        bool Command::performUndo() {
            return doPerformUndo();
        }
    }
}
