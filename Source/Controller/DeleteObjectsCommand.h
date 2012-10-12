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

#ifndef __TrenchBroom__DeleteObjectsCommand__
#define __TrenchBroom__DeleteObjectsCommand__

#include "Controller/Command.h"
#include "Model/EntityTypes.h"
#include "Model/BrushTypes.h"

namespace TrenchBroom {
    namespace Controller {
        class DeleteObjectsCommand : public DocumentCommand {
        protected:
            Model::EntityList m_entities;
            Model::BrushList m_brushes;
            
            Model::EntityList m_deletedEntities;
            Model::BrushParentMap m_deletedBrushes;
            
            bool performDo();
            bool performUndo();
            
            DeleteObjectsCommand(Type type, Model::MapDocument& document, const wxString& name, const Model::EntityList& entities, const Model::BrushList& brushes);
        public:
            static DeleteObjectsCommand* deleteObjects(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes, const wxString action = wxT("Delete"));
            
            inline const Model::EntityList& deletedEntities() const {
                return m_deletedEntities;
            }
            
            inline const Model::BrushList deletedBrushes() const {
                Model::BrushList result;
                Model::BrushParentMap::const_iterator it, end;
                for (it = m_deletedBrushes.begin(), end = m_deletedBrushes.end(); it != end; ++it)
                    result.push_back(it->first);
                return result;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__DeleteObjectsCommand__) */
