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
#include "Model/Map/Brush.h"
#include "Model/Map/EntityDefinition.h"
#include "Model/Map/Map.h"
#include "Model/Map/MapObject.h"
#include "Renderer/Vbo.h"
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
            mutable BBox m_maxBounds;
            mutable bool m_geometryValid;
            
            Map* m_map;
            std::vector<Brush*> m_brushes;
            
            std::map<std::string, std::string> m_properties;
            
            int m_filePosition;
            bool m_selected;
            Renderer::VboBlock* m_vboBlock;
            
            void init();
            void validateGeometry() const;
            void invalidateGeometry();
        public:
            Entity();
            Entity(const std::map<std::string, std::string>& properties);
            ~Entity();
            
            EMapObjectType objectType() const;
            const EntityDefinitionPtr entityDefinition() const;
            void setEntityDefinition(EntityDefinitionPtr entityDefinition);
            const Vec3f& center() const;
            const Vec3f& origin() const;
            const BBox& bounds() const;
            const BBox& maxBounds() const;
            
            void pick(const Ray& ray, HitList& hits);

            Map* quakeMap() const;
            void setMap(Map* quakeMap);
            const std::vector<Brush*>& brushes() const;
            
            const std::map<std::string, std::string>& properties() const;
            const std::string* propertyForKey(const std::string& key) const;
            bool propertyWritable(const std::string& key) const;
            bool propertyDeletable(const std::string& key) const;
            void setProperty(const std::string& key, const std::string& value);
            void setProperty(const std::string& key, const std::string* value);
            void setProperty(const std::string& key, const Vec3f& value, bool round);
            void setProperty(const std::string& key, int value);
            void setProperty(const std::string& key, float value, bool round);
            void setProperties(const std::map<std::string, std::string>& properties, bool replace);
            void deleteProperty(const std::string& key);
            
            const std::string* classname() const;
            const int angle() const;
            bool worldspawn() const;
            bool group() const;
            
            void addBrush(Brush* brush);
            void addBrushes(const std::vector<Brush*>& brushes);
            void brushChanged(Brush* brush);
            void removeBrush(Brush* brush);
            void removeBrushes(std::vector<Brush*>& brushes);
            
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
