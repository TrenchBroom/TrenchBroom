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

#include "Command.h"
#include "Exceptions.h"

namespace TrenchBroom {
    namespace Controller {
        Command::CommandType Command::freeType() {
            static CommandType type = 1;
            return type++;
        }

        Command::Command(const CommandType type, const String& name, const bool undoable, const bool modifiesDocument) :
        m_type(type),
        m_state(Default),
        m_name(name),
        m_undoable(undoable),
        m_modifiesDocument(modifiesDocument) {}
        
        Command::CommandType Command::type() const {
            return m_type;
        }
        
        Command::CommandState Command::state() const {
            return m_state;
        }

        const String& Command::name() const {
            return m_name;
        }
        
        bool Command::undoable() const {
            return m_undoable;
        }
        
        bool Command::performDo() {
            m_state = Doing;
            if (doPerformDo()) {
                m_state = Done;
                return true;
            } else {
                m_state = Default;
                return false;
            }
        }
        
        bool Command::performUndo() {
            if (!undoable())
                throw CommandProcessorException("Cannot undo one-shot command");
            m_state = Undoing;
            if (doPerformUndo()) {
                m_state = Default;
                return true;
            } else {
                m_state = Done;
                return false;
            }
        }
        
        bool Command::modifiesDocument() const {
            return m_modifiesDocument;
        }

        bool Command::doPerformUndo() {
            throw CommandProcessorException("Undo not implemented");
        }
    }
}
