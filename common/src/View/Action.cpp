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

#include "Action.h"
#include "ActionContext.h"

#include "PreferenceManager.h"

#include "Exceptions.h"

namespace TrenchBroom {
    namespace View {
        Action::Action() :
        m_id(wxID_NONE),
        m_name(""),
        m_modifiable(false) {}

        Action::Action(const String& name) :
        m_id(wxID_NONE),
        m_name(name),
        m_modifiable(false) {}

        Action::Action(const int id, const String& name, const bool modifiable) :
        m_id(id),
        m_name(name),
        m_modifiable(modifiable) {}

        int Action::id() const {
            return m_id;
        }
        
        const String& Action::name() const {
            return m_name;
        }

        bool Action::modifiable() const {
            return m_modifiable;
        }
    }
}
