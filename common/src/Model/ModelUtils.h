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

#ifndef TrenchBroom_ModelUtils_h
#define TrenchBroom_ModelUtils_h

#include "TrenchBroom.h"
#include "VecMath.h"
#include "CastIterator.h"
#include "FilterIterator.h"
#include "Notifier.h"
#include "Model/Entity.h"
#include "Model/ModelFilter.h"
#include "Model/ModelTypes.h"
#include "Model/Object.h"

#include <iterator>

namespace TrenchBroom {
    namespace Assets {
        class EntityDefinition;
        class PropertyDefinition;
    }
    
    namespace Model {
        class BrushEdge;
        class BrushVertex;
        class Map;
        
        Assets::EntityDefinition* selectEntityDefinition(const EntityList& entities);
        const Assets::PropertyDefinition* selectPropertyDefinition(const PropertyKey& key, const EntityList& entities);
        PropertyValue selectPropertyValue(const PropertyKey& key, const EntityList& entities);
        
        Brush* createBrushFromBounds(const Map& map, const BBox3& worldBounds, const BBox3& brushBounds, const String& textureName);
        
        Object* getParent(const Model::Object* object);
        
        BrushList mergeEntityBrushesMap(const EntityBrushesMap& map);
        ObjectParentList makeObjectParentList(const EntityBrushesMap& map);
        ObjectParentList makeObjectParentList(const ObjectList& list);
        ObjectParentList makeObjectParentList(const EntityList& list);
        ObjectParentList makeObjectParentList(const BrushList& list);
        ObjectParentList makeObjectParentList(const BrushList& list, Entity* parent);

        ObjectList makeChildList(const ObjectParentList& list);
        ObjectList makeParentList(const ObjectParentList& list);
        ObjectList makeParentList(const ObjectList& list);
        ObjectList makeParentList(const BrushList& list);
        void makeParentChildLists(const ObjectParentList& list, ObjectList& parents, ObjectList& children);
        void makeParentChildLists(const BrushList& list, ObjectList& parents, ObjectList& children);
        // ObjectList makeParentChildList(const ObjectParentList& list);
        // ObjectList makeParentChildList(const BrushList& list);
        ObjectList makeObjectList(const EntityList& list);
        ObjectList makeObjectList(const BrushList& list);
        ObjectList makeObjectList(const ObjectParentList& list);

        void appendToObjectParentList(ObjectParentList& list, Object* parent, const ObjectList& children);
        
        ObjectChildrenMap makeObjectChildrenMap(const ObjectList& list);
        ObjectChildrenMap makeObjectChildrenMap(const ObjectParentList& list);

        void filterEntityList(const EntityList& entities, EntityList& pointEntities, EntityList& brushEntities, EntityList& untypedEntities);
        void filterEntityList(const EntityList& entities, EntityList& emptyEntities, EntityList& brushEntities);

        template <typename I1, typename I2>
        struct FilterObjects : public ObjectVisitor {
            I1 entityInserter;
            I2 brushInserter;
            
            FilterObjects(I1 i_entityInserter, I2 i_brushInserter) :
            entityInserter(i_entityInserter),
            brushInserter(i_brushInserter) {}
            
            void doVisit(Entity* entity) {
                entityInserter = entity;
            }
            
            void doVisit(Brush* brush) {
                brushInserter = brush;
            }
        };

        void filterObjectList(const Model::ObjectList& objects, EntityList& entities, BrushList& brushes);
        EntityList makeEntityList(const ObjectList& objects);
        BrushList makeBrushList(const ObjectList& objects);
        BrushList makeBrushList(const EntityList& entities);
        
        template <typename I>
        void filterObjects(I cur, const I end, EntityList& entities, BrushList& brushes) {
            filterObjects(cur, end,
                          std::back_inserter(entities),
                          std::back_inserter(brushes));
        }

        template <typename I, typename O1, typename O2>
        void filterObjects(I cur, const I end, O1 entityInserter, O2 brushInserter) {
            FilterObjects<O1,O2> visitor(entityInserter, brushInserter);
            Object::accept(cur, end, visitor);
        }

        struct MatchAll {
            bool operator()(const ObjectParentPair& pair) const;
            bool operator()(const Object* object) const;
            bool operator()(const Entity* entity) const;
            bool operator()(const Brush* brush) const;
            bool operator()(const BrushFace* face) const;
            bool operator()(const BrushEdge* edge) const;
            bool operator()(const BrushVertex* vertex) const;
        };
        
        class MatchVisibleObjects {
        private:
            const ModelFilter& m_filter;
        public:
            MatchVisibleObjects(const ModelFilter& filter);
            
            bool operator()(const ObjectParentPair& pair) const;
            bool operator()(const Object* object) const;
            bool operator()(const Entity* entity) const;
            bool operator()(const Brush* brush) const;
            bool operator()(const BrushFace* face) const;
            bool operator()(const BrushEdge* edge) const;
            bool operator()(const BrushVertex* vertex) const;
        };
        
        struct MatchObjectByFilePosition {
        private:
            size_t m_position;
        public:
            MatchObjectByFilePosition(const size_t position);
            bool operator()(const Object* object) const;
        };
        
        struct Transform {
        private:
            const Mat4x4& m_transformation;
            const bool m_lockTextures;
            const BBox3& m_worldBounds;
        public:
            Transform(const Mat4x4& transformation, bool lockTextures, const BBox3& worldBounds);
            void operator()(Object* object) const;
            void operator()(BrushFace* face) const;
        };
        
        struct CheckBounds {
        private:
            const BBox3& m_bounds;
        public:
            CheckBounds(const BBox3& bounds);
            bool operator()(const Pickable* object) const;
        };

        template <typename Iter, class Operator, class Filter>
        void each(Iter cur, Iter end, const Operator& op, const Filter& filter) {
            while (cur != end) {
                if (filter(*cur))
                    op(*cur);
                ++cur;
            }
        }

        template <typename Iter, class Operator, class Filter>
        void each(Iter cur, Iter end, Operator& op, const Filter& filter) {
            while (cur != end) {
                if (filter(*cur))
                    op(*cur);
                ++cur;
            }
        }
        
        template <typename Iter, class Filter>
        bool each(Iter cur, Iter end, const Filter& filter) {
            while (cur != end) {
                if (!filter(*cur))
                    return false;
                ++cur;
            }
            return true;
        }
        
        template <typename Iter, class Filter>
        bool any(Iter cur, Iter end, const Filter& filter) {
            while (cur != end) {
                if (filter(*cur))
                    return true;
                ++cur;
            }
            return false;
        }
        
        template <typename InputIter, class Filter, typename OutputIter>
        void filter(InputIter cur, InputIter end, const Filter& filter, OutputIter output) {
            while (cur != end) {
                if (filter(*cur))
                    *output++ = *cur;
                ++cur;
            }
        }
        
        template <typename Iter, class Filter>
        Iter find(Iter cur, Iter end, const Filter& filter) {
            while (cur != end) {
                if (filter(*cur))
                    return cur;
                ++cur;
            }
            return end;
        }
    }
}
#endif
