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

#include "Selection.h"

#include "CollectionUtils.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/Map.h"
#include "Model/MapBrushesIterator.h"
#include "Model/MapEntitiesIterator.h"
#include "Model/MapFacesIterator.h"
#include "Model/MapObjectsIterator.h"
#include "Model/ModelFilter.h"
#include "Model/ModelUtils.h"
#include "Model/SelectionResult.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        struct Collect {
            EntityList entities;
            BrushList brushes;
            BrushFaceList faces;
            
            void operator()(Entity* entity) {
                entities.push_back(entity);
            }
            
            void operator()(Brush* brush) {
                brushes.push_back(brush);
            }
            
            void operator()(BrushFace* face) {
                faces.push_back(face);
            }
        };
        
        struct CollectFaces {
            BrushFaceList faces;

            void operator()(Brush* brush) {
                VectorUtils::append(faces, brush->faces());
            }
        };
        
        class SetSelection {
        private:
            bool m_select;
            const ModelFilter& m_filter;
            SelectionResult& m_result;
        public:
            SetSelection(const bool select, const ModelFilter& filter, SelectionResult& result) :
            m_select(select),
            m_filter(filter),
            m_result(result) {}
            
            void operator()(Object* object) const {
                if (m_select) {
                    if (!object->selected() && m_filter.selectable(object)) {
                        object->select();
                        m_result.addSelectedObject(object);
                    }
                } else {
                    if (object->selected()) {
                        object->deselect();
                        m_result.addDeselectedObject(object);
                    }
                }
            }

            void operator()(Entity* entity) const {
                if (m_select) {
                    if (!entity->selected() && m_filter.selectable(entity)) {
                        entity->select();
                        m_result.addSelectedObject(entity);
                    }
                } else {
                    if (entity->selected()) {
                        entity->deselect();
                        m_result.addDeselectedObject(entity);
                    }
                }
            }
            
            void operator()(Brush* brush) const {
                if (m_select) {
                    if (!brush->selected() && m_filter.selectable(brush)) {
                        brush->select();
                        m_result.addSelectedObject(brush);
                    }
                } else {
                    if (brush->selected()) {
                        brush->deselect();
                        m_result.addDeselectedObject(brush);
                    }
                }
            }
            
            void operator()(BrushFace* face) const {
                if (m_select) {
                    if (!face->selected() && m_filter.selectable(face)) {
                        face->select();
                        m_result.addSelectedFace(face);
                    }
                } else {
                    if (face->selected()) {
                        face->deselect();
                        m_result.addDeselectedFace(face);
                    }
                }
            }
        };

        struct MatchSelected  {
            bool operator()(const Object* object) const {
                return object->selected();
            }
            
            bool operator()(const Entity* entity) const {
                return entity->selected();
            }
            
            bool operator()(const Brush* brush) const {
                return brush->selected();
            }

            bool operator()(const BrushFace* face) const {
                return face->selected();
            }
        };
        
        struct MatchPartiallySelected  {
            bool operator()(const Entity* entity) const {
                return entity->selected() || entity->partiallySelected();
            }
            
            bool operator()(const Brush* brush) const {
                return brush->selected() || brush->partiallySelected();
            }
        };
        
        struct MatchAnySelectedFace  {
            bool operator()(const BrushFace* face) const {
                return face->selected() || face->parent()->selected();
            }
        };

        struct MatchUnselected {
            bool operator()(const Object* object) const {
                return !object->selected();
            }
            
            bool operator()(const Entity* entity) const {
                return !entity->selected();
            }
            
            bool operator()(const Brush* brush) const {
                return !brush->selected();
            }
            
            bool operator()(const BrushFace* face) const {
                return !face->selected();
            }
        };

        Selection::Selection(const ModelFilter& filter) :
        m_filter(filter),
        m_lastSelectedFace(NULL),
        m_boundsValid(false) {}

        bool Selection::hasSelectedObjects() const {
            return !m_selectedObjects.empty();
        }
        
        bool Selection::hasSelectedEntities() const {
            return !m_selectedEntities.empty();
        }
        
        bool Selection::hasSelectedBrushes() const {
            return !m_selectedBrushes.empty();
        }

        bool Selection::hasSelectedFaces() const {
            return !m_selectedFaces.empty();
        }
        
        bool Selection::hasSelection() const {
            return hasSelectedObjects() || hasSelectedFaces();
        }

        const ObjectList& Selection::selectedObjects() const {
            return m_selectedObjects;
        }
        
        const EntityList& Selection::selectedEntities() const {
            return m_selectedEntities;
        }
        
        const BrushList& Selection::selectedBrushes() const {
            return m_selectedBrushes;
        }
        
        const BrushFaceList& Selection::selectedFaces() const {
            return m_selectedFaces;
        }
        
        EntityList Selection::allSelectedEntities() const {
            return VectorUtils::concatenate(m_selectedEntities, m_partiallySelectedEntities);
        }
        
        BrushList Selection::allSelectedBrushes() const {
            return VectorUtils::concatenate(m_selectedBrushes, m_partiallySelectedBrushes);
        }
        
        const BrushFaceList& Selection::allSelectedFaces() const {
            if (!m_selectedFaces.empty())
                return m_selectedFaces;
            return m_selectedBrushFaces;
        }
        
        EntityList Selection::unselectedEntities(Map& map) const {
            EntityList result;
            filter(MapEntitiesIterator::begin(map),
                   MapEntitiesIterator::end(map),
                   MatchUnselected(),
                   std::back_inserter(result));
            return result;
        }

        BrushList Selection::unselectedBrushes(Map& map) const {
            BrushList result;
            filter(MapBrushesIterator::begin(map),
                   MapBrushesIterator::end(map),
                   MatchUnselected(),
                   std::back_inserter(result));
            return result;
        }

        BrushFaceList Selection::unselectedFaces(Map& map) const {
            BrushFaceList result;
            filter(MapFacesIterator::begin(map),
                   MapFacesIterator::end(map),
                   MatchUnselected(),
                   std::back_inserter(result));
            return result;
        }

        BrushFace* Selection::lastSelectedFace() const {
            return m_lastSelectedFace;
        }
        
        const BBox3& Selection::bounds() const {
            assert(hasSelectedObjects());
            
            if (!m_boundsValid) {
                m_bounds = Object::bounds(m_selectedObjects);
                m_boundsValid = true;
            }
            
            return m_bounds;
        }

        SelectionResult Selection::selectObjects(const ObjectList& objects) {
            SelectionResult result;
            if (!objects.empty()) {
                deselectAllFaces(result);
                each(objects.begin(),
                     objects.end(),
                     SetSelection(true, m_filter, result),
                     MatchAll());
                applyResult(result);
            }
            m_boundsValid = false;
            return result;
        }
        
        SelectionResult Selection::deselectObjects(const ObjectList& objects) {
            SelectionResult result;
            if (!objects.empty()) {
                each(objects.begin(),
                     objects.end(),
                     SetSelection(false, m_filter, result),
                     MatchAll());
                applyResult(result);
            }
            m_boundsValid = false;
            return result;
        }
        
        SelectionResult Selection::selectAllObjects(Map& map) {
            SelectionResult result;
            each(MapObjectsIterator::begin(map),
                 MapObjectsIterator::end(map),
                 SetSelection(true, m_filter, result),
                 MatchUnselected());
            applyResult(result);
            m_boundsValid = false;
            return result;
        }
        
        SelectionResult Selection::selectAllFaces(Map& map) {
            SelectionResult result;
            each(MapFacesIterator::begin(map),
                 MapFacesIterator::end(map),
                 SetSelection(true, m_filter, result),
                 MatchUnselected());
            applyResult(result);
            return result;
        }

        SelectionResult Selection::selectFaces(const BrushFaceList& faces, const bool keepBrushSelection) {
            SelectionResult result;
            if (!faces.empty()) {
                if (keepBrushSelection)
                    convertToFaceSelection(result);
                else
                    deselectAllObjects(result);
                each(faces.begin(),
                     faces.end(),
                     SetSelection(true, m_filter, result),
                     MatchAll());
                applyResult(result);
            }
            
            if (result.lastSelectedFace() != NULL)
                m_lastSelectedFace = result.lastSelectedFace();
            
            return result;
        }
        
        SelectionResult Selection::deselectFaces(const BrushFaceList& faces) {
            SelectionResult result;
            if (!faces.empty()) {
                convertToFaceSelection(result);
                each(faces.begin(),
                     faces.end(),
                     SetSelection(false, m_filter, result),
                     MatchAll());
                applyResult(result);
            }
            return result;
        }

        SelectionResult Selection::deselectAll() {
            SelectionResult result;
            deselectAllObjects(result);
            deselectAllFaces(result);
            m_selectedObjects.clear();
            m_selectedFaces.clear();
            m_selectedEntities.clear();
            m_selectedBrushes.clear();
            m_selectedBrushFaces.clear();
            m_partiallySelectedEntities.clear(),
            m_partiallySelectedBrushes.clear();
            return result;
        }
        
        void Selection::clear() {
            m_selectedObjects.clear();
            m_selectedFaces.clear();
            m_selectedEntities.clear();
            m_selectedBrushes.clear();
            m_selectedBrushFaces.clear();
            m_partiallySelectedEntities.clear(),
            m_partiallySelectedBrushes.clear();
            m_lastSelectedFace = NULL;
        }

        void Selection::convertToFaceSelection(SelectionResult& result) {
            CollectFaces collect;
            each(m_selectedBrushes.begin(),
                 m_selectedBrushes.end(),
                 collect,
                 MatchSelected());
            deselectAllObjects(result);
            each(collect.faces.begin(),
                 collect.faces.end(),
                 SetSelection(true, m_filter, result),
                 MatchAll());
        }

        void Selection::deselectAllObjects(SelectionResult& result) {
            each(m_selectedObjects.begin(),
                 m_selectedObjects.end(),
                 SetSelection(false, m_filter, result),
                 MatchSelected());
            m_boundsValid = false;
        }
        
        void Selection::deselectAllFaces(SelectionResult& result) {
            each(m_selectedFaces.begin(),
                 m_selectedFaces.end(),
                 SetSelection(false, m_filter, result),
                 MatchSelected());
        }

        void Selection::applyResult(const SelectionResult& result) {
            applySelectedObjects(result.selectedObjects());
            applyDeselectedObjects(result.deselectedObjects());
            applySelectedFaces(result.selectedFaces());
            applyDeselectedFaces(result.deselectedFaces());
        }

        void Selection::applySelectedObjects(const ObjectSet& objects) {
            ObjectSet::const_iterator it, end;
            for (it = objects.begin(), end = objects.end(); it != end; ++it) {
                Object* object = *it;
                VectorUtils::setInsert(m_selectedObjects, object);
                
                if (object->type() == Object::Type_Entity) {
                    Entity* entity = static_cast<Entity*>(object);
                    VectorUtils::setInsert(m_selectedEntities, entity);
                } else {
                    Brush* brush = static_cast<Brush*>(object);
                    VectorUtils::setInsert(m_selectedBrushes, brush);
                    
                    const BrushFaceList& brushFaces = brush->faces();
                    VectorUtils::setInsert(m_selectedBrushFaces, brushFaces.begin(), brushFaces.end());
                    
                    Entity* entity = brush->parent();
                    VectorUtils::setInsert(m_partiallySelectedEntities, entity);
                }
            }
        }
        
        void Selection::applyDeselectedObjects(const ObjectSet& objects) {
            ObjectSet::const_iterator it, end;
            for (it = objects.begin(), end = objects.end(); it != end; ++it) {
                Object* object = *it;
                VectorUtils::setRemove(m_selectedObjects, object);
                
                if (object->type() == Object::Type_Entity) {
                    Entity* entity = static_cast<Entity*>(object);
                    VectorUtils::setRemove(m_selectedEntities, entity);
                } else {
                    Brush* brush = static_cast<Brush*>(object);
                    VectorUtils::setRemove(m_selectedBrushes, brush);
                    
                    const BrushFaceList& brushFaces = brush->faces();
                    VectorUtils::setRemove(m_selectedBrushFaces, brushFaces.begin(), brushFaces.end());

                    Entity* entity = brush->parent();
                    if (!entity->partiallySelected())
                        VectorUtils::setRemove(m_partiallySelectedEntities, entity);
                }
            }
        }
        
        void Selection::applySelectedFaces(const BrushFaceSet& faces) {
            BrushFaceSet::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                BrushFace* face = *it;
                VectorUtils::setInsert(m_selectedFaces, face);
                
                Brush* brush = face->parent();
                VectorUtils::setInsert(m_partiallySelectedBrushes, brush);
            }
        }
        
        void Selection::applyDeselectedFaces(const BrushFaceSet& faces) {
            BrushFaceSet::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                BrushFace* face = *it;
                VectorUtils::setRemove(m_selectedFaces, face);
                
                Brush* brush = face->parent();
                if (!brush->partiallySelected())
                    VectorUtils::setRemove(m_partiallySelectedBrushes, brush);
            }
        }
    }
}
