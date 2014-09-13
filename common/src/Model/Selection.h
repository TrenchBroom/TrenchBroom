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

#ifndef __TrenchBroom__Selection__
#define __TrenchBroom__Selection__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/ModelTypes.h"
#include "Model/Object.h"

namespace TrenchBroom {
    namespace Model {
        class ModelFilter;
        class Map;
        class SelectionResult;
        
        class Selection {
        private:
            const ModelFilter& m_filter;
            ObjectList m_selectedObjects;
            EntityList m_selectedEntities;
            EntityList m_partiallySelectedEntities;
            BrushList m_selectedBrushes;
            BrushList m_partiallySelectedBrushes;
            BrushFaceList m_selectedFaces;
            BrushFaceList m_selectedBrushFaces;
        public:
            Selection(const ModelFilter& filter);

            bool hasSelectedObjects() const;
            bool hasSelectedEntities() const;
            bool hasSelectedBrushes() const;
            bool hasSelectedFaces() const;
            bool hasSelection() const;
            
            const ObjectList& selectedObjects() const;
            const EntityList& selectedEntities() const;
            const BrushList& selectedBrushes() const;
            const BrushFaceList& selectedFaces() const;

            EntityList allSelectedEntities() const;
            BrushList allSelectedBrushes() const;
            const BrushFaceList& allSelectedFaces() const;
            
            EntityList unselectedEntities(Map& map) const;
            BrushList unselectedBrushes(Map& map) const;
            BrushFaceList unselectedFaces(Map& map) const;

            const BBox3 computeBounds() const;
            
            SelectionResult selectObjects(const ObjectList& objects);
            SelectionResult deselectObjects(const ObjectList& objects);
            SelectionResult selectAllObjects(Map& map);
            SelectionResult selectAllFaces(Map& map);
            SelectionResult selectFaces(const BrushFaceList& faces, bool keepBrushSelection);
            SelectionResult deselectFaces(const BrushFaceList& faces);
            SelectionResult deselectAll();
            void clear();
        private:
            void convertToFaceSelection(SelectionResult& result);
            void deselectAllObjects(SelectionResult& result);
            void deselectAllFaces(SelectionResult& result);
            
            struct ApplySelectedObjects : public ObjectVisitor {
                Selection& selection;
                ApplySelectedObjects(Selection& i_selection);
                void doVisit(Entity* entity);
                void doVisit(Brush* brush);
            };
            
            struct ApplyDeselectedObjects : public ObjectVisitor {
                Selection& selection;
                ApplyDeselectedObjects(Selection& i_selection);
                void doVisit(Entity* entity);
                void doVisit(Brush* brush);
            };
            
            struct ApplyPartiallySelectedObjects : public ObjectVisitor {
                Selection& selection;
                ApplyPartiallySelectedObjects(Selection& i_selection);
                void doVisit(Entity* entity);
                void doVisit(Brush* brush);
            };
            
            struct ApplyPartiallyDeselectedObjects : public ObjectVisitor {
                Selection& selection;
                ApplyPartiallyDeselectedObjects(Selection& i_selection);
                void doVisit(Entity* entity);
                void doVisit(Brush* brush);
            };
            
            void applyResult(const SelectionResult& result);
            void applySelectedObjects(const ObjectSet& objects);
            void applyDeselectedObjects(const ObjectSet& objects);
            void applyPartiallySelectedObjects(const ObjectSet& objects);
            void applyPartiallyDeselectedObjects(const ObjectSet& objects);
            void applySelectedFaces(const BrushFaceSet& faces);
            void applyDeselectedFaces(const BrushFaceSet& faces);
        };
    }
}

#endif /* defined(__TrenchBroom__Selection__) */
