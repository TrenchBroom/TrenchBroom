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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_BrushAlgorithm_h
#define TrenchBroom_BrushAlgorithm_h

#include "CollectionUtils.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {
        class BrushGeometry;
        
        template <typename R>
        class BrushAlgorithm {
        private:
            BrushGeometry& m_geometry;
        protected:
            BrushFaceList m_addedFaces;
            BrushFaceList m_removedFaces;
        public:
            virtual ~BrushAlgorithm() {}

            bool canExecute() {
                return doCanExecute(m_geometry);
            }
            
            R execute() {
                return doExecute(m_geometry);
            }
            
            const BrushFaceList& addedFaces() const {
                return m_addedFaces;
            }
            
            const BrushFaceList& removedFaces() const {
                return m_removedFaces;
            }
        protected:
            BrushAlgorithm(BrushGeometry& geometry) :
            m_geometry(geometry) {}

            void addFace(BrushFace* face) {
                BrushFaceList::iterator it = VectorUtils::find(m_removedFaces, face);
                if (it != m_removedFaces.end())
                    m_removedFaces.erase(it);
                m_addedFaces.push_back(face);
            }
            
            void removeFace(BrushFace* face) {
                BrushFaceList::iterator it = VectorUtils::find(m_addedFaces, face);
                if (it != m_addedFaces.end()) {
                    m_addedFaces.erase(it);
                } else {
                    m_removedFaces.push_back(face);
                }
            }
        private:
            virtual bool doCanExecute(BrushGeometry& geometry) { return true; }
            virtual R doExecute(BrushGeometry& geometry) = 0;
        };
    }
}

#endif
