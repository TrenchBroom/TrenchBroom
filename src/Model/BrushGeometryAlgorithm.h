/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef TrenchBroom_BrushGeometryAlgorithm_h
#define TrenchBroom_BrushGeometryAlgorithm_h

#include "CollectionUtils.h"
#include "Model/BrushFaceTypes.h"

namespace TrenchBroom {
    namespace Model {
        class BrushGeometry;
        
        template <typename R>
        class BrushGeometryAlgorithm {
        private:
            BrushFaceList m_addedFaces;
            BrushFaceList m_removedFaces;
            BrushGeometry& m_geometry;
        public:
            BrushGeometryAlgorithm(BrushGeometry& geometry) :
            m_geometry(geometry) {}
            
            virtual ~BrushGeometryAlgorithm() {}

            inline R execute() {
                return doExecute(m_geometry);
            }
            
            inline const BrushFaceList& addedFaces() const {
                return m_addedFaces;
            }
            
            inline const BrushFaceList& removedFaces() const {
                return m_removedFaces;
            }
        protected:
            inline void addFace(const BrushFacePtr face) {
                BrushFaceList::iterator it = VectorUtils::find(m_removedFaces, face);
                if (it != m_removedFaces.end())
                    m_removedFaces.erase(it);
                m_addedFaces.push_back(face);
            }
            
            inline void removeFace(const BrushFacePtr face) {
                BrushFaceList::iterator it = VectorUtils::find(m_addedFaces, face);
                if (it != m_addedFaces.end()) {
                    m_addedFaces.erase(it);
                } else {
                    m_removedFaces.push_back(face);
                }
            }
        private:
            virtual R doExecute(BrushGeometry& geometry) = 0;
        };
    }
}

#endif
