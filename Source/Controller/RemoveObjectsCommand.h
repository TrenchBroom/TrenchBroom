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

#ifndef __TrenchBroom__RemoveObjectsCommand__
#define __TrenchBroom__RemoveObjectsCommand__

#include "Controller/Command.h"
#include "Controller/ObjectsCommand.h"
#include "Model/EntityTypes.h"
#include "Model/BrushTypes.h"

namespace TrenchBroom {
    namespace Controller {
        class RemoveObjectsCommand : public DocumentCommand, ObjectsCommand {
        protected:
            Model::EntityList m_entities;
            Model::BrushList m_brushes;
            
            Model::EntityList m_removedEntities;
            Model::BrushList m_removedBrushes;
            Model::BrushParentMap m_removedBrushParents;
            
            bool performDo();
            bool performUndo();
            
            RemoveObjectsCommand(Type type, Model::MapDocument& document, const wxString& name, const Model::EntityList& entities, const Model::BrushList& brushes);
        public:
            ~RemoveObjectsCommand();
            
            static RemoveObjectsCommand* removeObjects(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes);
            static RemoveObjectsCommand* removeEntities(Model::MapDocument& document, const Model::EntityList& entities);
            static RemoveObjectsCommand* removeBrush(Model::MapDocument& document, Model::Brush& brush);
            
            inline const Model::EntityList& removedEntities() const {
                return m_removedEntities;
            }
            
            inline const Model::BrushList& removedBrushes() const {
                return m_removedBrushes;
            }
            
            const Model::EntityList& entities() const {
                return m_entities;
            }
            
            const Model::BrushList& brushes() const {
                return m_brushes;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__RemoveObjectsCommand__) */
