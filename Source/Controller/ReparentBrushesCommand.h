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

#ifndef __TrenchBroom__ReparentBrushesCommand__
#define __TrenchBroom__ReparentBrushesCommand__

#include "Controller/Command.h"
#include "Model/BrushTypes.h"

namespace TrenchBroom {
    namespace Model {
        class Entity;
        class MapDocument;
    }
    
    namespace Controller {
        class ReparentBrushesCommand : public DocumentCommand {
        protected:
            const Model::BrushList m_brushes;
            Model::BrushParentMap m_oldParents;
            Model::Entity& m_newParent;
            
            bool performDo();
            bool performUndo();

            ReparentBrushesCommand(Model::MapDocument& document, const wxString& name, const Model::BrushList& brushes, Model::Entity& newParent);
        public:
            static ReparentBrushesCommand* reparent(Model::MapDocument& document, const Model::BrushList& brushes, Model::Entity& newParent);
        };
    }
}

#endif /* defined(__TrenchBroom__ReparentBrushesCommand__) */
