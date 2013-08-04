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
            
            inline void operator()(Brush* brush, BrushFace* face) {
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
            
            inline void operator()(Brush* brush, BrushFace* face) const {
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
            eachObject(*m_map, collect, MatchSelectedFilter());
            
            ObjectList result;
            VectorUtils::concatenate(collect.entities, collect.brushes, result);
            return result;
        }
        
        EntityList Selection::selectedEntities() const {
            assert(m_map != NULL);
            
            Collect collect;
            eachEntity(*m_map, collect, MatchSelectedFilter());
            return collect.entities;
        }
        
        EntityList Selection::unselectedEntities() const {
            assert(m_map != NULL);
            
            Collect collect;
            eachEntity(*m_map, collect, MatchUnselectedFilter());
            return collect.entities;
        }

        BrushList Selection::selectedBrushes() const {
            assert(m_map != NULL);
            
            Collect collect;
            eachBrush(*m_map, collect, MatchSelectedFilter());
            return collect.brushes;
        }
        
        BrushList Selection::unselectedBrushes() const {
            assert(m_map != NULL);
            
            Collect collect;
            eachBrush(*m_map, collect, MatchUnselectedFilter());
            return collect.brushes;
        }

        BrushFaceList Selection::selectedFaces() const {
            assert(m_map != NULL);
            
            Collect collect;
            eachFace(*m_map, collect, MatchSelectedFilter());
            return collect.faces;
        }
        
        SelectionResult Selection::selectObjects(const ObjectList& objects) {
            assert(m_map != NULL);

            SelectionResult result;
            if (!objects.empty()) {
                deselectAllFaces(result);
                eachObject(objects, SetSelection(true, result), MatchAllFilter());
            }
            return result;
        }
        
        SelectionResult Selection::deselectObjects(const ObjectList& objects) {
            assert(m_map != NULL);
            SelectionResult result;
            if (!objects.empty())
                eachObject(objects, SetSelection(false, result), MatchAllFilter());
            return result;
        }
        
        SelectionResult Selection::selectAllObjects() {
            assert(m_map != NULL);
            
            SelectionResult result;
            eachObject(*m_map, SetSelection(true, result), MatchUnselectedFilter());
            return result;
        }
        
        SelectionResult Selection::selectFaces(const BrushFaceList& faces) {
            assert(m_map != NULL);
            
            SelectionResult result;
            if (!faces.empty()) {
                deselectAllObjects(result);
                eachFace(faces, SetSelection(true, result), MatchAllFilter());
            }
            return result;
        }
        
        SelectionResult Selection::deselectFaces(const BrushFaceList& faces) {
            assert(m_map != NULL);
            
            SelectionResult result;
            if (!faces.empty())
                eachFace(faces, SetSelection(false, result), MatchAllFilter());
            return result;
        }

        SelectionResult Selection::deselectAll() {
            assert(m_map != NULL);
            
            SelectionResult result;
            deselectAllObjects(result);
            deselectAllFaces(result);
            return result;
        }

        void Selection::deselectAllObjects(SelectionResult& result) {
            assert(m_map != NULL);
            
            eachObject(*m_map, SetSelection(false, result), MatchSelectedObjectsFilter());
        }
        
        void Selection::deselectAllFaces(SelectionResult& result) {
            assert(m_map != NULL);
            
            eachFace(*m_map, SetSelection(false, result), MatchSelectedFacesFilter());
        }
    }
}
