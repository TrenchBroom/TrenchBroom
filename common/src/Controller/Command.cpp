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
        m_state(CommandState_Default),
        m_name(name),
        m_undoable(undoable),
        m_modifiesDocument(modifiesDocument) {}
        
        Command::~Command() {}
        
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
            m_state = CommandState_Doing;
            if (doPerformDo()) {
                m_state = CommandState_Done;
                return true;
            } else {
                m_state = CommandState_Default;
                return false;
            }
        }
        
        bool Command::performUndo() {
            if (!undoable())
                throw CommandProcessorException("Cannot undo one-shot command");
            m_state = CommandState_Undoing;
            if (doPerformUndo()) {
                m_state = CommandState_Default;
                return true;
            } else {
                m_state = CommandState_Done;
                return false;
            }
        }
        
        bool Command::modifiesDocument() const {
            return m_modifiesDocument;
        }

        bool Command::isRepeatable() const {
            return doIsRepeatable();
        }
        
        Command::Ptr Command::repeat(View::MapDocumentSPtr document) const {
            return Ptr(doRepeat(document));
        }

        bool Command::collateWith(Ptr command) {
            assert(command.get() != this);
            if (command->type() != m_type)
                return false;
            return doCollateWith(command);
        }
        
        bool Command::doPerformUndo() {
            throw CommandProcessorException("Undo not implemented");
        }
        
        Command* Command::doRepeat(View::MapDocumentSPtr document) const {
            throw CommandProcessorException("Command is not repeatable");
        }
    }
}
