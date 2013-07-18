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
#include "Model/Map.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        struct Collect {
            Entity::List entities;
            Brush::List brushes;
            BrushFace::List faces;
            
            inline void operator()(Entity::Ptr entity) {
                entities.push_back(entity);
            }
            
            inline void operator()(Brush::Ptr brush) {
                brushes.push_back(brush);
            }
            
            inline void operator()(Brush::Ptr brush, BrushFace::Ptr face) {
                faces.push_back(face);
            }
        };
        
        class SetSelection {
        private:
            bool m_select;
        public:
            SetSelection(const bool select) :
            m_select(select) {}
            
            inline void operator()(Entity::Ptr entity) const {
                if (m_select)
                    entity->select();
                else
                    entity->deselect();
            }
            
            inline void operator()(Brush::Ptr brush) const {
                if (m_select)
                    brush->select();
                else
                    brush->deselect();
            }
            
            inline void operator()(Brush::Ptr brush, BrushFace::Ptr face) const {
                if (m_select)
                    face->select();
                else
                    face->deselect();
            }
        };

        struct MatchSelectedObjectsFilter {
            inline bool operator()(Entity::Ptr entity) const {
                return entity->selected();
            }
            
            inline bool operator()(Brush::Ptr brush) const {
                return brush->selected();
            }
        };
        
        struct MatchSelectedFacesFilter {
            inline bool operator()(Brush::Ptr brush, BrushFace::Ptr face) const {
                return face->selected();
            }
        };

        struct MatchSelectedFilter : public MatchSelectedObjectsFilter, public MatchSelectedFacesFilter {};
        
        struct MatchUnselectedFilter {
            inline bool operator()(Entity::Ptr entity) const {
                return !entity->selected();
            }
            
            inline bool operator()(Brush::Ptr brush) const {
                return !brush->selected();
            }
            
            inline bool operator()(Brush::Ptr brush, BrushFace::Ptr face) const {
                return !face->selected();
            }
        };

        Selection::Selection(Map* map) :
        m_map(map) {}

        Object::List Selection::selectedObjects() const {
            assert(m_map != NULL);
            
            Collect collect;
            m_map->eachObject(collect, MatchSelectedFilter());
            
            Object::List result;
            VectorUtils::concatenate(collect.entities, collect.brushes, result);
            return result;
        }
        
        Entity::List Selection::selectedEntities() const {
            assert(m_map != NULL);
            
            Collect collect;
            m_map->eachEntity(collect, MatchSelectedFilter());
            return collect.entities;
        }
        
        Brush::List Selection::selectedBrushes() const {
            assert(m_map != NULL);
            
            Collect collect;
            m_map->eachBrush(collect, MatchSelectedFilter());
            return collect.brushes;
        }
        
        BrushFace::List Selection::selectedFaces() const {
            assert(m_map != NULL);
            
            Collect collect;
            m_map->eachBrushFace(collect, MatchSelectedFilter());
            return collect.faces;
        }
        
        void Selection::selectObjects(const Object::List& objects) {
            assert(m_map != NULL);
            if (objects.empty())
                return;
            
            deselectAllFaces();
            Object::List::const_iterator it, end;
            for (it = objects.begin(), end = objects.end(); it != end; ++it) {
                Object::Ptr object = *it;
                object->select();
            }
        }
        
        void Selection::deselectObjects(const Object::List& objects) {
            assert(m_map != NULL);
            if (objects.empty())
                return;
            
            Object::List::const_iterator it, end;
            for (it = objects.begin(), end = objects.end(); it != end; ++it) {
                Object::Ptr object = *it;
                object->deselect();
            }
        }
        
        void Selection::selectAllObjects() {
            assert(m_map != NULL);
            
            m_map->eachObject(SetSelection(true), MatchUnselectedFilter());
        }
        
        void Selection::selectFaces(const BrushFace::List& faces) {
            assert(m_map != NULL);
            if (faces.empty())
                return;
            
            deselectAllObjects();
            BrushFace::List::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                BrushFace::Ptr face = *it;
                face->select();
            }
        }
        
        void Selection::deselectFaces(const BrushFace::List& faces) {
            assert(m_map != NULL);
            if (faces.empty())
                return;

            BrushFace::List::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                BrushFace::Ptr face = *it;
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
