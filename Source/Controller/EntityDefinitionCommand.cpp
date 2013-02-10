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

#include "EntityDefinitionCommand.h"

#include "Model/Entity.h"
#include "Model/MapDocument.h"
#include "Utility/String.h"

namespace TrenchBroom {
    namespace Controller {
        bool EntityDefinitionCommand::performDo() {
            m_oldEntityDefinitionFile = "";

            Model::Entity* worldspawn = document().worldspawn(false);
            if (worldspawn == NULL) {
                m_createdWorldspawn = true;
                worldspawn = document().worldspawn(true);
            } else {
                m_createdWorldspawn = false;
                const Model::PropertyValue* defProperty = worldspawn->propertyForKey(Model::Entity::DefKey);
                if (defProperty != NULL)
                    m_oldEntityDefinitionFile = *defProperty;
            }
            
            if (m_newEntityDefinitionFile == m_oldEntityDefinitionFile)
                return false;

            document().setEntityDefinitionFile(m_newEntityDefinitionFile);
            return true;
        }
        
        bool EntityDefinitionCommand::performUndo() {
            document().setEntityDefinitionFile(m_oldEntityDefinitionFile);
            if (m_createdWorldspawn) {
                Model::Entity* worldspawn = document().worldspawn(false);
                assert(worldspawn != NULL);
                
                document().removeEntity(*worldspawn);
            }
            
            return true;
        }
        
        EntityDefinitionCommand::EntityDefinitionCommand(Model::MapDocument& document, const String& entityDefinitionFile) :
        DocumentCommand(SetEntityDefinitionFile, document, true, wxT("Set Entity Definition File"), true),
        m_createdWorldspawn(false),
        m_newEntityDefinitionFile(entityDefinitionFile) {}

        EntityDefinitionCommand* EntityDefinitionCommand::setEntityDefinitionFile(Model::MapDocument& document, const String& entityDefinitionFile) {
            return new EntityDefinitionCommand(document, entityDefinitionFile);
        }
    }
}
