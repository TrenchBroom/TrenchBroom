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
            
            inline void operator()(Brush* brush, BrushFace* face) {
                faces.push_back(face);
            }
        };
        
        class SetSelection {
        private:
            bool m_select;
        public:
            SetSelection(const bool select) :
            m_select(select) {}
            
            inline void operator()(Entity* entity) const {
                if (m_select)
                    entity->select();
                else
                    entity->deselect();
            }
            
            inline void operator()(Brush* brush) const {
                if (m_select)
                    brush->select();
                else
                    brush->deselect();
            }
            
            inline void operator()(Brush* brush, BrushFace* face) const {
                if (m_select)
                    face->select();
                else
                    face->deselect();
            }
        };

        struct MatchSelectedObjectsFilter {
            inline bool operator()(Entity* entity) const {
                return entity->selected();
            }
            
            inline bool operator()(Brush* brush) const {
                return brush->selected();
            }
        };
        
        struct MatchSelectedFacesFilter {
            inline bool operator()(Brush* brush, BrushFace* face) const {
                return face->selected();
            }
        };

        struct MatchSelectedFilter  {
            inline bool operator()(Entity* entity) const {
                return entity->selected();
            }
            
            inline bool operator()(Brush* brush) const {
                return brush->selected();
            }

            inline bool operator()(Brush* brush, BrushFace* face) const {
                return face->selected();
            }
        };
        
        struct MatchUnselectedFilter {
            inline bool operator()(Entity* entity) const {
                return !entity->selected();
            }
            
            inline bool operator()(Brush* brush) const {
                return !brush->selected();
            }
            
            inline bool operator()(Brush* brush, BrushFace* face) const {
                return !face->selected();
            }
        };

        Selection::Selection(Map* map) :
        m_map(map) {}

        ObjectList Selection::selectedObjects() const {
            assert(m_map != NULL);
            
            Collect collect;
            m_map->eachObject(collect, MatchSelectedFilter());
            
            ObjectList result;
            VectorUtils::concatenate(collect.entities, collect.brushes, result);
            return result;
        }
        
        EntityList Selection::selectedEntities() const {
            assert(m_map != NULL);
            
            Collect collect;
            m_map->eachEntity(collect, MatchSelectedFilter());
            return collect.entities;
        }
        
        BrushList Selection::selectedBrushes() const {
            assert(m_map != NULL);
            
            Collect collect;
            m_map->eachBrush(collect, MatchSelectedFilter());
            return collect.brushes;
        }
        
        BrushFaceList Selection::selectedFaces() const {
            assert(m_map != NULL);
            
            Collect collect;
            m_map->eachBrushFace(collect, MatchSelectedFilter());
            return collect.faces;
        }
        
        void Selection::selectObjects(const ObjectList& objects) {
            assert(m_map != NULL);
            if (objects.empty())
                return;
            
            deselectAllFaces();
            ObjectList::const_iterator it, end;
            for (it = objects.begin(), end = objects.end(); it != end; ++it) {
                Object* object = *it;
                object->select();
            }
        }
        
        void Selection::deselectObjects(const ObjectList& objects) {
            assert(m_map != NULL);
            if (objects.empty())
                return;
            
            ObjectList::const_iterator it, end;
            for (it = objects.begin(), end = objects.end(); it != end; ++it) {
                Object* object = *it;
                object->deselect();
            }
        }
        
        void Selection::selectAllObjects() {
            assert(m_map != NULL);
            
            m_map->eachObject(SetSelection(true), MatchUnselectedFilter());
        }
        
        void Selection::selectFaces(const BrushFaceList& faces) {
            assert(m_map != NULL);
            if (faces.empty())
                return;
            
            deselectAllObjects();
            BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                BrushFace* face = *it;
                face->select();
            }
        }
        
        void Selection::deselectFaces(const BrushFaceList& faces) {
            assert(m_map != NULL);
            if (faces.empty())
                return;

            BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                BrushFace* face = *it;
                face->deselect();
            }
        }

        void Selection::deselectAll() {
            assert(m_map != NULL);
            
            deselectAllObjects();
            deselectAllFaces();
        }

        void Selection::deselectAllObjects() {
            assert(m_map != NULL);
            
            m_map->eachObject(SetSelection(false), MatchSelectedObjectsFilter());
        }
        
        void Selection::deselectAllFaces() {
            assert(m_map != NULL);
            
            m_map->eachBrushFace(SetSelection(false), MatchSelectedFacesFilter());
        }
    }
}
