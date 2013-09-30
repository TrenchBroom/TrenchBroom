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
        
        class SetSelection {
        private:
            bool m_select;
            SelectionResult& m_result;
        public:
            SetSelection(const bool select, SelectionResult& result) :
            m_select(select),
            m_result(result) {}
            
            void operator()(Object* object) const {
                if (m_select) {
                    if (!object->selected()) {
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
                    if (!entity->selected()) {
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
                    if (!brush->selected()) {
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
                    if (!face->selected()) {
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

        Selection::Selection(Map* map) :
        m_map(map),
        m_lastSelectedFace(NULL) {}

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
            if (m_map == NULL)
                return false;
            
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
            assert(!(!m_selectedFaces.empty() && !m_selectedBrushFaces.empty()));
            if (!m_selectedFaces.empty())
                return m_selectedFaces;
            return m_selectedBrushFaces;
        }
        
        EntityList Selection::unselectedEntities() const {
            if (m_map == NULL)
                return EmptyEntityList;
            
            EntityList result;
            filter(MapEntitiesIterator::begin(*m_map),
                   MapEntitiesIterator::end(*m_map),
                   MatchUnselected(),
                   std::back_inserter(result));
            return result;
        }

        BrushList Selection::unselectedBrushes() const {
            if (m_map == NULL)
                return EmptyBrushList;
            
            BrushList result;
            filter(MapBrushesIterator::begin(*m_map),
                   MapBrushesIterator::end(*m_map),
                   MatchUnselected(),
                   std::back_inserter(result));
            return result;
        }

        BrushFaceList Selection::unselectedFaces() const {
            if (m_map == NULL)
                return EmptyBrushFaceList;
            
            BrushFaceList result;
            filter(MapFacesIterator::begin(*m_map),
                   MapFacesIterator::end(*m_map),
                   MatchUnselected(),
                   std::back_inserter(result));
            return result;
        }

        BrushFace* Selection::lastSelectedFace() const {
            if (m_map == NULL)
                return NULL;
            return m_lastSelectedFace;
        }
        
        SelectionResult Selection::selectObjects(const ObjectList& objects) {
            assert(m_map != NULL);

            SelectionResult result;
            if (!objects.empty()) {
                deselectAllFaces(result);
                each(objects.begin(),
                     objects.end(),
                     SetSelection(true, result),
                     MatchAll());
                applyResult(result);
            }
            return result;
        }
        
        SelectionResult Selection::deselectObjects(const ObjectList& objects) {
            assert(m_map != NULL);
            SelectionResult result;
            if (!objects.empty()) {
                each(objects.begin(),
                     objects.end(),
                     SetSelection(false, result),
                     MatchAll());
                applyResult(result);
            }
            return result;
        }
        
        SelectionResult Selection::selectAllObjects() {
            assert(m_map != NULL);
            
            SelectionResult result;
            each(MapObjectsIterator::begin(*m_map),
                 MapObjectsIterator::end(*m_map),
                 SetSelection(true, result),
                 MatchUnselected());
            applyResult(result);
            return result;
        }
        
        SelectionResult Selection::selectFaces(const BrushFaceList& faces) {
            assert(m_map != NULL);
            
            SelectionResult result;
            if (!faces.empty()) {
                deselectAllObjects(result);
                each(faces.begin(),
                     faces.end(),
                     SetSelection(true, result),
                     MatchAll());
                applyResult(result);
            }
            
            if (result.lastSelectedFace() != NULL)
                m_lastSelectedFace = result.lastSelectedFace();
            
            return result;
        }
        
        SelectionResult Selection::deselectFaces(const BrushFaceList& faces) {
            assert(m_map != NULL);
            
            SelectionResult result;
            if (!faces.empty()) {
                each(faces.begin(),
                     faces.end(),
                     SetSelection(false, result),
                     MatchAll());
                applyResult(result);
            }
            return result;
        }

        SelectionResult Selection::deselectAll() {
            assert(m_map != NULL);
            
            SelectionResult result;
            deselectAllObjects(result);
            deselectAllFaces(result);
            applyResult(result);
            return result;
        }

        void Selection::deselectAllObjects(SelectionResult& result) {
            assert(m_map != NULL);
            
            each(MapObjectsIterator::begin(*m_map),
                 MapObjectsIterator::end(*m_map),
                 SetSelection(false, result),
                 MatchSelected());
        }
        
        void Selection::deselectAllFaces(SelectionResult& result) {
            assert(m_map != NULL);
            
            each(MapFacesIterator::begin(*m_map),
                 MapFacesIterator::end(*m_map),
                 SetSelection(false, result),
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
                VectorUtils::insertOrdered(m_selectedObjects, object);
                
                if (object->type() == Object::OTEntity) {
                    Entity* entity = static_cast<Entity*>(object);
                    VectorUtils::insertOrdered(m_selectedEntities, entity);
                } else {
                    Brush* brush = static_cast<Brush*>(object);
                    VectorUtils::insertOrdered(m_selectedBrushes, brush);
                    
                    const BrushFaceList& brushFaces = brush->faces();
                    VectorUtils::insertOrdered(m_selectedBrushFaces, brushFaces.begin(), brushFaces.end());
                    
                    Entity* entity = brush->parent();
                    VectorUtils::insertOrdered(m_partiallySelectedEntities, entity);
                }
            }
        }
        
        void Selection::applyDeselectedObjects(const ObjectSet& objects) {
            ObjectSet::const_iterator it, end;
            for (it = objects.begin(), end = objects.end(); it != end; ++it) {
                Object* object = *it;
                VectorUtils::removeOrdered(m_selectedObjects, object);
                
                if (object->type() == Object::OTEntity) {
                    Entity* entity = static_cast<Entity*>(object);
                    VectorUtils::removeOrdered(m_selectedEntities, entity);
                } else {
                    Brush* brush = static_cast<Brush*>(object);
                    VectorUtils::removeOrdered(m_selectedBrushes, brush);
                    
                    const BrushFaceList& brushFaces = brush->faces();
                    VectorUtils::removeOrdered(m_selectedBrushFaces, brushFaces.begin(), brushFaces.end());

                    Entity* entity = brush->parent();
                    if (!entity->partiallySelected())
                        VectorUtils::removeOrdered(m_partiallySelectedEntities, entity);
                }
            }
        }
        
        void Selection::applySelectedFaces(const BrushFaceSet& faces) {
            BrushFaceSet::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                BrushFace* face = *it;
                VectorUtils::insertOrdered(m_selectedFaces, face);
                
                Brush* brush = face->parent();
                VectorUtils::insertOrdered(m_partiallySelectedBrushes, brush);
            }
        }
        
        void Selection::applyDeselectedFaces(const BrushFaceSet& faces) {
            BrushFaceSet::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                BrushFace* face = *it;
                VectorUtils::removeOrdered(m_selectedFaces, face);
                
                Brush* brush = face->parent();
                if (!brush->partiallySelected())
                    VectorUtils::removeOrdered(m_partiallySelectedBrushes, brush);
            }
        }
    }
}
