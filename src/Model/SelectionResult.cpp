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

#include "SelectionResult.h"
#include "CollectionUtils.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        SelectionResult::SelectionResult() :
        m_lastSelectedFace(NULL) {}

        const ObjectSet& SelectionResult::selectedObjects() const {
            return m_selectedObjects;
        }
        
        const ObjectSet& SelectionResult::deselectedObjects() const {
            return m_deselectedObjects;
        }
            
        const BrushFaceSet& SelectionResult::selectedFaces() const {
            return m_selectedFaces;
        }
        
        const BrushFaceSet& SelectionResult::deselectedFaces() const {
            return m_deselectedFaces;
        }

        void SelectionResult::addSelectedObject(Object* object) {
            m_deselectedObjects.erase(object);
            m_selectedObjects.insert(object);
        }
        
        void SelectionResult::addDeselectedObject(Object* object) {
            m_selectedObjects.erase(object);
            m_deselectedObjects.insert(object);
        }
        
        void SelectionResult::addSelectedFace(BrushFace* face) {
            m_deselectedFaces.erase(face);
            m_selectedFaces.insert(face);
            m_lastSelectedFace = face;
        }
        
        void SelectionResult::addDeselectedFace(BrushFace* face) {
            m_selectedFaces.erase(face);
            m_deselectedFaces.insert(face);
        }

        BrushFace* SelectionResult::lastSelectedFace() const {
            return m_lastSelectedFace;
        }

        SelectionResult& SelectionResult::operator+=(const SelectionResult& rhs) {
            mergeWith(rhs);
            return *this;
        }
        
        SelectionResult SelectionResult::operator+(const SelectionResult& rhs) const {
            SelectionResult result = *this;
            return result += rhs;
        }

        void SelectionResult::mergeWith(const SelectionResult& other) {
            assert(this != &other);
            
            const Model::ObjectSet& otherSelectedObjects = other.selectedObjects();
            const Model::ObjectSet& otherDeselectedObjects = other.deselectedObjects();
            const Model::BrushFaceSet& otherSelectedFaces = other.selectedFaces();
            const Model::BrushFaceSet& otherDeselectedFaces = other.deselectedFaces();
            
            m_deselectedObjects = SetUtils::minus(m_deselectedObjects, otherSelectedObjects);
            m_selectedObjects = SetUtils::minus(m_selectedObjects, otherDeselectedObjects);
            m_selectedObjects.insert(otherSelectedObjects.begin(), otherSelectedObjects.end());
            m_deselectedObjects.insert(otherDeselectedObjects.begin(), otherDeselectedObjects.end());
            
            m_deselectedFaces = SetUtils::minus(m_deselectedFaces, otherSelectedFaces);
            m_selectedFaces = SetUtils::minus(m_selectedFaces, otherDeselectedFaces);
            m_selectedFaces.insert(otherSelectedFaces.begin(), otherSelectedFaces.end());
            m_deselectedFaces.insert(otherDeselectedFaces.begin(), otherDeselectedFaces.end());
            
            if (other.lastSelectedFace() != NULL)
                m_lastSelectedFace = other.lastSelectedFace();
        }
    }
}
