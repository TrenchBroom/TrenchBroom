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

#include "ModelUtils.h"

#include "Assets/EntityDefinition.h"
#include "Assets/PropertyDefinition.h"
#include "Model/Map.h"
#include "Model/Entity.h"
#include "Model/BrushFace.h"
#include "Model/Brush.h"

namespace TrenchBroom {
    namespace Model {
        const Assets::PropertyDefinition* safeGetPropertyDefinition(const PropertyKey& key, const Entity* entity);

        Assets::EntityDefinition* selectEntityDefinition(const EntityList& entities) {
            Assets::EntityDefinition* definition = NULL;
            
            EntityList::const_iterator entityIt, entityEnd;
            for (entityIt = entities.begin(), entityEnd = entities.end(); entityIt != entityEnd; ++entityIt) {
                Entity* entity = *entityIt;
                if (definition == NULL) {
                    definition = entity->definition();
                } else if (definition != entity->definition()) {
                    definition = NULL;
                    break;
                }
            }
            
            return definition;
        }

        const Assets::PropertyDefinition* safeGetPropertyDefinition(const PropertyKey& key, const Entity* entity) {
            const Assets::EntityDefinition* definition = entity->definition();
            if (definition == NULL)
                return NULL;
            
            const Assets::PropertyDefinition* propDefinition = definition->propertyDefinition(key);
            if (propDefinition == NULL)
                return NULL;

            return propDefinition;
        }
        
        const Assets::PropertyDefinition* selectPropertyDefinition(const PropertyKey& key, const EntityList& entities) {
            EntityList::const_iterator it = entities.begin();
            EntityList::const_iterator end = entities.end();
            if (it == end)
                return NULL;
            
            const Entity* entity = *it;
            const Assets::PropertyDefinition* definition = safeGetPropertyDefinition(key, entity);
            if (definition == NULL)
                return NULL;
            
            while (++it != end) {
                entity = *it;
                const Assets::PropertyDefinition* currentDefinition = safeGetPropertyDefinition(key, entity);
                if (currentDefinition == NULL)
                    return NULL;
                
                if (!definition->equals(currentDefinition))
                    return NULL;
            }
            
            return definition;
        }

        PropertyValue selectPropertyValue(const PropertyKey& key, const EntityList& entities) {
            EntityList::const_iterator it = entities.begin();
            EntityList::const_iterator end = entities.end();
            if (it == end)
                return "";
            
            const Entity* entity = *it;
            if (!entity->hasProperty(key))
                return "";
            
            const PropertyValue& value = entity->property(key);
            while (++it != end) {
                entity = *it;
                if (!entity->hasProperty(key))
                    return "";
                if (value != entity->property(key))
                    return "";
            }
            return value;
        }

        Brush* createBrushFromBounds(const Map& map, const BBox3& worldBounds, const BBox3& brushBounds, const String& textureName) {
            const Vec3 size = brushBounds.size();
            const Vec3 x = Vec3(size.x(), 0.0, 0.0);
            const Vec3 y = Vec3(0.0, size.y(), 0.0);
            const Vec3 z = Vec3(0.0, 0.0, size.z());
            
            // east, west, front, back, top, bottom
            BrushFaceList faces(6);
            faces[0] = map.createFace(brushBounds.min, brushBounds.min + y, brushBounds.min + z, textureName);
            faces[1] = map.createFace(brushBounds.max, brushBounds.max - z, brushBounds.max - y, textureName);
            faces[2] = map.createFace(brushBounds.min, brushBounds.min + z, brushBounds.min + x, textureName);
            faces[3] = map.createFace(brushBounds.max, brushBounds.max - x, brushBounds.max - z, textureName);
            faces[4] = map.createFace(brushBounds.max, brushBounds.max - y, brushBounds.max - x, textureName);
            faces[5] = map.createFace(brushBounds.min, brushBounds.min + x, brushBounds.min + y, textureName);
            
            return map.createBrush(worldBounds, faces);
        }

        BrushList mergeEntityBrushesMap(const EntityBrushesMap& map) {
            BrushList result;
            EntityBrushesMap::const_iterator it, end;
            for (it = map.begin(), end = map.end(); it != end; ++it) {
                const BrushList& entityBrushes = it->second;
                result.insert(result.end(), entityBrushes.begin(), entityBrushes.end());
            }
            return result;
        }

        ObjectParentList makeObjectParentList(const EntityBrushesMap& map) {
            ObjectParentList result;
            
            EntityBrushesMap::const_iterator eIt, eEnd;
            for (eIt = map.begin(), eEnd = map.end(); eIt != eEnd; ++eIt) {
                Entity* entity = eIt->first;
                const BrushList& brushes = eIt->second;
                
                BrushList::const_iterator bIt, bEnd;
                for (bIt = brushes.begin(), bEnd = brushes.end(); bIt != bEnd; ++bIt) {
                    Brush* brush = *bIt;
                    result.push_back(ObjectParentPair(brush, entity));
                }
            }
            
            return result;
        }

        ObjectParentList makeObjectParentList(const ObjectList& list) {
            ObjectParentList result;
            result.reserve(list.size());
            
            ObjectList::const_iterator it, end;
            for (it = list.begin(), end = list.end(); it != end; ++it)
                result.push_back(ObjectParentPair(*it));
            return result;
        }

        ObjectParentList makeObjectParentList(const EntityList& list) {
            ObjectParentList result;
            result.reserve(list.size());
            
            EntityList::const_iterator it, end;
            for (it = list.begin(), end = list.end(); it != end; ++it)
                result.push_back(ObjectParentPair(*it));
            return result;
        }
        
        ObjectParentList makeObjectParentList(const BrushList& list) {
            ObjectParentList result;
            result.reserve(list.size());
            
            BrushList::const_iterator it, end;
            for (it = list.begin(), end = list.end(); it != end; ++it)
                result.push_back(ObjectParentPair(*it));
            return result;
        }

        ObjectParentList makeObjectParentList(const BrushList& list, Entity* parent) {
            ObjectParentList result;
            result.reserve(list.size());
            
            BrushList::const_iterator it, end;
            for (it = list.begin(), end = list.end(); it != end; ++it)
                result.push_back(ObjectParentPair(*it, parent));
            return result;
        }

        ObjectList makeChildList(const ObjectParentList& list) {
            ObjectList result;
            result.reserve(list.size());
            
            ObjectParentList::const_iterator it, end;
            for (it = list.begin(), end = list.end(); it != end; ++it) {
                const ObjectParentPair& pair = *it;
                result.push_back(pair.object);
            }
            return result;
        }

        ObjectList makeParentList(const ObjectParentList& list) {
            ObjectSet set;
            ObjectList result;
            result.reserve(list.size());
            
            ObjectParentList::const_iterator it, end;
            for (it = list.begin(), end = list.end(); it != end; ++it) {
                const ObjectParentPair& pair = *it;
                if (pair.parent != NULL && set.insert(pair.parent).second)
                    result.push_back(pair.parent);
            }
            return result;
        }

        ObjectList makeParentList(const ObjectList& list) {
            ObjectSet set;
            ObjectList result;
            result.reserve(list.size());
            
            ObjectList::const_iterator it, end;
            for (it = list.begin(), end = list.end(); it != end; ++it) {
                Object* object = *it;
                if (object->type() == Object::Type_Brush) {
                    Brush* brush = static_cast<Brush*>(object);
                    Entity* entity = brush->parent();
                    if (entity != NULL && set.insert(entity).second)
                        result.push_back(entity);
                }
            }
            return result;
        }

        ObjectList makeParentList(const BrushList& list) {
            ObjectSet set;
            ObjectList result;
            result.reserve(list.size());
            
            BrushList::const_iterator it, end;
            for (it = list.begin(), end = list.end(); it != end; ++it) {
                const ObjectParentPair& pair = *it;
                if (pair.parent != NULL && set.insert(pair.parent).second)
                    result.push_back(pair.parent);
            }
            return result;
        }

        void makeParentChildLists(const ObjectParentList& list, ObjectList& parents, ObjectList& children) {
            ObjectSet parentSet;
            parents.reserve(parents.size() + list.size());
            children.reserve(children.size() + list.size());

            ObjectParentList::const_iterator it, end;
            for (it = list.begin(), end = list.end(); it != end; ++it) {
                const ObjectParentPair& pair = *it;
                if (pair.parent != NULL && parentSet.insert(pair.parent).second)
                    parents.push_back(pair.parent);
                children.push_back(pair.object);
            }
        }

        void makeParentChildLists(const BrushList& list, ObjectList& parents, ObjectList& children) {
            ObjectSet parentSet;
            parents.reserve(parents.size() + list.size());
            children.reserve(children.size() + list.size());
            
            BrushList::const_iterator it, end;
            for (it = list.begin(), end = list.end(); it != end; ++it) {
                Brush* brush = *it;
                Object* parent = brush->parent();
                if (parent != NULL && parentSet.insert(parent).second)
                    parents.push_back(parent);
                children.push_back(brush);
            }
        }

        /*ObjectList makeParentChildList(const ObjectParentList& list) {
            ObjectList result;
            makeParentChildLists(list, result, result);
            return result;
        }
        
        ObjectList makeParentChildList(const BrushList& list) {
            ObjectList result;
            makeParentChildLists(list, result, result);
            return result;
        }
         */

        ObjectList makeObjectList(const EntityList& list) {
            return VectorUtils::cast<Object*>(list);
        }
        
        ObjectList makeObjectList(const BrushList& list) {
            return VectorUtils::cast<Object*>(list);
        }

        ObjectList makeObjectList(const ObjectParentList& list) {
            ObjectList result;
            result.reserve(list.size());
            
            ObjectParentList::const_iterator it, end;
            for (it = list.begin(), end = list.end(); it != end; ++it)
                result.push_back(it->object);
            return result;
        }

        ObjectChildrenMap makeObjectChildrenMap(const ObjectList& list) {
            ObjectChildrenMap map;
            
            ObjectList::const_iterator it, end;
            for (it = list.begin(), end = list.end(); it != end; ++it) {
                Object* object = *it;
                if (object->type() == Object::Type_Entity) {
                    map[NULL].push_back(object);
                } else if (object->type() == Object::Type_Brush) {
                    Brush* brush = static_cast<Brush*>(object);
                    Entity* entity = brush->parent();
                    map[entity].push_back(brush);
                }
            }
            return map;
        }

        ObjectChildrenMap makeObjectChildrenMap(const ObjectParentList& list) {
            ObjectChildrenMap map;
            
            ObjectParentList::const_iterator it, end;
            for (it = list.begin(), end = list.end(); it != end; ++it) {
                Object* object = it->object;
                Object* parent = it->parent;
                map[parent].push_back(object);
            }
            return map;
        }
        
        void filterEntityList(const EntityList& entities, EntityList& pointEntities, EntityList& brushEntities, EntityList& untypedEntities) {
            EntityList::const_iterator it, end;
            for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                Entity* entity = *it;
                const Assets::EntityDefinition* definition = entity->definition();
                if (definition == NULL) {
                    untypedEntities.push_back(entity);
                } else if (definition->type() == Assets::EntityDefinition::Type_PointEntity) {
                    pointEntities.push_back(entity);
                } else if (definition->type() == Assets::EntityDefinition::Type_BrushEntity) {
                    brushEntities.push_back(entity);
                }
            }
        }

        void filterEntityList(const EntityList& entities, EntityList& emptyEntities, EntityList& brushEntities) {
            EntityList::const_iterator it, end;
            for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                Entity* entity = *it;
                if (entity->brushes().empty())
                    emptyEntities.push_back(entity);
                else
                    brushEntities.push_back(entity);
            }
        }

        EntityList makeEntityList(const ObjectList& objects) {
            EntityList result;
            ObjectList::const_iterator it, end;
            for (it = objects.begin(), end = objects.end(); it != end; ++it) {
                Object* object = *it;
                if (object->type() == Object::Type_Entity) {
                    Entity* entity = static_cast<Entity*>(object);
                    result.push_back(entity);
                }
            }
            return result;
        }

        BrushList makeBrushList(const EntityList& entities) {
            BrushList result;
            EntityList::const_iterator it, end;
            for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                const Entity* entity = *it;
                const BrushList& brushes = entity->brushes();
                VectorUtils::append(result, brushes);
            }
            return result;
        }

        BrushList makeBrushList(const ObjectList& objects) {
            BrushList result;
            ObjectList::const_iterator it, end;
            for (it = objects.begin(), end = objects.end(); it != end; ++it) {
                Object* object = *it;
                if (object->type() == Object::Type_Brush) {
                    Brush* brush = static_cast<Brush*>(object);
                    result.push_back(brush);
                }
            }
            return result;
        }

        bool MatchAll::operator()(const ObjectParentPair& pair) const {
            return true;
        }

        bool MatchAll::operator()(const Object* object) const {
            return true;
        }
        
        bool MatchAll::operator()(const Entity* entity) const {
            return true;
        }
        
        bool MatchAll::operator()(const Brush* brush) const {
            return true;
        }
        
        bool MatchAll::operator()(const BrushFace* face) const {
            return true;
        }

        bool MatchAll::operator()(const BrushEdge* edge) const {
            return true;
        }
        
        bool MatchAll::operator()(const BrushVertex* vertex) const {
            return true;
        }

        MatchVisibleObjects::MatchVisibleObjects(const ModelFilter& filter) :
        m_filter(filter) {}
        
        bool MatchVisibleObjects::operator()(const ObjectParentPair& pair) const {
            return m_filter.visible(pair.object);
        }
        
        bool MatchVisibleObjects::operator()(const Object* object) const {
            return m_filter.visible(object);
        }
        
        bool MatchVisibleObjects::operator()(const Entity* entity) const {
            return m_filter.visible(entity);
        }
        
        bool MatchVisibleObjects::operator()(const Brush* brush) const {
            return m_filter.visible(brush);
        }
        
        bool MatchVisibleObjects::operator()(const BrushFace* face) const {
            return m_filter.visible(face);
        }
        
        bool MatchVisibleObjects::operator()(const BrushEdge* edge) const {
            return true;
        }
        
        bool MatchVisibleObjects::operator()(const BrushVertex* vertex) const {
            return true;
        }

        MatchObjectByType::MatchObjectByType(const Object::Type type) :
        m_type(type) {}
        
        bool MatchObjectByType::operator()(const Object* object) const {
            return object->type() == m_type;
        }

        MatchObjectByFilePosition::MatchObjectByFilePosition(const size_t position) :
        m_position(position) {}
        
        bool MatchObjectByFilePosition::operator()(const Object* object) const {
            return object->containsLine(m_position);
        }

        Transform::Transform(const Mat4x4& transformation, const bool lockTextures, const BBox3& worldBounds) :
        m_transformation(transformation),
        m_lockTextures(lockTextures),
        m_worldBounds(worldBounds) {}
        
        void Transform::operator()(Object* object) const {
            object->transform(m_transformation, m_lockTextures, m_worldBounds);
        }
        
        void Transform::operator()(BrushFace* face) const {
            face->transform(m_transformation, m_lockTextures);
        }

        CheckBounds::CheckBounds(const BBox3& bounds) :
        m_bounds(bounds) {}
        
        bool CheckBounds::operator()(const Pickable* object) const {
            return m_bounds.contains(object->bounds());
        }

        NotifyParent::NotifyParent(Notifier1<Object*>& notifier) :
        m_notifier(notifier) {}
        
        void NotifyParent::operator()(const ObjectParentPair& pair) {
            if (pair.parent != NULL && m_notified.insert(pair.parent).second)
                m_notifier(pair.parent);
        }

        void NotifyParent::operator()(Object* object) {
            if (object->type() == Object::Type_Brush) {
                Object* parent = static_cast<Brush*>(object)->parent();
                if (parent != NULL && m_notified.insert(parent).second)
                    m_notifier(parent);
            }
        }

        NotifyObject::NotifyObject(Notifier1<Object*>& notifier) :
        m_notifier(notifier) {}
        
        void NotifyObject::operator()(Object* object) {
            m_notifier(object);
        }
    }
}
