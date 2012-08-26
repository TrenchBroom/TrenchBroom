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

#include "Model/Brush.h"
#include "Model/EntityDefinition.h"

#include <algorithm>

namespace TrenchBroom {
    namespace Model {
        String const Entity::ClassnameKey        = "classname";
        String const Entity::SpawnFlagsKey       = "spawnflags";
        String const Entity::WorldspawnClassname = "worldspawn";
        String const Entity::GroupClassname      = "func_group";
        String const Entity::GroupNameKey        = "__tb_group_name";
        String const Entity::GroupVisibilityKey  = "__tb_group_visible";
        String const Entity::OriginKey           = "origin";
        String const Entity::AngleKey            = "angle";
        String const Entity::MessageKey          = "message";
        String const Entity::ModsKey             = "__tb_mods";
        String const Entity::WadKey              = "wad";

        void Entity::init() {
            m_map = NULL;
            m_filePosition = 0;
            m_editState = EditState::Default;
            m_selectedBrushCount = 0;
            m_origin = Vec3f::Null;
            m_angle = 0.0f;
            invalidateGeometry();
        }

        void Entity::validateGeometry() const {
            assert(!m_geometryValid);
            
            if (m_definition == NULL || m_definition->type() == EntityDefinition::Type::Brush) {
                if (!m_brushes.empty()) {
                    m_bounds = m_brushes[0]->bounds();
                    for (unsigned int i = 1; i < m_brushes.size(); i++)
                        m_bounds += m_brushes[i]->bounds();
                } else {
                    m_bounds = BBox(Vec3f(-8, -8, -8), Vec3f(8, 8, 8));
                    m_bounds.translate(m_origin);
                }
            } else {
                PointEntityDefinition* pointDefinition = static_cast<PointEntityDefinition*>(m_definition);
                m_bounds = pointDefinition->bounds();
                m_bounds.translate(m_origin);
            }
            
            m_center = m_bounds.center();
            m_geometryValid = true;
        }

        Entity::Entity(const BBox& worldBounds) : MapObject(), m_worldBounds(worldBounds) {
            init();
        }

        Entity::~Entity() {
            setMap(NULL);
            while (!m_brushes.empty()) delete m_brushes.back(), m_brushes.pop_back();
            m_brushes.clear();
            setDefinition(NULL);
            m_geometryValid = false;
        }

        void Entity::setProperty(const PropertyKey& key, const PropertyValue& value) {
            setProperty(key, &value);
        }
        
        void Entity::setProperty(const PropertyKey& key, const PropertyValue* value) {
            if (key == ClassnameKey) {
                if (value != classname())
                    setDefinition(NULL);
            } else if (key == OriginKey) {
                if (value == NULL)
                    m_origin = Vec3f(0, 0, 0);
                else
                    m_origin = Vec3f(*value);
            } else if (key == AngleKey) {
                if (value != NULL)
                    m_angle = static_cast<float>(atof(value->c_str()));
                else
                    m_angle = Math::nan();
            }
            
            const PropertyValue* oldValue = propertyForKey(key);
            if (oldValue != NULL && oldValue == value)
                return;
            m_properties[key] = *value;
            invalidateGeometry();
        }
        
        void Entity::setProperty(const PropertyKey& key, const Vec3f& value, bool round) {
            StringStream valueStr;
            if (round)
                valueStr    << static_cast<int>(Math::round(value.x)) <<
                " "         << static_cast<int>(Math::round(value.y)) <<
                " "         << static_cast<int>(Math::round(value.z));
            else
                valueStr << value.x << " " << value.y << " " << value.z;
            setProperty(key, valueStr.str());
        }
        
        void Entity::setProperty(const PropertyKey& key, int value) {
            StringStream valueStr;
            valueStr << value;
            setProperty(key, valueStr.str());
        }
        
        void Entity::setProperty(const PropertyKey& key, float value, bool round) {
            StringStream valueStr;
            if (round)
                valueStr << static_cast<float>(Math::round(value));
            else
                valueStr << value;
            setProperty(key, valueStr.str());
        }
        
        void Entity::setProperties(const Properties& properties, bool replace) {
            if (replace)
                m_properties.clear();
            Properties::const_iterator it;
            for (it = properties.begin(); it != properties.end(); ++it)
                setProperty(it->first, it->second);
        }
        
        void Entity::deleteProperty(const PropertyKey& key) {
            if (m_properties.count(key) == 0)
                return;
            
            if (key == ClassnameKey)
                setDefinition(NULL);
            else if (key == OriginKey)
                m_origin = Vec3f();
            else if (key == AngleKey)
                m_angle = Math::nan();
            
            m_properties.erase(key);
            invalidateGeometry();
        }
        void Entity::addBrush(Brush* brush) {
            if (m_definition == NULL || m_definition->type() == EntityDefinition::Type::Point)
                return;
            
            brush->setEntity(this);
            m_brushes.push_back(brush);
            invalidateGeometry();
        }
        
        void Entity::addBrushes(const BrushList& brushes) {
            if (m_definition == NULL || m_definition->type() == EntityDefinition::Type::Point)
                return;
            
            for (unsigned int i = 0; i < brushes.size(); i++) {
                Model::Brush* brush = brushes[i];
                brush->setEntity(this);
                m_brushes.push_back(brush);
            }
            invalidateGeometry();
        }
        
        void Entity::removeBrush(Brush* brush) {
            if (m_definition == NULL || m_definition->type() == EntityDefinition::Type::Point)
                return;
            
            brush->setEntity(NULL);
            m_brushes.erase(std::remove(m_brushes.begin(), m_brushes.end(), brush), m_brushes.end());
            invalidateGeometry();
        }

        void Entity::setDefinition(EntityDefinition* definition) {
            if (m_definition != NULL)
                m_definition->decUsageCount();
            m_definition = definition;
            if (m_definition != NULL)
                m_definition->incUsageCount();
            invalidateGeometry();
        }

        void Entity::pick(const Ray& ray, PickResult& pickResults, Filter& filter) const {
        }
    }
}