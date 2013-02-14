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

#ifndef __TrenchBroom__SnapVerticesCommand__
#define __TrenchBroom__SnapVerticesCommand__

#include "Controller/SnapshotCommand.h"

namespace TrenchBroom {
    namespace Controller {
        class SnapVerticesCommand : public SnapshotCommand {
        protected:
            Model::BrushList m_brushes;
            unsigned int m_snapTo;
            
            bool performDo();
            bool performUndo();

            SnapVerticesCommand(Model::MapDocument& document, const wxString& name, const Model::BrushList& brushes, unsigned int snapTo = 0);
        public:
            static SnapVerticesCommand* correct(Model::MapDocument& document, const Model::BrushList& brushes);
            static SnapVerticesCommand* snapTo1(Model::MapDocument& document, const Model::BrushList& brushes);
            static SnapVerticesCommand* snapToGrid(Model::MapDocument& document, const Model::BrushList& brushes);
        };
    }
}

#endif /* defined(__TrenchBroom__SnapVerticesCommand__) */
