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

#ifndef __TrenchBroom__AddObjectsCommand__
#define __TrenchBroom__AddObjectsCommand__

#include "Controller/Command.h"

#include "Model/BrushTypes.h"
#include "Model/EntityTypes.h"

namespace TrenchBroom {
    namespace Model {
        class MapDocument;
    }
    
    namespace Controller {
        class AddObjectsCommand : public DocumentCommand {
        private:
            Model::EntityList m_entities;
            Model::BrushList m_brushes;
            
            Model::EntityList m_addedEntities;
            Model::BrushList m_addedBrushes;
            
            bool m_hasAddedBrushes;
            
            bool performDo();
            bool performUndo();
            
            AddObjectsCommand(Model::MapDocument& document, const wxString& name, const Model::EntityList& entities, const Model::BrushList& brushes);
        public:
            static AddObjectsCommand* addObjects(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes);
            static AddObjectsCommand* addEntity(Model::MapDocument& document, Model::Entity& entity);
            static AddObjectsCommand* addBrush(Model::MapDocument& document, Model::Brush& brush);

            ~AddObjectsCommand();
            
            inline const Model::EntityList& addedEntities() const {
                return m_entities;
            }

            inline bool hasAddedBrushes() const {
                return m_hasAddedBrushes;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__AddObjectsCommand__) */
