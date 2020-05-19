/*
 Copyright (C) 2010-2017 Kristian Duske

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

#ifndef TrenchBroom_BrushFaceSnapshot
#define TrenchBroom_BrushFaceSnapshot

#include "FloatType.h"
#include "Model/BrushFaceAttributes.h"
#include "Model/BrushFaceReference.h"

#include <vecmath/plane.h>

#include <memory>

namespace TrenchBroom {
    namespace Model {
        class BrushFace;
        class BrushNode;
        class TexCoordSystem;
        class TexCoordSystemSnapshot;

        class BrushFaceSnapshot {
        private:
            BrushFaceReference m_faceRef;
            BrushFaceAttributes m_attribs;
            std::unique_ptr<TexCoordSystemSnapshot> m_coordSystemSnapshot;
        public:
            BrushFaceSnapshot(BrushNode* node, const BrushFace* face);
            ~BrushFaceSnapshot();

            void restore();
        };
    }
}

#endif /* defined(TrenchBroom_BrushFaceSnapshot) */
