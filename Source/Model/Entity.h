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

#ifndef __TrenchBroom__Entity__
#define __TrenchBroom__Entity__

#include "Model/BrushTypes.h"
#include "Model/EditState.h"
#include "Model/EntityTypes.h"
#include "Model/MapObject.h"
#include "Utility/Allocator.h"
#include "Utility/VecMath.h"

#include <cstdlib>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class Brush;
        class EntityDefinition;
        class Map;
        
        class Entity : public MapObject, public Utility::Allocator<Entity> {
        public:
            static String const ClassnameKey;
            static String const SpawnFlagsKey;
            static String const WorldspawnClassname;
            static String const GroupClassname;
            static String const GroupNameKey;
            static String const GroupVisibilityKey;
            static String const OriginKey;
            static String const AngleKey;
            static String const MessageKey;
            static String const ModsKey;
            static String const WadKey;
        protected:
            Map* m_map;
            Properties m_properties;
            BrushList m_brushes;
            
            EntityDefinition* m_definition;
            
            unsigned int m_selectedBrushCount;
            
            const BBox& m_worldBounds;
            
            mutable BBox m_bounds;
            mutable Vec3f m_center;
            mutable bool m_geometryValid;
            
            size_t m_filePosition;
            
            void init();
            void validateGeometry() const;
        public:
            Entity(const BBox& worldBounds);
            Entity(const BBox& worldBounds, const Entity& entityTemplate);
            ~Entity();
            
            inline MapObject::Type objectType() const {
                return MapObject::EntityObject;
            }
            
            inline Map* map() const {
                return m_map;
            }
            
            inline void setMap(Map* map) {
                m_map = map;
            }
            
            inline const Properties& properties() const {
                return m_properties;
            }
            
            inline const PropertyValue* propertyForKey(const PropertyKey& key) const {
                Properties::const_iterator it = m_properties.find(key);
                if (it == m_properties.end())
                    return NULL;
                return &it->second;
            }
            
            void setProperty(const PropertyKey& key, const PropertyValue& value);
            void setProperty(const PropertyKey& key, const PropertyValue* value);
            void setProperty(const PropertyKey& key, const Vec3f& value, bool round);
            void setProperty(const PropertyKey& key, int value);
            void setProperty(const PropertyKey& key, float value, bool round);
            void setProperties(const Properties& properties, bool replace);
            static bool propertyKeyIsMutable(const PropertyKey& key);
            void deleteProperty(const PropertyKey& key);

            inline const PropertyValue* classname() const {
                return propertyForKey(ClassnameKey);
            }
            
            inline bool worldspawn() const {
                const PropertyValue* classname = this->classname();
                return classname != NULL && *classname == WorldspawnClassname;
            }
            
            inline const Vec3f origin() const {
                const PropertyValue* value = propertyForKey(OriginKey);
                if (value == NULL)
                    return Vec3f::Null;
                return Vec3f(*value);
            }
            
            inline const int angle() const {
                const PropertyValue* value = propertyForKey(AngleKey);
                if (value == NULL)
                    return 0;
                
                double dblValue = std::atof(value->c_str());
                float fltValue = static_cast<float>(dblValue);
                return static_cast<int>(Math::round(fltValue));
            }
            
            inline const BrushList& brushes() const {
                return m_brushes;
            }
            
            void addBrush(Brush& brush);
            void addBrushes(const BrushList& brushes);
            void removeBrush(Brush& brush);
            
            inline EntityDefinition* definition() const {
                return m_definition;
            }
        
            void setDefinition(EntityDefinition* definition);

            bool selectable() const;
            
            inline bool partiallySelected() const {
                return m_selectedBrushCount > 0;
            }
            
            inline void incSelectedBrushCount() {
                m_selectedBrushCount++;
            }
            
            inline void decSelectedBrushCount() {
                m_selectedBrushCount--;
            }
            
            virtual EditState::Type setEditState(EditState::Type editState);

            inline const BBox& worldBounds() const {
                return m_worldBounds;
            }
            
            inline const Vec3f& center() const {
                if (!m_geometryValid)
                    validateGeometry();
                return m_center;
            }
            
            inline const BBox& bounds() const {
                if (!m_geometryValid)
                    validateGeometry();
                return m_bounds;
            }
            
            inline void invalidateGeometry() {
                m_geometryValid = false;
            }
            
            inline size_t filePosition() const {
                return m_filePosition;
            }
            
            inline void setFilePosition(size_t filePosition) {
                m_filePosition = filePosition;
            }
            
            void translate(const Vec3f& delta, bool lockTextures);
            void rotate90(Axis::Type axis, const Vec3f& center, bool clockwise, bool lockTextures);
            void rotate(const Quat& rotation, const Vec3f& center, bool lockTextures);
            void flip(Axis::Type axis, const Vec3f& center, bool lockTextures);
            void pick(const Ray& ray, PickResult& pickResults);
        };
    }
}

#endif /* defined(__TrenchBroom__Entity__) */
