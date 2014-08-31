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

#include "Map.h"

#include "CollectionUtils.h"
#include "Model/Brush.h"
#include "Model/Layer.h"
#include "Model/ModelUtils.h"
#include "Model/Object.h"

namespace TrenchBroom {
    namespace Model {
        Map::Map(const ModelFactory& factory) :
        m_factory(factory),
        m_worldspawn(NULL) {
            addLayer(createLayer("Default Layer"));
        }
        
        Map::~Map() {
            m_worldspawn = NULL;
            VectorUtils::clearAndDelete(m_layers);
            VectorUtils::clearAndDelete(m_entities);
        }

        MapFormat::Type Map::format() const {
            return m_factory.format();
        }

        Layer* Map::createLayer(const String& name) const {
            return new Layer(name);
        }
        
        void Map::addLayer(Layer* layer) {
            assert(layer != NULL);
            m_layers.push_back(layer);
        }
        
        bool Map::canRemoveLayer(const Layer* layer) const {
            assert(layer != NULL);
            return layer != defaultLayer();
        }

        void Map::removeLayer(Layer* layer) {
            assert(canRemoveLayer(layer));
            assert(layer->objects().empty());
            
            const bool found = VectorUtils::erase(m_layers, layer);
            assert(found);
        }
        
        Layer* Map::defaultLayer() const {
            return m_layers.front();
        }
        
        const LayerList& Map::layers() const {
            return m_layers;
        }

        Entity* Map::createEntity() const {
            return m_factory.createEntity();
        }
        
        Brush* Map::createBrush(const BBox3& worldBounds, const BrushFaceList& faces) const {
            return m_factory.createBrush(worldBounds, faces);
        }
        
        BrushFace* Map::createFace(const Vec3& point0, const Vec3& point1, const Vec3& point2, const String& textureName) const {
            return m_factory.createFace(point0, point1, point2, textureName);
        }

        const EntityList& Map::entities() const {
            return m_entities;
        }

        void Map::addEntity(Entity* entity) {
            addEntityPropertiesToIndex(entity);
            m_entities.push_back(entity);
            entity->setMap(this);
        }

        void Map::removeEntity(Entity* entity) {
            VectorUtils::erase(m_entities, entity);
            entity->setMap(NULL);
            removeEntityPropertiesFromIndex(entity);
        }

        Entity* Map::worldspawn() const {
            if (m_worldspawn == NULL)
                m_worldspawn = findWorldspawn();
            return m_worldspawn;
        }
        
        Object* Map::findObjectByFilePosition(const size_t position) const {
            EntityList::const_iterator it = Model::find(m_entities.begin(), m_entities.end(), MatchObjectByFilePosition(position));
            if (it == m_entities.end())
                return NULL;
            Entity* entity = *it;
            Brush* brush = entity->findBrushByFilePosition(position);
            if (brush != NULL)
                return brush;
            return entity;
        }

        void Map::addEntityPropertiesToIndex(Entity* entity) {
            m_entityPropertyIndex.addEntity(entity);
        }
        
        void Map::removeEntityPropertiesFromIndex(Entity* entity) {
            m_entityPropertyIndex.removeEntity(entity);
        }

        void Map::addEntityPropertyToIndex(Entity* entity, const PropertyKey& key, const PropertyValue& value) {
            m_entityPropertyIndex.addEntityProperty(entity, key, value);
        }
        
        void Map::removeEntityPropertyFromIndex(Entity* entity, const PropertyKey& key, const PropertyValue& value) {
            m_entityPropertyIndex.removeEntityProperty(entity, key, value);
        }

        EntityList Map::findEntitiesWithProperty(const PropertyKey& key, const PropertyValue& value) const {
            return m_entityPropertyIndex.findEntities(EntityPropertyQuery::exact(key),
                                                      EntityPropertyQuery::exact(value));
        }
        
        EntityList Map::findEntitiesWithNumberedProperty(const PropertyKey& prefix, const PropertyValue& value) const {
            return m_entityPropertyIndex.findEntities(EntityPropertyQuery::numbered(prefix),
                                                      EntityPropertyQuery::exact(value));
        }

        const BrushList Map::brushes() const {
            BrushList brushes;
            EntityList::const_iterator it, end;
            for (it = m_entities.begin(), end = m_entities.end(); it != end; ++it) {
                const Entity* entity = *it;
                const Model::BrushList& entityBrushes = entity->brushes();
                brushes.insert(brushes.end(), entityBrushes.begin(), entityBrushes.end());
            }
            return brushes;
        }

        Entity* Map::findWorldspawn() const {
            EntityList::const_iterator it, end;
            for (it = m_entities.begin(), end = m_entities.end(); it != end; ++it) {
                Entity* entity = *it;
                if (entity->classname() == PropertyValues::WorldspawnClassname)
                    return entity;
            }
            return NULL;
        }
    }
}
