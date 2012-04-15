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

using namespace std;

namespace TrenchBroom {
    namespace Model {
        static string const ClassnameKey        = "classname";
        static string const SpawnFlagsKey       = "spawnflags";
        static string const WorldspawnClassname = "worldspawn";
        static string const GroupClassname      = "func_group";
        static string const GroupNameKey        = "__tb_group_name";
        static string const GroupVisibilityKey  = "__tb_group_visible";
        static string const OriginKey           = "origin";
        static string const AngleKey            = "angle";
        static string const MessageKey          = "message";
        static string const ModsKey             = "__tb_mods";
        static string const WadKey              = "wad";
        
        class Map;
        class Brush;
        class Entity : public MapObject {
        private:
            EntityDefinition* m_entityDefinition;
            Vec3f m_center;
            Vec3f m_origin;
            float m_angle;
            BBox m_bounds;
            BBox m_maxBounds;
            bool m_geometryValid;
            
            Map* m_map;
            vector<Brush*> m_brushes;
            
            map<string, string> m_properties;
            
            int m_filePosition;
            bool m_selected;
            Renderer::VboBlock* m_vboBlock;
            
            void init();
            void validateGeometry();
            void invalidateGeometry();
            void rotate90(EAxis axis, Vec3f rotationCenter, bool clockwise);
        public:
            Entity();
            Entity(const map<string, string> properties);
            ~Entity();
            
            EMapObjectType objectType() const;
            const EntityDefinition* entityDefinition() const;
            void setEntityDefinition(EntityDefinition* entityDefinition);
            const Vec3f& center();
            const Vec3f& origin() const;
            const BBox& bounds();
            const BBox& maxBounds();
            
            void pick(const Ray& ray, HitList& hits);

            Map* quakeMap() const;
            void setMap(Map* quakeMap);
            const vector<Brush*>& brushes() const;
            
            const map<string, string> properties() const;
            const string* propertyForKey(const string& key) const;
            bool propertyWritable(const string& key) const;
            bool propertyDeletable(const string& key) const;
            void setProperty(const string& key, const string& value);
            void setProperty(const string& key, const string* value);
            void setProperty(const string& key, Vec3f value, bool round);
            void setProperty(const string& key, float value, bool round);
            void setProperties(map<string, string> properties, bool replace);
            void deleteProperty(const string& key);
            
            const string* classname() const;
            const int angle() const;
            bool worldspawn() const;
            bool group() const;
            
            void addBrush(Brush* brush);
            void addBrushes(const vector<Brush*>& brushes);
            void brushChanged(Brush* brush);
            void removeBrush(Brush* brush);
            void removeBrushes(vector<Brush*>& brushes);
            
            void translate(Vec3f delta);
            void rotate90CW(EAxis axis, Vec3f rotationCenter);
            void rotate90CCW(EAxis axis, Vec3f rotationCenter);
            void rotate(Quat rotation, Vec3f rotationCenter);
            void flip(EAxis axis, Vec3f flipCenter);
            
            int filePosition() const;
            void setFilePosition(int filePosition);
            bool selected() const;
            void setSelected(bool selected);
            Renderer::VboBlock* vboBlock() const;
            void setVboBlock(Renderer::VboBlock* vboBlock);
        };
    }
}
#endif
