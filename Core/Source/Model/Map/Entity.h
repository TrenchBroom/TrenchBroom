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

#ifndef TrenchBroom_Entity_h
#define TrenchBroom_Entity_h

#include <string>
#include <vector>
#include <map>
#include "Model/Map/BrushTypes.h"
#include "Model/Map/EntityDefinition.h"
#include "Model/Map/EntityTypes.h"
#include "Model/Map/MapObject.h"
#include "Utilities/VecMath.h"

namespace TrenchBroom {
    namespace Model {
        static std::string const ClassnameKey        = "classname";
        static std::string const SpawnFlagsKey       = "spawnflags";
        static std::string const WorldspawnClassname = "worldspawn";
        static std::string const GroupClassname      = "func_group";
        static std::string const GroupNameKey        = "__tb_group_name";
        static std::string const GroupVisibilityKey  = "__tb_group_visible";
        static std::string const OriginKey           = "origin";
        static std::string const AngleKey            = "angle";
        static std::string const MessageKey          = "message";
        static std::string const ModsKey             = "__tb_mods";
        static std::string const WadKey              = "wad";
        
        class Map;
        class Brush;
        
        class Entity : public MapObject {
        private:
            EntityDefinitionPtr m_entityDefinition;
            Vec3f m_origin;
            float m_angle;
            mutable Vec3f m_center;
            mutable BBox m_bounds;
            mutable bool m_geometryValid;
            
            Map* m_map;
            BrushList m_brushes;
            
            Properties m_properties;
            
            int m_filePosition;
            bool m_selected;
            
            void init();
            void validateGeometry() const;
            void invalidateGeometry();
        public:
            Entity();
            Entity(const Properties& properties);
            ~Entity();
            
            EMapObjectType objectType() const;
            const EntityDefinitionPtr entityDefinition() const;
            void setEntityDefinition(EntityDefinitionPtr entityDefinition);
            const Vec3f& center() const;
            const Vec3f& origin() const;
            const BBox& bounds() const;
            
            void pick(const Ray& ray, HitList& hits);

            Map* quakeMap() const;
            void setMap(Map* quakeMap);
            const BrushList& brushes() const;
            
            const Properties& properties() const;
            const PropertyValue* propertyForKey(const PropertyKey& key) const;
            bool propertyWritable(const PropertyKey& key) const;
            bool propertyDeletable(const PropertyKey& key) const;
            void setProperty(const PropertyKey& key, const PropertyValue& value);
            void setProperty(const PropertyKey& key, const PropertyValue* value);
            void setProperty(const PropertyKey& key, const Vec3f& value, bool round);
            void setProperty(const PropertyKey& key, int value);
            void setProperty(const PropertyKey& key, float value, bool round);
            void setProperties(const Properties& properties, bool replace);
            void deleteProperty(const PropertyKey& key);
            
            const PropertyValue* classname() const;
            const int angle() const;
            bool worldspawn() const;
            bool group() const;
            
            void addBrush(Brush* brush);
            void addBrushes(const BrushList& brushes);
            void brushChanged(Brush* brush);
            void removeBrush(Brush* brush);
            void removeBrushes(BrushList& brushes);
            
            void translate(const Vec3f& delta);
            void rotate90(EAxis axis, const Vec3f& rotationCenter, bool clockwise);
            void rotate(const Quat& rotation, const Vec3f& rotationCenter);
            void flip(EAxis axis, const Vec3f& flipCenter);
            
            int filePosition() const;
            void setFilePosition(int filePosition);
            bool selected() const;
            void setSelected(bool selected);
        };
    }
}
#endif
