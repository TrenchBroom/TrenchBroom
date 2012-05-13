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

#include "Entity.h"
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <sstream>
#include <cassert>
#include <algorithm>
#include "Model/Map/Picker.h"
#include "Utilities/Console.h"

using namespace std;

namespace TrenchBroom {
    namespace Model {
        void Entity::init() {
            m_map = NULL;
            m_filePosition = -1;
            m_selected = false;
            m_vboBlock = NULL;
            m_origin = Null3f;
            m_angle = 0;
            invalidateGeometry();
        }

        void Entity::validateGeometry() const {
            assert(!m_geometryValid);

            m_bounds.min = m_bounds.max = Null3f;
            m_maxBounds.min = m_maxBounds.max = Null3f;
            if (m_entityDefinition == NULL || m_entityDefinition->type == TB_EDT_BRUSH) {
                if (!m_brushes.empty()) {
                    m_bounds = m_brushes[0]->bounds();
                    for (unsigned int i = 1; i < m_brushes.size(); i++)
                        m_bounds += m_brushes[i]->bounds();
                }
            } else if (m_entityDefinition->type == TB_EDT_POINT) {
                m_bounds = m_entityDefinition->bounds.translate(m_origin);
            }

            m_center = m_bounds.center();
            m_maxBounds = m_bounds.maxBounds();
            m_geometryValid = true;
        }

        void Entity::invalidateGeometry() {
            m_geometryValid = false;
        }

        Entity::Entity() : MapObject() {
            init();
        }

        Entity::Entity(const map<string, string> properties) : MapObject() {
            init();
            m_properties = properties;
            map<string, string>::iterator it;
            if ((it = m_properties.find(AngleKey)) != m_properties.end())
                m_angle = static_cast<float>(atof(it->second.c_str()));
            if ((it = m_properties.find(OriginKey)) != m_properties.end())
                m_origin = Vec3f(it->second);
            invalidateGeometry();
        }

        Entity::~Entity() {
            while (!m_brushes.empty()) delete m_brushes.back(), m_brushes.pop_back();

        }

        EMapObjectType Entity::objectType() const {
            return TB_MT_ENTITY;
        }

        const EntityDefinitionPtr Entity::entityDefinition() const {
            return m_entityDefinition;
        }

        void Entity::setEntityDefinition(EntityDefinitionPtr entityDefinition) {
            if (m_entityDefinition != NULL)
                m_entityDefinition->usageCount--;
            m_entityDefinition = entityDefinition;
            if (m_entityDefinition != NULL)
                m_entityDefinition->usageCount++;
            invalidateGeometry();
        }

        const Vec3f& Entity::center() const {
            if (!m_geometryValid) validateGeometry();
            return m_center;
        }

        const Vec3f& Entity::origin() const {
            return m_origin;
        }

        const BBox& Entity::bounds() const {
            if (!m_geometryValid) validateGeometry();
            return m_bounds;
        }

        const BBox& Entity::maxBounds() const {
            if (!m_geometryValid) validateGeometry();
            return m_maxBounds;
        }

        void Entity::pick(const Ray& ray, HitList& hits) {
            if (worldspawn()) return;

            float dist = bounds().intersectWithRay(ray, NULL);
            if (Math::isnan(dist)) return;

            Vec3f hitPoint = ray.pointAtDistance(dist);
            Hit* hit = new Hit(this, TB_HT_ENTITY, hitPoint, dist);
            hits.add(*hit);
        }

        Map* Entity::quakeMap() const {
            return m_map;
        }

        void Entity::setMap(Map* quakeMap) {
            m_map = quakeMap;
        }

        const vector<Brush*>& Entity::brushes() const {
            return m_brushes;
        }

        const map<string, string> Entity::properties() const {
            return m_properties;
        }

        const string* Entity::propertyForKey(const string& key) const {
            map<string, string>::const_iterator it = m_properties.find(key);
            if (it == m_properties.end())
                return NULL;
            return &it->second;

        }

        bool Entity::propertyWritable(const string& key) const {
            return ClassnameKey != key;
        }

        bool Entity::propertyDeletable(const string& key) const {
            if (ClassnameKey == key)
                return false;
            if (OriginKey == key)
                return false;
            if (SpawnFlagsKey == key)
                return false;
            return true;
        }

        void Entity::setProperty(const string& key, const string& value) {
            setProperty(key, &value);
        }

        void Entity::setProperty(const string& key, const string* value) {
            if (key == ClassnameKey && classname() != NULL) {
                log(TB_LL_WARN, "Cannot overwrite classname property");
                return;
            } else if (key == OriginKey) {
                if (value == NULL) {
                    log(TB_LL_WARN, "Cannot set origin to NULL");
                    return;
                }
                m_origin = Vec3f(*value);
            } else if (key == AngleKey) {
                if (value != NULL) m_angle = static_cast<float>(atof(value->c_str()));
                else m_angle = numeric_limits<float>::quiet_NaN();
            }

            const string* oldValue = propertyForKey(key);
            if (oldValue != NULL && oldValue == value) return;
            m_properties[key] = *value;
            invalidateGeometry();
        }

        void Entity::setProperty(const string& key, const Vec3f& value, bool round) {
            stringstream valueStr;
            if (round) valueStr << (int)Math::fround(value.x) << " " << (int)Math::fround(value.y) << " " << (int)Math::fround(value.z);
            else valueStr << value.x << " " << value.y << " " << value.z;
            setProperty(key, valueStr.str());
        }

        void Entity::setProperty(const string& key, float value, bool round) {
            stringstream valueStr;
            if (round) valueStr << (int)Math::fround(value);
            else valueStr << value;
            setProperty(key, valueStr.str());
        }

        void Entity::setProperties(map<string, string> properties, bool replace) {
            if (replace) m_properties.clear();
            m_properties.insert(properties.begin(), properties.end());
        }

        void Entity::deleteProperty(const string& key) {
            if (!propertyDeletable(key)) {
                log(TB_LL_WARN, "Cannot delete property '%s'", key.c_str());
                return;
            }

            if (key == AngleKey) m_angle = numeric_limits<float>::quiet_NaN();
            if (m_properties.count(key) == 0) return;
            m_properties.erase(key);
            invalidateGeometry();
        }

        const string* Entity::classname() const {
            return propertyForKey(ClassnameKey);
        }

        const int Entity::angle() const {
            return static_cast<int>(Math::fround(m_angle));
        }

        bool Entity::worldspawn() const {
            return *classname() == WorldspawnClassname;
        }

        bool Entity::group() const {
            return *classname() == GroupClassname;
        }

        void Entity::addBrush(Brush* brush) {
            if (m_entityDefinition != NULL && m_entityDefinition->type != TB_EDT_BRUSH)
                return;

            brush->setEntity(this);
            m_brushes.push_back(brush);
            invalidateGeometry();
        }

        void Entity::addBrushes(const vector<Brush*>& brushes) {
            if (m_entityDefinition != NULL && m_entityDefinition->type != TB_EDT_BRUSH)
                return;

            for (unsigned int i = 0; i < brushes.size(); i++) {
                brushes[i]->setEntity(this);
                m_brushes.push_back(brushes[i]);
            }
            invalidateGeometry();
        }

        void Entity::brushChanged(Brush* brush) {
            invalidateGeometry();
        }

        void Entity::removeBrush(Brush* brush) {
            if (m_entityDefinition != NULL && m_entityDefinition->type != TB_EDT_BRUSH)
                return;

            brush->setEntity(NULL);
            m_brushes.erase(find(m_brushes.begin(), m_brushes.end(), brush));
            invalidateGeometry();
        }

        void Entity::removeBrushes(vector<Brush*>& brushes) {
            if (m_entityDefinition != NULL && m_entityDefinition->type != TB_EDT_BRUSH)
                return;

            for (unsigned int i = 0; i < brushes.size(); i++) {
                brushes[i]->setEntity(NULL);
                m_brushes.erase(find(m_brushes.begin(), m_brushes.end(), brushes[i]));
            }
            invalidateGeometry();
        }

        void Entity::translate(const Vec3f& delta) {
            if (m_entityDefinition != NULL && m_entityDefinition->type != TB_EDT_POINT)
                return;

            setProperty(OriginKey, m_origin + delta, true);
        }

        void Entity::rotate90(EAxis axis, const Vec3f& rotationCenter, bool clockwise) {
            if (m_entityDefinition != NULL && m_entityDefinition->type != TB_EDT_BRUSH)
                return;
            
            setProperty(OriginKey, m_origin.rotate90(axis, rotationCenter, clockwise), true);
            
            Vec3f direction;
            if (m_angle >= 0) {
                direction.x = cos(2 * Math::Pi - m_angle * Math::Pi / 180);
                direction.y = sin(2 * Math::Pi - m_angle * Math::Pi / 180);
                direction.z = 0;
            } else if (m_angle == -1) {
                direction = ZAxisPos;
            } else if (m_angle == -2) {
                direction = ZAxisNeg;
            } else {
                return;
            }
            
            direction = direction.rotate90(axis, clockwise);
            if (direction.z > 0.9) {
                setProperty(AngleKey, -1, true);
            } else if (direction.z < -0.9) {
                setProperty(AngleKey, -2, true);
            } else {
                if (direction.z != 0) {
                    direction.z = 0;
                    direction = direction.normalize();
                }
                
                m_angle = Math::fround(acos(direction.x) * 180 / Math::Pi);
                Vec3f cross = direction % XAxisPos;
                if (!cross.equals(Null3f) && cross.z < 0)
                    m_angle = 360 - m_angle;
                setProperty(AngleKey, m_angle, true);
            }
        }
        
        void Entity::rotate(const Quat& rotation, const Vec3f& rotationCenter) {
            if (m_entityDefinition != NULL && m_entityDefinition->type != TB_EDT_BRUSH)
                return;

            Vec3f offset = center() - origin();
            Vec3f newCenter = rotation * (center() - rotationCenter) + rotationCenter;
            setProperty(OriginKey, newCenter - offset, true);
            setProperty(AngleKey, 0, true);

            Vec3f direction;
            if (m_angle >= 0) {
                direction.x = cos(2 * Math::Pi - m_angle * Math::Pi / 180);
                direction.y = sin(2 * Math::Pi - m_angle * Math::Pi / 180);
                direction.z = 0;
            } else if (m_angle == -1) {
                direction = ZAxisPos;
            } else if (m_angle == -2) {
                direction = ZAxisNeg;
            } else {
                return;
            }

            direction = rotation * direction;
            if (direction.z > 0.9) {
                setProperty(AngleKey, -1, true);
            } else if (direction.z < -0.9) {
                setProperty(AngleKey, -2, true);
            } else {
                if (direction.z != 0) {
                    direction.z = 0;
                    direction = direction.normalize();
                }

                m_angle = Math::fround(acos(direction.x) * 180 / Math::Pi);
                Vec3f cross = direction % XAxisPos;
                if (!cross.equals(Null3f) && cross.z < 0)
                    m_angle = 360 - m_angle;
                setProperty(AngleKey, m_angle, true);
            }
            invalidateGeometry();
        }

        void Entity::flip(EAxis axis, const Vec3f& flipCenter) {
            if (m_entityDefinition != NULL && m_entityDefinition->type != TB_EDT_BRUSH)
                return;

            Vec3f offset = center() - origin();
            Vec3f newCenter = center().flip(axis, flipCenter);
            setProperty(OriginKey, newCenter + offset, true);
            setProperty(AngleKey, 0, true);

            if (m_angle >= 0)
                m_angle = (m_angle + 180) - (int)((m_angle / 360)) * m_angle;
            else if (m_angle == -1)
                m_angle = -2;
            else if (m_angle == -2)
                m_angle = -1;
            setProperty(AngleKey, m_angle, true);
            invalidateGeometry();
        }

        int Entity::filePosition() const {
            return m_filePosition;
        }

        void Entity::setFilePosition(int filePosition) {
            m_filePosition = filePosition;
        }

        bool Entity::selected() const {
            return m_selected;
        }

        void Entity::setSelected(bool selected) {
            m_selected = selected;
        }

        Renderer::VboBlock* Entity::vboBlock() const {
            return m_vboBlock;
        }

        void Entity::setVboBlock(Renderer::VboBlock* vboBlock) {
            if (m_vboBlock != NULL)
                m_vboBlock->freeBlock();
            m_vboBlock = vboBlock;
        }
    }
}
