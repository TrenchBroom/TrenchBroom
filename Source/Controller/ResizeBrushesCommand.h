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

#ifndef __TrenchBroom__ResizeBrushesCommand__
#define __TrenchBroom__ResizeBrushesCommand__

#include "Controller/SnapshotCommand.h"
#include "Model/BrushTypes.h"
#include "Model/FaceTypes.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Controller {
        class ResizeBrushesCommand : public DocumentCommand {
        protected:
            const Model::FaceList m_faces;
            const Model::BrushList m_brushes;
            const Vec3f m_delta;
            const bool m_lockTextures;
            
            bool performDo();
            bool performUndo();

            ResizeBrushesCommand(Model::MapDocument& document, const wxString& name, const Model::FaceList& faces, const Model::BrushList& brushes, const Vec3f& delta, bool lockTextures);
        public:
            static ResizeBrushesCommand* resizeBrushes(Model::MapDocument& document, const Model::FaceList& faces, const Vec3f& delta, bool lockTextures);
        };
    }
}

#endif /* defined(__TrenchBroom__ResizeBrushesCommand__) */
