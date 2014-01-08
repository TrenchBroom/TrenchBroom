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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "EntityPropertySelectedCommand.h"

DEFINE_EVENT_TYPE(EVT_ENTITY_PROPERTY_SELECTED_EVENT)

namespace TrenchBroom {
    namespace View {
        IMPLEMENT_DYNAMIC_CLASS(EntityPropertySelectedCommand, wxNotifyEvent)
        
        EntityPropertySelectedCommand::EntityPropertySelectedCommand() :
        wxNotifyEvent(EVT_ENTITY_PROPERTY_SELECTED_EVENT, wxID_ANY) {}
        
        const Model::PropertyKey& EntityPropertySelectedCommand::key() const {
            return m_key;
        }
        
        void EntityPropertySelectedCommand::setKey(const Model::PropertyKey& key) {
            m_key = key;
        }

        wxEvent* EntityPropertySelectedCommand::Clone() const {
            return new EntityPropertySelectedCommand(*this);
        }
        
    }
}
