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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "EntityAttributeSelectedCommand.h"

wxDEFINE_EVENT(ENTITY_ATTRIBUTE_SELECTED_EVENT, TrenchBroom::View::EntityAttributeSelectedCommand);

namespace TrenchBroom {
    namespace View {
        wxIMPLEMENT_DYNAMIC_CLASS(EntityAttributeSelectedCommand, wxNotifyEvent)
        
        EntityAttributeSelectedCommand::EntityAttributeSelectedCommand() :
        wxNotifyEvent(ENTITY_ATTRIBUTE_SELECTED_EVENT, wxID_ANY) {}
        
        const Model::AttributeName& EntityAttributeSelectedCommand::name() const {
            return m_name;
        }
        
        void EntityAttributeSelectedCommand::setName(const Model::AttributeName& name) {
            m_name = name;
        }

        wxEvent* EntityAttributeSelectedCommand::Clone() const {
            return new EntityAttributeSelectedCommand(*this);
        }
        
    }
}
