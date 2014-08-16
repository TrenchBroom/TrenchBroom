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

#ifndef __TrenchBroom__SelectionResult__
#define __TrenchBroom__SelectionResult__

#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {
        class Object;
        class BrushFace;
        
        class SelectionResult {
        private:
            ObjectSet m_selectedObjects;
            ObjectSet m_deselectedObjects;
            ObjectSet m_partiallySelectedObjects;
            ObjectSet m_partiallyDeselectedObjects;
            BrushFaceSet m_selectedFaces;
            BrushFaceSet m_deselectedFaces;
            BrushFace* m_lastSelectedFace;
        public:
            SelectionResult();
            
            const ObjectSet& selectedObjects() const;
            const ObjectSet& deselectedObjects() const;
            const ObjectSet& partiallySelectedObjects() const;
            const ObjectSet& partiallyDeselectedObjects() const;
            const BrushFaceSet& selectedFaces() const;
            const BrushFaceSet& deselectedFaces() const;
            
            void addSelectedObject(Object* object);
            void addDeselectedObject(Object* object);
            void addPartiallySelectedObject(Object* object);
            void addPartiallyDeselectedObject(Object* object);
            void addSelectedFace(BrushFace* face);
            void addDeselectedFace(BrushFace* face);
            
            BrushFace* lastSelectedFace() const;
            
            SelectionResult& operator+=(const SelectionResult& rhs);
            SelectionResult operator+(const SelectionResult& rhs) const;
            void mergeWith(const SelectionResult& other);
        };
    }
}

#endif /* defined(__TrenchBroom__SelectionResult__) */
