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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__ReparentBrushesCommand__
#define __TrenchBroom__ReparentBrushesCommand__

#include "SharedPointer.h"
#include "Controller/DocumentCommand.h"
#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Controller {
        class ReparentBrushesCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            typedef std::tr1::shared_ptr<ReparentBrushesCommand> Ptr;
        private:
            Model::BrushList m_brushes;
            Model::Entity* m_newParent;
            Model::BrushEntityMap m_oldParents;
            Model::ObjectLayerMap m_oldLayers;
            Model::EntityList m_emptyEntities;
        public:
            static Ptr reparent(View::MapDocumentWPtr document, const Model::BrushList& brushes, Model::Entity* newParent);
            const Model::EntityList& emptyEntities() const;
        private:
            ReparentBrushesCommand(View::MapDocumentWPtr document, const Model::BrushList& brushes, Model::Entity* newParent);
            static String makeName(const Model::BrushList& brushes, Model::Entity* newParent);
            
            bool doPerformDo();
            bool doPerformUndo();

            bool doIsRepeatable(View::MapDocumentSPtr document) const;
            Command* doRepeat(View::MapDocumentSPtr document) const;

            bool doCollateWith(Command::Ptr command);
        };
    }
}

#endif /* defined(__TrenchBroom__ReparentBrushesCommand__) */
