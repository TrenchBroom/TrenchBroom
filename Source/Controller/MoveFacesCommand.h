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

#ifndef __TrenchBroom__MoveFacesCommand__
#define __TrenchBroom__MoveFacesCommand__

#include "Controller/SnapshotCommand.h"
#include "Model/BrushGeometryTypes.h"
#include "Model/BrushTypes.h"
#include "Model/FaceTypes.h"
#include "Utility/VecMath.h"

#include <map>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Controller {
        class VertexHandleManager;
        
        class MoveFacesCommand : public SnapshotCommand {
        protected:
            VertexHandleManager& m_handleManager;
            
            Model::BrushList m_brushes;
            Model::BrushFacesMap m_brushFaces;
            Model::FaceInfoList m_facesBefore;
            Model::FaceInfoList m_facesAfter;
            Vec3f m_delta;

            bool performDo();
            bool performUndo();

            MoveFacesCommand(Model::MapDocument& document, const wxString& name, VertexHandleManager& handleManager, const Vec3f& delta);
        public:
            static MoveFacesCommand* moveFaces(Model::MapDocument& document, VertexHandleManager& handleManager, const Vec3f& delta);

            bool canDo() const;
        };
    }
}

#endif /* defined(__TrenchBroom__MoveFacesCommand__) */
