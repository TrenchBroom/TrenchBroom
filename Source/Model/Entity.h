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

#include "Model/Brush.h"
#include "Model/EntityDefinition.h"
#include "Model/MapObject.h"
#include "Utility/VecMath.h"

#include <cassert>
#include <map>
#include <set>
#include <vector>

using namespace TrenchBroom::Math;

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
        
        class MapDocument;
        
        typedef std::string PropertyKey;
        typedef std::string PropertyValue;
        typedef std::map<PropertyKey, PropertyValue> Properties;

        class Entity : public MapObject {
        public:
            typedef std::vector<Entity*> List;
            typedef std::set<Entity*> Set;
        protected:
            EntityDefinition* m_entityDefinition;
            Vec3f m_origin;
            float m_angle;
            mutable Vec3f m_center;
            mutable BBox m_bounds;
            mutable bool m_geometryValid;
            
            MapDocument* m_map;
            Brush::List m_brushes;
            
            Properties m_properties;
            
            int m_filePosition;
            bool m_selected;
            unsigned int m_selectedBrushCount;

            inline void ValidateGeometry() const {
                assert(!m_geometryValid);
                
                /*
                if (m_entityDefinition == NULL || m_entityDefinition->type() == EntityDefinition::Type::Brush) {
                    if (!m_brushes.empty()) {
                        m_bounds = m_brushes[0]->bounds();
                        for (unsigned int i = 1; i < m_brushes.size(); i++)
                            m_bounds += m_brushes[i]->bounds();
                    } else {
                        m_bounds = BBox(Vec3f(-8, -8, -8), Vec3f(8, 8, 8));
                        m_bounds.translate(m_origin);
                    }
                } else if (m_entityDefinition->Type() == EntityDefinition::Type::Point) {
                    m_bounds = m_entityDefinition->bounds().translate(m_origin);
                }
                 */
                
                m_center = m_bounds.center();
                m_geometryValid = true;
            }
            
            void InvalidateGeometry() {
                m_geometryValid = false;
            }
        public:
            Entity();
            ~Entity();
            
            inline const BBox& Bounds() const {
                return m_bounds;
            }
            
            inline Type ObjectType() const {
                return Type::Entity;
            }
            
            void Pick(const Ray& ray, PickResult& pickResults, Filter& filter) {}
            
            
        };
    }
}

#endif /* defined(__TrenchBroom__Entity__) */
