/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#ifndef __TrenchBroom__CreateEntityCommand__
#define __TrenchBroom__CreateEntityCommand__

#include "Controller/Command.h"
#include "Model/EntityTypes.h"

namespace TrenchBroom {
    namespace Model {
        class Entity;
    }
    
    namespace Controller {
        class CreateEntityCommand : public DocumentCommand {
        protected:
            Model::Properties m_properties;
            Model::Entity* m_entity;
            
            CreateEntityCommand(Model::MapDocument& document, const Model::Properties& properties);
            ~CreateEntityCommand();
            
            bool performDo();
            bool performUndo();
        public:
            static CreateEntityCommand* createFromTemplate(Model::MapDocument& document, const Model::Entity& entityTemplate);
            
            inline Model::Entity* entity() const {
                return m_entity;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__CreateEntityCommand__) */
