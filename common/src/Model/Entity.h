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

#ifndef __TrenchBroom__Entity__
#define __TrenchBroom__Entity__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "SharedPointer.h"
#include "Allocator.h"
#include "Hit.h"
#include "Assets/AssetTypes.h"
#include "Assets/EntityDefinition.h"
#include "Model/EntityProperties.h"
#include "Model/ModelTypes.h"
#include "Model/Object.h"

#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class EntityModelManager;
        struct ModelSpecification;
    }
    
    namespace Model {
        class Entity;
        class EntitySnapshot {
        private:
            Entity* m_entity;
            EntityProperty::List m_properties;
        public:
            EntitySnapshot(Entity& entity);
            void restore();
        };
        
        class Entity : public Object {
        public:
            static const Hit::HitType EntityHit;
        private:
            static const String DefaultPropertyValue;
            static const BBox3 DefaultBounds;
            
            Map* m_map;
            Assets::EntityDefinition* m_definition;
            Assets::EntityModel* m_model;
            EntityProperties m_properties;
            BrushList m_brushes;
            
            mutable BBox3 m_bounds;
            mutable bool m_boundsValid;
            bool m_isWorldspawn;
            
            EntityList m_linkSources;
            EntityList m_linkTargets;
            EntityList m_killSources;
            EntityList m_killTargets;
        public:
            virtual ~Entity();
            
            Map* map() const;
            void setMap(Map* map);
            
            Entity* clone(const BBox3& worldBounds) const;
            EntitySnapshot takeSnapshot();
            
            const BBox3& bounds() const;
            void pick(const Ray3& ray, Hits& hits);
            
            Assets::EntityDefinition* definition() const;
            void setDefinition(Assets::EntityDefinition* definition);
            bool worldspawn() const;
            bool pointEntity() const;
            
            Assets::ModelSpecification modelSpecification() const;
            Assets::EntityModel* model() const;
            void setModel(Assets::EntityModel* model);
            
            const EntityProperty::List& properties() const;
            void setProperties(const EntityProperty::List& properties);
            bool hasProperty(const PropertyKey& key) const;
            const PropertyValue& property(const PropertyKey& key, const PropertyValue& defaultValue = DefaultPropertyValue) const;
            
            template <typename T>
            void addOrUpdateProperty(const PropertyKey& key, const T& value) {
                const Assets::PropertyDefinition* definition = Assets::EntityDefinition::safeGetPropertyDefinition(m_definition, key);
                const PropertyValue* oldValue = m_properties.property(key);
                if (oldValue != NULL) {
                    removePropertyFromIndex(key, *oldValue);
                    removeLinks(key, *oldValue);
                }
                
                const EntityProperty& newProperty = m_properties.addOrUpdateProperty(key, value, definition);
                addPropertyToIndex(newProperty.key(), newProperty.value());
                addLinks(newProperty.key(), newProperty.value());
                invalidateBounds();
                setIsWorldspawn();
            }
            
            template <typename T, size_t S>
            void addOrUpdateProperty(const PropertyKey& key, const Vec<T,S> value) {
                const Assets::PropertyDefinition* definition = Assets::EntityDefinition::safeGetPropertyDefinition(m_definition, key);
                const PropertyValue* oldValue = m_properties.property(key);
                if (oldValue != NULL) {
                    removePropertyFromIndex(key, *oldValue);
                    removeLinks(key, *oldValue);
                }

                const EntityProperty& newProperty = m_properties.addOrUpdateProperty(key, value.asString(), definition);
                addPropertyToIndex(newProperty.key(), newProperty.value());
                addLinks(newProperty.key(), newProperty.value());
                invalidateBounds();
                setIsWorldspawn();
            }
            
            void renameProperty(const PropertyKey& key, const PropertyKey& newKey);
            void removeProperty(const PropertyKey& key);
            
            const PropertyValue& classname(const PropertyValue& defaultClassname = PropertyValues::NoClassname) const;
            Vec3 origin() const;
            virtual Quat3 rotation() const;
            
            const BrushList& brushes() const;
            
            void addBrushes(const BrushList& brushes);
            void addBrush(Brush* brush);
            
            void removeBrush(Brush* brush);
            void removeAllBrushes();
            
            Brush* findBrushByFilePosition(const size_t position) const;
            void childBrushChanged();
            
            const EntityList& linkSources() const;
            const EntityList& linkTargets() const;
            const EntityList& killSources() const;
            const EntityList& killTargets() const;

            bool hasMissingSources() const;
            PropertyKeyList findMissingLinkTargets() const;
            PropertyKeyList findMissingKillTargets() const;
        private:
            void updatePropertyIndex(const EntityProperty::List& newProperties);
            void addPropertyToIndex(const PropertyKey& key, const PropertyValue& value);
            void removePropertyFromIndex(const PropertyKey& key, const PropertyValue& value);
            void updatePropertyIndex(const PropertyKey& oldKey, const PropertyValue& oldValue, const PropertyKey& newKey, const PropertyValue& newValue);
            
            void addLinks(const PropertyKey& key, const PropertyValue& value);
            void removeLinks(const PropertyKey& key, const PropertyValue& value);
            void updateLinks(const PropertyKey& oldKey, const PropertyValue& oldValue, const PropertyKey& newKey, const PropertyValue& newValue);
            
            void addLinkTargets(const PropertyValue& targetname);
            void addKillTargets(const PropertyValue& targetname);
            
            void removeLinkTargets(const PropertyValue& targetname);
            void removeKillTargets(const PropertyValue& targetname);
            
            void addAllLinkSources(const PropertyValue& targetname);
            void addAllLinkTargets();
            void addAllKillSources(const PropertyValue& targetname);
            void addAllKillTargets();
            
            void removeAllLinkSources();
            void removeAllLinkTargets();
            void removeAllKillSources();
            void removeAllKillTargets();
            
            void findMissingTargets(const PropertyKey& prefix, PropertyKeyList& result) const;
            
            void doTransform(const Mat4x4& transformation, const bool lockTextures, const BBox3& worldBounds);

            void setOrigin(const Vec3& origin);
            virtual void applyRotation(const Mat4x4& rotation);

            void doAddToLayer(Layer* layer);
            void doRemoveFromLayer(Layer* layer);

            bool doContains(const Object& object) const;
            bool doContains(const Entity& entity) const;
            bool doContains(const Brush& brush) const;
            bool doContainedBy(const Object& object) const;
            bool doContainedBy(const Entity& entity) const;
            bool doContainedBy(const Brush& brush) const;
            bool doIntersects(const Object& object) const;
            bool doIntersects(const Entity& entity) const;
            bool doIntersects(const Brush& brush) const;
            void doVisit(ObjectVisitor& visitor);
            
            void invalidateBounds();
            void validateBounds() const;
            void setIsWorldspawn();
        protected:
            void addLinkSource(Entity* entity);
            void addLinkTarget(Entity* entity);
            void addKillSource(Entity* entity);
            void addKillTarget(Entity* entity);
            
            void removeLinkSource(Entity* entity);
            void removeLinkTarget(Entity* entity);
            void removeKillSource(Entity* entity);
            void removeKillTarget(Entity* entity);

            Entity();
        private:
            Entity(const Entity& other);
            Entity& operator=(const Entity& other);
        };
        
        template <class RotationPolicy>
        class ConfigurableEntity : public Entity, public Allocator<ConfigurableEntity<RotationPolicy> > {
        public:
            ConfigurableEntity() :
            Entity() {}
            
            Quat3 rotation() const {
                return RotationPolicy::getRotation(*this);
            }
            
            void applyRotation(const Mat4x4& rotation) {
                RotationPolicy::applyRotation(*this, rotation);
            }
        private:
            Object* doClone(const BBox3& worldBounds) const {
                ConfigurableEntity<RotationPolicy>* entity = new ConfigurableEntity<RotationPolicy>();
                entity->setProperties(properties());
                return entity;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__Entity__) */
