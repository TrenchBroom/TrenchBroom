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

#include "EntityDragTargetTool.h"

#include "Model/EntityDefinition.h"
#include "Model/EntityDefinitionManager.h"
#include "Model/MapDocument.h"
#include "View/DocumentViewHolder.h"
#include "Utility/String.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        bool EntityDragTargetTool::handleDragEnter(InputEvent& event, const String& payload) {
            StringList parts = Utility::split(payload, ':');
            if (parts.size() != 2)
                return NULL;
            if (parts[0] != "entity")
                return NULL;
            
            Model::MapDocument& document = documentViewHolder().document();
            Model::EntityDefinitionManager& definitionManager = document.definitionManager();
            m_definition = definitionManager.definition(parts[1]);
            return m_definition != NULL;
        }
        
        void EntityDragTargetTool::handleDragMove(InputEvent& event) {
            assert(m_definition != NULL);
        }
        
        void EntityDragTargetTool::handleDrop(InputEvent& event) {
            assert(m_definition != NULL);
        }
        
        void EntityDragTargetTool::handleDragLeave() {
            assert(m_definition != NULL);
        }
    }
}