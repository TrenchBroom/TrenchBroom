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

#include "SetModCommand.h"

#include "Model/Entity.h"

namespace TrenchBroom {
    namespace Controller {
        bool SetModCommand::performDo() {
            m_oldMod = "";
            
            Model::Entity* worldspawn = document().worldspawn(false);
            if (worldspawn == NULL) {
                m_createdWorldspawn = true;
                worldspawn = document().worldspawn(true);
            } else {
                m_createdWorldspawn = false;
                const Model::PropertyValue* modProperty = worldspawn->propertyForKey(Model::Entity::ModKey);
                if (modProperty != NULL)
                    m_oldMod = *modProperty;
            }
            
            if (m_newMod == m_oldMod)
                return false;
            
            worldspawn->setProperty(Model::Entity::ModKey, m_newMod);
            document().invalidateSearchPaths();
            return true;
        }
        
        bool SetModCommand::performUndo() {
            document().setEntityDefinitionFile(m_oldMod);
            if (m_createdWorldspawn) {
                Model::Entity* worldspawn = document().worldspawn(false);
                assert(worldspawn != NULL);
                
                document().removeEntity(*worldspawn);
            }
            document().invalidateSearchPaths();
            return true;
        }
        
        SetModCommand::SetModCommand(Model::MapDocument& document, const String& mod) :
        DocumentCommand(SetMod, document, true, wxT("Set Mod"), true),
        m_newMod(mod),
        m_createdWorldspawn(false) {
        }
        
        SetModCommand* SetModCommand::setMod(Model::MapDocument& document, const String& mod) {
            return new SetModCommand(document, mod);
        }
    }
}
