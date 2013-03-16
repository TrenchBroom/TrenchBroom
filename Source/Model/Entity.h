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
#include "Model/EntityProperty.h"
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
            static String const AnglesKey;
            static String const MangleKey;
            static String const MessageKey;
            static String const ModKey;
            static String const TargetKey;
            static String const WadKey;
            static String const DefKey;
            static String const DefaultDefinition;
        protected:
            Map* m_map;
            PropertyStore m_propertyStore;
            BrushList m_brushes;
            bool m_worldspawn;

            EntityDefinition* m_definition;

            unsigned int m_selectedBrushCount;
            unsigned int m_hiddenBrushCount;

            const BBox& m_worldBounds;

            mutable BBox m_bounds;
            mutable Vec3f m_center;
            mutable bool m_geometryValid;

            void init();
            void validateGeometry() const;

            typedef enum {
                RTNone,
                RTZAngle,
                RTZAngleWithUpDown,
                RTEulerAngles
            } RotationType;
            
            struct RotationInfo {
                const RotationType type;
                const PropertyKey property;

                RotationInfo(RotationType i_type, const PropertyKey& i_property) :
                type(i_type),
                property(i_property) {}
            };

            const RotationInfo rotationInfo() const;
            void applyRotation(const Quat& rotation);
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

            inline const PropertyList& properties() const {
                return m_propertyStore.properties();
            }

            inline const PropertyValue* propertyForKey(const PropertyKey& key) const {
                return m_propertyStore.propertyValue(key);
            }

            void setProperty(const PropertyKey& key, const PropertyValue& value);
            void setProperty(const PropertyKey& key, const PropertyValue* value);
            void setProperty(const PropertyKey& key, const Vec3f& value, bool round);
            void setProperty(const PropertyKey& key, int value);
            void setProperty(const PropertyKey& key, float value, bool round);
            void renameProperty(const PropertyKey& oldKey, const PropertyKey& newKey);
            void setProperties(const PropertyList& properties, bool replace);
            static bool propertyKeyIsMutable(const PropertyKey& key);
            void removeProperty(const PropertyKey& key);

            inline const PropertyValue* classname() const {
                return propertyForKey(ClassnameKey);
            }

            inline bool worldspawn() const {
                return m_worldspawn;
            }

            inline const Vec3f origin() const {
                const PropertyValue* value = propertyForKey(OriginKey);
                if (value == NULL)
                    return Vec3f::Null;
                return Vec3f(*value);
            }

            inline bool rotated() const {
                if (classname() == NULL)
                    return false;
                if (Utility::startsWith(*classname(), "light")) {
                    if (propertyForKey(MangleKey) != NULL)
                        return true;
                } else {
                    if (propertyForKey(AngleKey) != NULL)
                        return true;
                    if (propertyForKey(AnglesKey) != NULL)
                        return true;
                }
                return false;
            }

            const Quat rotation() const;

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

            inline bool fullyHidden() const {
                return !m_brushes.empty() && m_hiddenBrushCount == m_brushes.size();
            }

            inline void incHiddenBrushCount() {
                m_hiddenBrushCount++;
            }

            inline void decHiddenBrushCount() {
                m_hiddenBrushCount--;
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

            void translate(const Vec3f& delta, bool lockTextures);
            void rotate90(Axis::Type axis, const Vec3f& center, bool clockwise, bool lockTextures);
            void rotate(const Quat& rotation, const Vec3f& center, bool lockTextures);
            void flip(Axis::Type axis, const Vec3f& center, bool lockTextures);
            void pick(const Ray& ray, PickResult& pickResults);
        };
    }
}

#endif /* defined(__TrenchBroom__Entity__) */
