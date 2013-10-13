/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "Entity.h"

#include "CollectionUtils.h"
#include "Assets/EntityDefinition.h"
#include "Assets/ModelDefinition.h"
#include "IO/Path.h"
#include "Model/Brush.h"
#include "Model/ModelUtils.h"

namespace TrenchBroom {
    namespace Model {
        EntitySnapshot::EntitySnapshot(Entity& entity) :
        m_entity(&entity),
        m_properties(m_entity->properties()) {}

        void EntitySnapshot::restore() {
            m_entity->setProperties(m_properties);
        }

        const String Entity::DefaultPropertyValue = "";
        const Hit::HitType Entity::EntityHit = Hit::freeHitType();
        
        Entity::~Entity() {
            VectorUtils::clearAndDelete(m_brushes);
        }

        Entity* Entity::clone(const BBox3& worldBounds) const {
            return static_cast<Entity*>(doClone(worldBounds));
        }

        EntitySnapshot Entity::takeSnapshot() {
            return EntitySnapshot(*this);
        }

        BBox3 Entity::bounds() const {
            const Assets::EntityDefinition* def = definition();
            if (def != NULL && def->type() == Assets::EntityDefinition::PointEntity) {
                BBox3 bounds = static_cast<const Assets::PointEntityDefinition*>(def)->bounds();
                bounds.translate(origin());
                return bounds;
            }
            if (m_brushes.empty())
                return BBox3(-8.0, +8.0);
            BBox3 bounds = m_brushes[0]->bounds();
            for (size_t i = 1; i < m_brushes.size(); ++i)
                bounds.mergeWith(m_brushes[i]->bounds());
            return bounds;
        }

        void Entity::pick(const Ray3& ray, PickResult& result) {
            const BBox3 myBounds = bounds();
            if (!myBounds.contains(ray.origin)) {
                const FloatType distance = myBounds.intersectWithRay(ray);
                if (!Math::isnan(distance)) {
                    const Vec3 hitPoint = ray.pointAtDistance(distance);
                    Hit hit(EntityHit, distance, hitPoint, this);
                    result.addHit(hit);
                }
            }
        }

        Assets::EntityDefinition* Entity::definition() const {
            return m_definition;
        }
        
        void Entity::setDefinition(Assets::EntityDefinition* definition) {
            if (m_definition == definition)
                return;
            if (m_definition != NULL)
                m_definition->decUsageCount();
            m_definition = definition;
            if (m_definition != NULL)
                m_definition->incUsageCount();
        }

        bool Entity::worldspawn() const {
            return classname() == PropertyValues::WorldspawnClassname;
        }

        Assets::ModelSpecification Entity::modelSpecification() const {
            if (m_definition == NULL || m_definition->type() != Assets::EntityDefinition::PointEntity)
                return Assets::ModelSpecification();
            Assets::PointEntityDefinition* pointDefinition = static_cast<Assets::PointEntityDefinition*>(m_definition);
            return pointDefinition->model(m_properties);
        }

        Assets::EntityModel* Entity::model() const {
            return m_model;
        }
        
        void Entity::setModel(Assets::EntityModel* model) {
            m_model = model;
        }
        
        const EntityProperty::List& Entity::properties() const {
            return m_properties.properties();
        }
        
        void Entity::setProperties(const EntityProperty::List& properties) {
            m_properties.setProperties(properties);
        }

        bool Entity::hasProperty(const PropertyKey& key) const {
            return m_properties.hasProperty(key);
        }

        const PropertyValue& Entity::property(const PropertyKey& key, const PropertyValue& defaultValue) const {
            const PropertyValue* value = m_properties.property(key);
            if (value == NULL)
                return defaultValue;
            return *value;
        }
        
        void Entity::addOrUpdateProperty(const PropertyKey& key, const PropertyValue& value) {
            m_properties.addOrUpdateProperty(key, value);
        }

        void Entity::renameProperty(const PropertyKey& key, const PropertyKey& newKey) {
            m_properties.renameProperty(key, newKey);
        }

        void Entity::removeProperty(const PropertyKey& key) {
            m_properties.removeProperty(key);
        }

        const PropertyValue& Entity::classname(const PropertyValue& defaultClassname) const {
            return property(PropertyKeys::Classname, defaultClassname);
        }

        Vec3 Entity::origin() const {
            const PropertyValue* value = m_properties.property(PropertyKeys::Origin);
            if (value == NULL)
                return Vec3();
            return Vec3(*value);
        }

        Quatf Entity::rotation() const {
            return Quatf();
        }

        const BrushList& Entity::brushes() const {
            return m_brushes;
        }

        void Entity::addBrush(Brush* brush) {
            assert(brush->parent() == NULL);
            m_brushes.push_back(brush);
            brush->setParent(this);
        }

        void Entity::removeBrush(Brush* brush) {
            assert(brush->parent() == this);
            VectorUtils::remove(m_brushes, brush);
            brush->setParent(NULL);
        }

        Brush* Entity::findBrushByFilePosition(const size_t position) const {
            if (!containsLine(position))
                return NULL;
            BrushList::const_iterator it = Model::find(m_brushes.begin(), m_brushes.end(), MatchObjectByFilePosition(position));
            if (it == m_brushes.end())
                return NULL;
            return *it;
        }

        bool Entity::doContains(const Object& object) const {
            return object.containedBy(*this);
        }
        
        bool Entity::doContains(const Entity& entity) const {
            return bounds().contains(entity.bounds());
        }
        
        bool Entity::doContains(const Brush& brush) const {
            return brush.containedBy(*this);
        }

        bool Entity::doContainedBy(const Object& object) const {
            return object.contains(*this);
        }
        
        bool Entity::doContainedBy(const Entity& entity) const {
            return entity.contains(*this);
        }
        
        bool Entity::doContainedBy(const Brush& brush) const {
            return brush.contains(*this);
        }

        bool Entity::doIntersects(const Object& object) const {
            return object.intersects(*this);
        }
        
        bool Entity::doIntersects(const Entity& entity) const {
            return bounds().intersects(entity.bounds());
        }
        
        bool Entity::doIntersects(const Brush& brush) const {
            return brush.intersects(*this);
        }

        Entity::Entity() :
        Object(OTEntity),
        m_definition(NULL),
        m_model(NULL) {}
        
        void Entity::doTransform(const Mat4x4& transformation, const bool lockTextures, const BBox3& worldBounds) {
        }
    }
}
