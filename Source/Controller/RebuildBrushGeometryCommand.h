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

#ifndef __TrenchBroom__RebuildBrushGeometryCommand__
#define __TrenchBroom__RebuildBrushGeometryCommand__

#include "Controller/SnapshotCommand.h"
#include "Model/BrushTypes.h"

namespace TrenchBroom {
    namespace Controller {
        class RebuildBrushGeometryCommand : public SnapshotCommand {
        protected:
            Model::BrushList m_brushes;
            size_t m_precedingChangeCount;
            
            bool performDo();
            bool performUndo();
            
            RebuildBrushGeometryCommand(Model::MapDocument& document, const wxString& name, const Model::BrushList& brushes, size_t precedingChangeCount);
        public:
            static RebuildBrushGeometryCommand* rebuildGeometry(Model::MapDocument& document, const Model::BrushList& brushes);
            static RebuildBrushGeometryCommand* rebuildGeometry(Model::MapDocument& document, const Model::BrushList& brushes, size_t precedingChangeCount);
            
            inline const Model::BrushList& brushes() const {
                return m_brushes;
            }
            
            inline bool activateMoveVerticesTool() const {
                return m_precedingChangeCount > 0;
            }
            
            inline size_t precedingChangeCount() const {
                return m_precedingChangeCount;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__RebuildBrushGeometryCommand__) */
