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
            
            inline void operator()(Entity* entity) {
                entities.push_back(entity);
            }
            
            inline void operator()(Brush* brush) {
                brushes.push_back(brush);
            }
            
            inline void operator()(BrushFace* face) {
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
            
            inline void operator()(Object* object) const {
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

            inline void operator()(Entity* entity) const {
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
            
            inline void operator()(Brush* brush) const {
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
            
            inline void operator()(BrushFace* face) const {
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
            inline bool operator()(const Object* object) const {
                return object->selected();
            }
            
            inline bool operator()(const Entity* entity) const {
                return entity->selected();
            }
            
            inline bool operator()(const Brush* brush) const {
                return brush->selected();
            }

            inline bool operator()(const BrushFace* face) const {
                return face->selected();
            }
        };
        
        struct MatchPartiallySelected  {
            inline bool operator()(const Entity* entity) const {
                return entity->selected() || entity->partiallySelected();
            }
            
            inline bool operator()(const Brush* brush) const {
                return brush->selected() || brush->partiallySelected();
            }
        };
        
        struct MatchUnselected {
            inline bool operator()(const Object* object) const {
                return !object->selected();
            }
            
            inline bool operator()(const Entity* entity) const {
                return !entity->selected();
            }
            
            inline bool operator()(const Brush* brush) const {
                return !brush->selected();
            }
            
            inline bool operator()(const BrushFace* face) const {
                return !face->selected();
            }
        };

        Selection::Selection(Map* map) :
        m_map(map),
        m_lastSelectedFace(NULL) {}

        bool Selection::hasSelectedObjects() const {
            if (m_map == NULL)
                return false;
            
            return any(MapObjectsIterator::begin(*m_map),
                       MapObjectsIterator::end(*m_map),
                       MatchSelected());
        }
        
        bool Selection::hasSelectedFaces() const {
            if (m_map == NULL)
                return false;
            
            return any(MapFacesIterator::begin(*m_map),
                       MapFacesIterator::end(*m_map),
                       MatchSelected());
        }
        
        bool Selection::hasSelection() const {
            if (m_map == NULL)
                return false;
            
            return hasSelectedObjects() || hasSelectedFaces();
        }

        ObjectList Selection::selectedObjects() const {
            if (m_map == NULL)
                return EmptyObjectList;
            
            ObjectList result;
            filter(MapObjectsIterator::begin(*m_map),
                   MapObjectsIterator::end(*m_map),
                   MatchSelected(),
                   std::back_inserter(result));
            return result;
        }
        
        EntityList Selection::selectedEntities() const {
            if (m_map == NULL)
                return EmptyEntityList;

            EntityList result;
            filter(MapEntitiesIterator::begin(*m_map),
                   MapEntitiesIterator::end(*m_map),
                   MatchSelected(),
                   std::back_inserter(result));
            return result;
        }
        
        EntityList Selection::allSelectedEntities() const {
            if (m_map == NULL)
                return EmptyEntityList;
            
            EntityList result;
            filter(MapEntitiesIterator::begin(*m_map),
                   MapEntitiesIterator::end(*m_map),
                   MatchPartiallySelected(),
                   std::back_inserter(result));
            return result;
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

        BrushList Selection::selectedBrushes() const {
            if (m_map == NULL)
                return EmptyBrushList;
            
            BrushList result;
            filter(MapBrushesIterator::begin(*m_map),
                   MapBrushesIterator::end(*m_map),
                   MatchSelected(),
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

        BrushFaceList Selection::selectedFaces() const {
            if (m_map == NULL)
                return EmptyBrushFaceList;
            
            BrushFaceList result;
            filter(MapFacesIterator::begin(*m_map),
                   MapFacesIterator::end(*m_map),
                   MatchSelected(),
                   std::back_inserter(result));
            return result;
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
            }
            return result;
        }
        
        SelectionResult Selection::deselectObjects(const ObjectList& objects) {
            assert(m_map != NULL);
            SelectionResult result;
            if (!objects.empty())
                each(objects.begin(),
                     objects.end(),
                     SetSelection(false, result),
                     MatchAll());
            return result;
        }
        
        SelectionResult Selection::selectAllObjects() {
            assert(m_map != NULL);
            
            SelectionResult result;
            each(MapObjectsIterator::begin(*m_map),
                 MapObjectsIterator::end(*m_map),
                 SetSelection(true, result),
                 MatchUnselected());
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
            }
            
            if (result.lastSelectedFace() != NULL)
                m_lastSelectedFace = result.lastSelectedFace();
            
            return result;
        }
        
        SelectionResult Selection::deselectFaces(const BrushFaceList& faces) {
            assert(m_map != NULL);
            
            SelectionResult result;
            if (!faces.empty())
                each(faces.begin(),
                     faces.end(),
                     SetSelection(false, result),
                     MatchAll());
            return result;
        }

        SelectionResult Selection::deselectAll() {
            assert(m_map != NULL);
            
            SelectionResult result;
            deselectAllObjects(result);
            deselectAllFaces(result);
            return result;
        }

        BrushFace* Selection::lastSelectedFace() const {
            if (m_map == NULL)
                return NULL;
            return m_lastSelectedFace;
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
    }
}
