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
#include "Exceptions.h"

namespace TrenchBroom {
    namespace Controller {
        Command::CommandType Command::freeType() {
            static CommandType type = 1;
            return type++;
        }

        Command::Command(const CommandType type, const String& name, const bool undoable) :
        m_type(type),
        m_name(name),
        m_undoable(undoable) {}
        
        Command::CommandType Command::type() const {
            return m_type;
        }
        
        const String& Command::name() const {
            return m_name;
        }
        
        bool Command::undoable() const {
            return m_undoable;
        }
        
        bool Command::performDo() {
            return doPerformDo();
        }
        
        bool Command::performUndo() {
            if (!undoable())
                throw CommandProcessorException("Cannot undo one-shot command");
            return doPerformUndo();
        }
        
        const Model::ObjectList Command::affectedObjects() const {
            return doAffectedObjects();
        }

        const Model::EntityList Command::affectedEntities() const {
            return doAffectedEntities();
        }
        
        const Model::BrushList Command::affectedBrushes() const {
            return doAffectedBrushes();
        }

        bool Command::doPerformUndo() {
            throw CommandProcessorException("Undo not implemented");
        }

        const Model::ObjectList Command::doAffectedObjects() const {
            return Model::EmptyObjectList;
        }

        const Model::EntityList Command::doAffectedEntities() const {
            return Model::EmptyEntityList;
        }
        
        const Model::BrushList Command::doAffectedBrushes() const {
            return Model::EmptyBrushList;
        }
    }
}
