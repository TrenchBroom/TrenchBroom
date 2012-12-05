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

#ifndef __TrenchBroom__MoveVerticesCommand__
#define __TrenchBroom__MoveVerticesCommand__

#include "Controller/SnapshotCommand.h"
#include "Model/BrushGeometryTypes.h"
#include "Model/BrushTypes.h"
#include "Utility/VecMath.h"

#include <map>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Controller {
        class MoveVerticesCommand : public SnapshotCommand {
        public:
            typedef std::map<Model::Brush*, Vec3f::List> BrushVerticesMap;
        protected:
            BrushVerticesMap m_brushVertices;
            Vec3f m_delta;
            
            bool performDo();
            bool performUndo();

            MoveVerticesCommand(Model::MapDocument& document, const wxString& name, const BrushVerticesMap& brushVertices, const Vec3f& delta);
        public:
            static MoveVerticesCommand* moveVertices(Model::MapDocument& document, const BrushVerticesMap& brushVertices, const Vec3f& delta);
        };
    }
}

#endif /* defined(__TrenchBroom__MoveVerticesCommand__) */
