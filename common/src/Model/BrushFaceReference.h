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

#ifndef BrushFaceReference_h
#define BrushFaceReference_h

#include "FloatType.h"
#include "Model/BrushFaceHandle.h"

#include <vecmath/plane.h>

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushNode;

        /**
         * A brush face reference creates a persistent reference to a face that has a specific boundary plane. It can
         * be resolved later, and if the brush then has a face with the same boundary plane, that face will be returned.
         *
         * Depending on how the brush was modified in time since the reference has been created, the resolved face might
         * not be the same as the face which the reference was created with.
         */
        class BrushFaceReference {
        private:
            BrushNode* m_node;
            vm::plane3 m_facePlane;
        public:
            /**
             * Creates a new reference to the given face.
             *
             * @param node the containing brush node, must not be null
             * @param face the face to reference, must not be null
             */
            BrushFaceReference(Model::BrushNode* node, const Model::BrushFace* face);

            /**
             * Resolves the referenced brush face.
             *
             * @throws BrushFaceReferenceException if the face cannot be resolved
             */
            BrushFaceHandle resolve() const;
        };

        /**
         * Returns a vector of brush face references for faces represented by the given handles.
         */
        std::vector<BrushFaceReference> createRefs(const std::vector<BrushFaceHandle>& handles);

        /**
         * Returns a vector brush face handles representing the faces to which the given face references are resolved.
         *
         * @throws BrushFAceReferenceException if any of the given face references cannot be resolved
         */
        std::vector<BrushFaceHandle> resolveAllRefs(const std::vector<BrushFaceReference>& faceRefs);
    }
}

#endif /* BrushFaceReference_h */
