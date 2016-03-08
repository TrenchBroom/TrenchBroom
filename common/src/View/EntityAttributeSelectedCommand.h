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

#ifndef TrenchBroom_EntityAttributeSelectedCommand
#define TrenchBroom_EntityAttributeSelectedCommand

#include "Model/ModelTypes.h"

#include <wx/event.h>

namespace TrenchBroom {
    namespace View {
        class EntityAttributeSelectedCommand : public wxNotifyEvent {
        protected:
            Model::AttributeName m_name;
        public:
            EntityAttributeSelectedCommand();
            
            const Model::AttributeName& name() const;
            void setName(const Model::AttributeName& name);
            
            virtual wxEvent* Clone() const;
            
            DECLARE_DYNAMIC_CLASS(EntityAttributeSelectedCommand)
        };
    }
}

typedef void (wxEvtHandler::*EntityAttributeSelectedCommandFunction)(TrenchBroom::View::EntityAttributeSelectedCommand &);

wxDECLARE_EVENT(ENTITY_ATTRIBUTE_SELECTED_EVENT, TrenchBroom::View::EntityAttributeSelectedCommand);
#define EntityAttributeSelectedHandler(func) wxEVENT_HANDLER_CAST(EntityAttributeSelectedCommandFunction, func)

#endif /* defined(TrenchBroom_EntityAttributeSelectedCommand) */
